#include "method.h"
#include "utils.h"
#include "call_ctx.h"
#include "tree.h"
#include "interpreter.h"
#include "number.h"
#include "consts.h"
#include "envelope.h"
#include "throw_error.h"
#include "bt_string.h"
#include "parametr.h"
#include "type.h"
#include "sys_brkp.h"
#include "evaluate.h"
#include "variable.h"
#include "parametr.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/**
 * Creates a new method
 */
method::method(char* name, char* return_type, call_context* cc)
{
    call_context* method_main_cc = NULL;
    char* method_main_cc_name = new_string(strlen(name) + strlen(cc->name) + 3); /* will contain: parent_name::function_name*/
    strcpy(method_main_cc_name, cc->name);
    strcat(method_main_cc_name, STR_CALL_CONTEXT_SEPARATOR);
    strcat(method_main_cc_name, name);

    method_name = duplicate_string(name);
    if(return_type && strstr(return_type, "extern"))
    {
        /* if this method is a method defined somewhere else ... such as your C++ application */
        char* after_ext = return_type + 6;
        after_ext = trim(after_ext);
        return_type = duplicate_string(after_ext);
        def_loc = DEF_EXTERN;
    }
    else
    if(return_type)
    {
        return_type = duplicate_string(return_type);
    }
    method_main_cc = call_context_create(CALL_CONTEXT_TYPE_METHOD_MAIN, method_main_cc_name, this, cc);
    main_cc = method_main_cc;
}

constructor_call::constructor_call(char* name, call_context* cc) : method(name, 0, cc)
{
    return_type = 0;
    the_class = call_context_get_class_declaration(cc, name);
}

/**
 * Adds a new variable.
 * If the dimensions is 1
 */
variable* method_add_new_variable(method* the_method, char* name, char* type, int dimension, const expression_with_location* expwloc)
{
char* new_name = trim(name);
    if(strlen(name) > 0 && !is_valid_variable_name(name))
    {
        throw_error(E0018_INVIDENT, name, NULL);
    }
    return variable_list_add_variable(new_name, type, dimension, the_method->variables, the_method, the_method->main_cc, expwloc);
}

/**
 * Returns the  variable from this  method. The algorithm is the following:
 * 1. find the variables of the call context passed in
 * 2. find the variables of the cc's father, and its father, etc...
 * 3. if reached to the method's main cc find variable in all the CC's above
 * 4. find the variables of the method (meaning: parameters)
 */
 variable* method_has_variable(method* the_method, call_context* cc, char* varname, int* templed, int* env_var)
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

    std::vector<variable*>::const_iterator location = variable_list_has_variable(varname, cc->variables);
    if(location != cc->variables.end())
    {
        return *location;
    }

    if(the_method)         /* after this, whether this is a parameter ? */
    {
        location = variable_list_has_variable(varname, the_method->variables);
        if(location != the_method->variables.end())
        {
            if(!(*location)->templ_parameters && *templed)    /* variable accessed as templated but in fact has no templates */
            {
                throw_error(E0020_ACCTNOTP, (*location)->name, NULL);
            }
            return *location;
        }

        /* parameter as a $sign? */
        if(varname[0] == C_DOLLAR)
        {
            varname++;
            int varc = atoi(varname);
            variable* v = the_method->variables[varc];

            if(v)
            {
                if(!v->templ_parameters && *templed)    /* variable accessed as templated but in fact has no templates */
                {
                    throw_error(E0020_ACCTNOTP, v->name, NULL);
                }
                return v;
            }
        }
    }

    /* variable in the call contexts above our call context? */
    cc = cc->father;
    while(cc)
    {
        location = variable_list_has_variable(varname, cc->variables);
        if(location != cc->variables.end())
        {
            if(!(*location)->templ_parameters && *templed)    /* variable accessed as templated but in fact has no templates */
            {
                throw_error(E0020_ACCTNOTP, (*location)->name, NULL);
            }
            return *location;
        }
        cc = cc->father;
    }

    return NULL;
}


