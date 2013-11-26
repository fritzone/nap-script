#include "method.h"
#include "utils.h"
#include "call_ctx.h"
#include "interpreter.h"
#include "number.h"
#include "consts.h"
#include "envelope.h"
#include "bt_string.h"
#include "parametr.h"
#include "type.h"
#include "sys_brkp.h"
#include "evaluate.h"
#include "variable.h"
#include "parametr.h"
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
method::method(nap_compiler* _compiler, char* name, char* preturn_type, call_context* cc) : return_type("")
{
    call_context* method_main_cc = NULL;
    std::stringstream ss;
    ss << cc->get_name() << STR_CALL_CONTEXT_SEPARATOR << name;

    def_loc = DEF_INTERN;
    method_name = name;
    if(preturn_type && strstr(preturn_type, "extern"))
    {
        /* if this method is a method defined somewhere else ... such as your C++ application */
        char* after_ext = preturn_type + 6;
        return_type = after_ext;
        strim(return_type);
        def_loc = DEF_EXTERN;
    }
    else
    if(preturn_type)
    {
        return_type = preturn_type;
    }
    method_main_cc = new call_context(_compiler, CALL_CONTEXT_TYPE_METHOD_MAIN, ss.str(), this, cc);
    garbage_bin<call_context*>::instance().place(method_main_cc, cc->compiler());
    main_cc = method_main_cc;

    cur_cf = 0;
}

constructor_call::constructor_call(char* name, call_context* cc) : method(cc->compiler(), name, 0, cc)
{
    the_class = cc->get_class_declaration(name);
}

/**
 * Adds a new variable.
 * If the dimensions is 1
 */
variable* method::add_new_variable(char* pname, char* type, int dimension, const expression_with_location* expwloc, bool& psuccess)
{
    char* new_name = trim(pname, mcompiler);
    if(!is_valid_variable_name(pname))
    {
        mcompiler->throw_error(E0018_INVIDENT, pname, NULL);
        psuccess = false;
        return 0;
    }
    bool success = true;
    variable* v = main_cc->variable_list_add_variable(new_name, type, dimension, variables, this, main_cc, expwloc, success);
    if(!success)
    {
        psuccess = false;
        return 0;
    }
    return v;
}

/**
 * Returns the  variable from this  method. The algorithm is the following:
 * 1. find the variables of the call context passed in
 * 2. find the variables of the cc's father, and its father, etc...
 * 3. if reached to the method's main cc find variable in all the CC's above
 * 4. find the variables of the method (meaning: parameters)
 */
 variable* method::has_variable(call_context* cc, char* varname, int* templed, int* env_var, bool& psuccess)
{
    //printf("\t[MGV]: Variable [%s] in method [%s]\n", varname, the_method?the_method->name:"global");
    if(varname[0] == C_DOLLAR)        /* is this an enviornment variable? */
    {
        *env_var = 1;
        return NULL;
    }

    if(strchr(varname, C_PAR_OP))    /* this is a templated variable */
    {
        *templed = 1;
        *strchr(varname, C_PAR_OP) = 0;
    }

    std::vector<variable*>::const_iterator location = cc->variable_list_has_variable(varname, cc->get_variables());
    if(location != cc->get_variables().end())
    {
        return *location;
    }

    // first run_ is this variable defined in here?
    location = cc->variable_list_has_variable(varname, variables);
    if(location != variables.end())
    {
        if((*location)->templ_parameters.empty() && *templed)    /* variable accessed as templated but in fact has no templates */
        {
            mcompiler->throw_error(E0020_ACCTNOTP, (*location)->name, NULL);
            psuccess = false;
            return 0;
        }
        return *location;
    }

    /* parameter as a $sign? */
    if(varname[0] == C_DOLLAR)
    {
        varname++;
        int varc = atoi(varname);
        variable* v = variables[varc];

        if(v)
        {
            if(v->templ_parameters.empty() && *templed)    /* variable accessed as templated but in fact has no templates */
            {
                mcompiler->throw_error(E0020_ACCTNOTP, v->name, NULL);
                psuccess = false;
                return 0;
            }
            return v;
        }
    }

    /* variable in the call contexts above our call context? */
    cc = cc->get_father();
    while(cc)
    {
        location = cc->variable_list_has_variable(varname, cc->get_variables());
        if(location != cc->get_variables().end())
        {
            if((*location)->templ_parameters.empty() && *templed)    /* variable accessed as templated but in fact has no templates */
            {
                mcompiler->throw_error(E0020_ACCTNOTP, (*location)->name, NULL);
                psuccess = false;
                return 0;
            }
            return *location;
        }
        cc = cc->get_father();
    }

    return NULL;
}


