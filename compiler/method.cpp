#include "method.h"
#include "utils.h"
#include "call_ctx.h"
#include "interpreter.h"
#include "number.h"
#include "consts.h"
#include "bt_string.h"
#include "parameter.h"
#include "type.h"
#include "sys_brkp.h"
#include "evaluate.h"
#include "variable.h"
#include "expression_tree.h"
#include "compiler.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include <sstream>

/**
 * Creates a new method
 */
method::method(nap_compiler* _compiler, const std::string &name, const std::string &preturn_type, call_context* cc) :
    method_name (name), library_name("-"), return_type(preturn_type), def_loc(DEF_INTERN),
    ret_type(0), mcompiler(_compiler), owns_cc(false)
{
    call_context* method_main_cc = NULL;
    std::stringstream ss;
    ss << cc->name << STR_CALL_CONTEXT_SEPARATOR << name;

    if(!preturn_type.empty() && starts_with(preturn_type, "extern"))
    {
        /* if this method is a method defined somewhere else ... such as your C++ application */
        const char* after_ext = preturn_type.c_str() + 6;
        return_type = after_ext;
        strim(return_type);
        def_loc = DEF_EXTERN;
    }

    method_main_cc = new call_context(_compiler, call_context::CC_NAMED, ss.str(), this, cc);
    main_cc = method_main_cc;
}

method::~method()
{
    if(owns_cc)
    {
        // this will delete the main_cc of this too!
        delete main_cc->father;
    }

    for(size_t i=0; i<parameters.size(); i++)
    {
        delete parameters[i];
    }

}

call_frame_entry::~call_frame_entry()
{
    for(size_t i=0; i<parameters.size(); i++)
    {
        delete parameters[i];
    }
}


constructor_call::constructor_call(char* name, call_context* cc) : method(cc->compiler, name, 0, cc)
{
    the_class = cc->get_class_declaration(name);
}

/**
 * Adds a new variable.
 * If the dimensions is 1
 */
variable* method::add_new_variable(const std::string& pname,
                                   const std::string& ptype, int dimension,
                                   bool& psuccess)
{
    if(pname.empty())
    {
        if(def_loc == DEF_INTERN)
        {
            if(!is_valid_variable_name(pname.c_str()))
            {
                mcompiler->throw_error(E0018_INVIDENT, pname, NULL);
                psuccess = false;
                return 0;
            }
        }
    }

    variable* v = add_variable(pname.c_str(), ptype.c_str(), dimension, this,
                               main_cc, psuccess);
    SUCCES_OR_RETURN 0;
    return v;
}

/**
 * Returns the  variable from this  method. The algorithm is the following:
 * 1. find the variables of the call context passed in
 * 2. find the variables of the cc's father, and its father, etc...
 * 3. if reached to the method's main cc find variable in all the CC's above
 * 4. find the variables of the method (meaning: parameters)
 */
 variable* method::has_variable(call_context* cc, const std::string& varname, int* env_var)
{
    //printf("\t[MGV]: Variable [%s] in method [%s]\n", varname, the_method?the_method->name:"global");
    if(varname[0] == C_DOLLAR)        /* is this an enviornment variable? */
    {
        *env_var = 1;
        return NULL;
    }

    std::vector<variable*>::const_iterator location = cc->has_variable(varname);
    if(location != cc->variables.end())
    {
        return *location;
    }

    // first run_ is this variable defined in here?
    location = variable_holder::has_variable(varname);
    if(location != variables.end())
    {
        return *location;
    }

    /* parameter as a $sign? */
    if(varname[0] == C_DOLLAR)
    {
        std::string real_varname = varname.substr(1);
        int varc = atoi(real_varname.c_str());
        variable* v = variables[varc];

        if(v)
        {
            return v;
        }
    }

    /* variable in the call contexts above our call context? */
    cc = cc->father;
    while(cc)
    {
        location = cc->has_variable(varname);
        if(location != cc->variables.end())
        {
            return *location;
        }
        cc = cc->father;
    }

    return NULL;
}


/**
 * Adds a new parameter to the method. These will go in the
 */
parameter* method::add_parameter(std::string pname,
                                 const std::string& ptype,
                                 int pdimension,
                                 expression_with_location* pexpwloc,
                                 bool& psuccess)
{
    parameter* func_par = new parameter(this, pname, get_typeid(ptype));

    size_t indexOfEq = pname.find(C_EQ);
    variable* nvar = NULL;

    if(indexOfEq != std::string::npos)
    {
        std::string afterEq = pname.substr(indexOfEq + 1);
        int res = -1;
        func_par->initial_value = new expression_tree(pexpwloc);

        char* tmp = new char[afterEq.length() + 1];
        afterEq.copy(tmp, afterEq.length());
        mcompiler->get_interpreter().build_expr_tree(tmp,
                                                     func_par->initial_value,
                                                     this, tmp, main_cc,
                                                     &res, pexpwloc, psuccess);
        delete[] tmp;
        SUCCES_OR_RETURN 0;
        pname = pname.substr(0, indexOfEq);
        strim(pname);
    }

    nvar = add_new_variable(pname, ptype, pdimension, psuccess);
    SUCCES_OR_RETURN 0;
    nvar->func_par = func_par;

    parameters.push_back(func_par);
    return func_par;
}

parameter* method::get_parameter(size_t i)
{
    // TODO: This was variables[i].the_parameter ... why?
    if(i < parameters.size())
    {
        return parameters[i];
    }
    else
    {
        return 0;
    }
}

/**
 * Populates the parameters of this method with the definitions from the string
 */
void method::feed_parameter_list(const char* par_list, expression_with_location* expwloc, bool& psuccess)
{
    std::vector<std::string> entries = string_list_create_bsep(par_list, C_COMMA, mcompiler, psuccess);
    SUCCES_OR_RETURN;

    std::vector<std::string>::iterator q = entries.begin();
    while(q != entries.end())
    {
        size_t i=0;
        if(!q->empty())
        {
            std::string par_type, par_name;
            while(i < q->length()
                  && (
                      is_identifier_char((*q)[i])
                      || C_SQPAR_OP  == (*q)[i]
                      || C_SQPAR_CL == (*q)[i]
                     )
                  )
            {
                par_type += (*q)[i++];
            }
            /* now par_type contains the type of the parameter */
            while(i < q->length() && is_whitespace((*q)[i]))
            {
                i++;
            }
			size_t ql = q->length();
            if(i >= ql && def_loc != DEF_EXTERN)
            {
                mcompiler->throw_error(E0009_PARAMISM, par_list);
                psuccess = false;
                return;
            }

			//modifiable = (C_AND == (*q)[i]);
            //if(modifiable)
            //{
            //    i++;
                // TODO: There is no support for this in the bytecode yet.
            //}

            while(i < q->length())
            {
                par_name += (*q)[i++];
            }
            strim(par_name);
            strim(par_type);
            if(def_loc == DEF_INTERN)
            {
                parameter* new_par_decl = add_parameter(par_name, par_type, 1, expwloc, psuccess);
                SUCCES_OR_RETURN;

                /* here we should identify the dimension of the parameter */
                if(par_type.find(C_SQPAR_CL) != std::string::npos
                        && par_type.find(C_SQPAR_OP) != std::string::npos)
                {
                    new_par_decl->simple_value = 0;
                }
            }
            else
            {
                add_parameter("", par_type, 1, expwloc, psuccess);
                SUCCES_OR_RETURN;
            }
        }
        q ++;
    }
}