/**
 * Adds a new parameter to the method
 */
parameter* method_add_parameter(method* the_method, char* name, char* type, int dimension, int modifiable, const expression_with_location* expwloc)
{
parameter* func_par = new_parameter(the_method);
char* indexOfEq = strchr(name, C_EQ);
variable* nvar = NULL;
parameter_list* flist = NULL, *q = NULL;
    if(indexOfEq)
    {
    char* afterEq = indexOfEq + 1;
    int res = -1;
        func_par->initial_value = new_expression_tree(expwloc);
        build_expr_tree(afterEq, func_par->initial_value, the_method, afterEq, the_method->main_cc, &res, expwloc);
        *indexOfEq = 0;
    }
    nvar = method_add_new_variable(the_method, name, type, dimension, expwloc);
    nvar->func_par = func_par;
    flist = alloc_mem(parameter_list,1);
    q = the_method->parameters;

    func_par->value = new_envelope(nvar, BASIC_TYPE_VARIABLE);
    func_par->modifiable = modifiable;
    flist->param = func_par;

    if(!strcmp(type, "int"))
    {
        func_par->type = BASIC_TYPE_INT;
    }

    func_par->name = duplicate_string(name);

    if(NULL == q)
    {
        the_method->parameters = flist;
        return func_par;
    }
    while(q->next) q=q->next;
    q->next = flist;

    return func_par;
}

struct parameter* method_get_parameter(method* the_method, int i)
{
    if(the_method)
    {
        return the_method->variables[i]->func_par;
    }
    return 0;
}

struct parameter* method_get_parameter(method* the_method, const char* varname)
{
    std::vector<variable*>::const_iterator location;

    if(the_method)
    {
        location = variable_list_has_variable(varname, the_method->variables);
        if(location != the_method->variables.end())
        {
            if(!(*location)->templ_parameters)    /* variable accessed as templated but in fact has no templates */
            {
                throw_error(E0020_ACCTNOTP, (*location)->name, NULL);
            }
            return (*location)->func_par;
        }

        /* parameter as a $sign? */
        if(varname[0] == C_DOLLAR)
        {
            varname++;
            int varc = atoi(varname);
            variable* var = the_method->variables[varc];
            if(var)
            {
                if(!var->templ_parameters)    /* variable accessed as templated but in fact has no templates */
                {
                    throw_error(E0020_ACCTNOTP, var->name, NULL);
                }
                return var->func_par;
            }
        }
    }
    return 0;
}

/**
 * Populates the parameters of this method with the definitions from the string
 */
void method_feed_parameter_list( method* the_method, char* par_list, const expression_with_location* expwloc)
{
 string_list* entries = string_list_create_bsep(par_list, C_COMMA), *q ;
    q = entries;
    while(q)
    {
    int i=0;
    char* par_type = new_string(q->len);
    char* par_name = new_string(q->len);
        if(q->len > 0)
        {
        int j = 0;
        int modifiable = 0;
            while(i < q->len && (is_identifier_char(q->str[i]) || C_SQPAR_OP  == q->str[i]|| C_SQPAR_CL == q->str[i]) )
            {
                par_type[j++] = q->str[i++];
            }
            /* now par_type contains the type of the parameter */
            while(i < q->len && is_whitespace(q->str[i])) i++;
            if((modifiable = (C_AND == q->str[i]))) i++;
            j = 0;
            while(i < q->len)
            {
                par_name[j++] = q->str[i++];
            }
        parameter* new_par_decl = method_add_parameter(the_method, trim(par_name), trim(par_type), 1, modifiable, expwloc);
            /* here we should identify the dimension of the parameter */
            if(strchr(par_type, C_SQPAR_CL) && strchr(par_type, C_SQPAR_OP))
            {
                new_par_decl->simple_value = 0;
            }
        }
        q=q->next;
    }
}