/**
 * Adds a new parameter to the method. These will go in the
 */
parameter* method::add_parameter(char* pname, char* ptype, int pdimension, const expression_with_location* pexpwloc, call_context*cc, bool& psuccess)
{
    parameter* func_par = new_parameter(this, cc);
    char* indexOfEq = strchr(pname, C_EQ);
    variable* nvar = NULL;

    if(indexOfEq)
    {
        char* afterEq = indexOfEq + 1;
        int res = -1;
        func_par->initial_value = new expression_tree(pexpwloc);
        bool success = true;
        mcompiler->get_interpreter().build_expr_tree(afterEq, func_par->initial_value, this, afterEq, main_cc, &res, pexpwloc, success);
        if(!success)
        {
            psuccess = false;
            return 0;
        }

        *indexOfEq = 0;
    }
    bool success = true;
    nvar = add_new_variable(pname, ptype, pdimension, pexpwloc, success);
    if(!success)
    {
        psuccess = false;
        return 0;
    }

    func_par->value = new_envelope(nvar, BASIC_TYPE_VARIABLE, mcompiler);

    nvar->func_par = func_par;

    if(!strcmp(ptype, "int"))
    {
        func_par->type = BASIC_TYPE_INT;
    }

    func_par->name = mcompiler->duplicate_string(pname);
    parameters.push_back(func_par);
    return func_par;
}

parameter* method::get_parameter(size_t i)
{
    // TODO: This was variables[i].the_parameter ... why?
    if(i < variables.size())
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
void method::feed_parameter_list(char* par_list, const expression_with_location* expwloc, bool& psuccess)
{
    bool success = true;
    std::vector<std::string> entries = string_list_create_bsep(par_list, C_COMMA, mcompiler, success);
    if(!success)
    {
        psuccess = false;
        return;
    }

    std::vector<std::string>::iterator q = entries.begin();
    while(q != entries.end())
    {
        size_t i=0;
        if(!q->empty())
        {
            char* par_type = mcompiler->new_string(q->length());
            char* par_name = mcompiler->new_string(q->length());
            size_t j = 0;
            int modifiable = 0;
            while(i < q->length() && (is_identifier_char((*q)[i]) || C_SQPAR_OP  == (*q)[i]|| C_SQPAR_CL == (*q)[i]) )
            {
                par_type[j++] = (*q)[i++];
            }
            /* now par_type contains the type of the parameter */
            while(i < q->length() && is_whitespace((*q)[i]))
            {
                i++;
            }
            if(i == q->length())
            {
                mcompiler->throw_error(E0009_PARAMISM, par_list);
                psuccess = false;
                return;
            }

            if((modifiable = (C_AND == (*q)[i])))
            {
                i++;
                // TODO: There is no support for this in the bytecode yet.
            }
            j = 0;
            while(i < q->length())
            {
                par_name[j++] = (*q)[i++];
            }
            bool success = true;
            parameter* new_par_decl = add_parameter(trim(par_name, mcompiler), trim(par_type, mcompiler), 1, expwloc, main_cc, success);
            if(!success)
            {
                psuccess = false;
                return;
            }

            /* here we should identify the dimension of the parameter */
            if(strchr(par_type, C_SQPAR_CL) && strchr(par_type, C_SQPAR_OP))
            {
                new_par_decl->simple_value = 0;
            }
        }
        q ++;
    }
}

