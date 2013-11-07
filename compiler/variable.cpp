#include "variable.h"
#include "number.h"
#include "bt_string.h"
#include "type.h"
#include "method.h"
#include "hash.h"
#include "interpreter.h"
#include "utils.h"
#include "parametr.h"
#include "notimpl.h"
#include "sys_brkp.h"
#include "envelope.h"
#include "consts.h"
#include "throw_error.h"
#include "parametr.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

/**
 * Creates a new  variable
 */
variable::variable(int pdimension, int type)
{
    if(pdimension<1)
    {
        throw_error(STR_INVALID_DIMENSION, NULL);
    }

    dimension = pdimension;
    multi_dim_count = 1;
    i_type = type;

    name = NULL;
    c_type = NULL;
    mult_dim_def = NULL;
    func_par = NULL;
    templ_parameters = NULL;
    static_var = 0;
    environment_variable = 0;
    dynamic_dimension = 0;
    cc = 0;
}

/**
 * Creates a new multi-dimension index object
 * @return a new object, inited to 0
 */
multi_dimension_index* new_multi_dimension_index(const char* indx_id)
{
multi_dimension_index* tmp = alloc_mem(multi_dimension_index,1);
    tmp->id = duplicate_string(indx_id);
    return tmp;
}

/**
 * Adds a new template variable to the variable
 */
static variable* variable_add_new_template_variable(variable* the_variable, const char* name, const char* type, method* the_method, call_context* cc, const expression_with_location* expwloc)
{
char* new_name = trim(name);
    if(strchr(new_name, C_PAR_CL) || strchr(new_name, C_PAR_OP)) /* not allowed to have parantheses in the variable name */
    {
        throw_error(E0029_PARANOTALL, new_name, NULL);
    }
    return variable_list_add_variable(new_name, type, 1, the_variable->templ_variables, the_method, cc, expwloc);
}


/**
 * Adds a template parameter to this very variable
 * @param the_variable - is the variable we are updating
 * @param name - the name of the template parameter we are adding we are working with
 * @param type - is the type of the template parameter
 * @param the_method - the method we are working in (if any)
 * @param cc - the cc in which we are located
 * @return the newly created parameter object
 */
static parameter* variable_add_template_parameter(variable* the_variable, const char* name, const char* type, method* the_method, call_context* cc, const expression_with_location* expwloc)
{
    if(BASIC_TYPE_INT == the_variable->i_type || BASIC_TYPE_REAL == the_variable->i_type)
    {
        throw_error(E0030_PARMNOTALL, the_variable->name, NULL);
    }
    parameter* func_par = new_parameter(the_method);
    char *name_dup = duplicate_string(name);
    char* indexOfEq = strchr(name_dup, C_EQ);
    variable* nvar = NULL;
    parameter_list* flist = NULL, *q = NULL;

    if(indexOfEq)
    {
    const char* afterEq = indexOfEq + 1;
    int res = -1;
        func_par->initial_value = new_expression_tree(expwloc);
        build_expr_tree(afterEq, func_par->initial_value, the_method, afterEq, cc, &res, expwloc);
        *indexOfEq = 0;
    }

    name = trim(name);

    nvar = variable_add_new_template_variable(the_variable, name, type, the_method, cc, expwloc);
    nvar->func_par = func_par;
    flist = alloc_mem(parameter_list,1);
    q = the_variable->templ_parameters;

    func_par->value = new_envelope(nvar, BASIC_TYPE_VARIABLE);
    func_par->modifiable = 0;
    func_par->name = duplicate_string(name);

    flist->param = func_par;

    if(NULL == q)
    {
        the_variable->templ_parameters = flist;
        return func_par;
    }
    while(q->next) q=q->next;
    q->next = flist;
    return func_par;
}

/**
 * Populates the template parameter list of the variable with values grabbed from the given string
 * @param the_variable - the variable which will be populated
 * @param par_list - the parameter list as is
 * @param the_method - the method we are working on
 * @param cc - the cc we are palced in
 */
void variable_feed_parameter_list(variable* the_variable, char* par_list, method* the_method, call_context* cc, const expression_with_location* expwloc)
{
    //printf("\n\nFeeding parameter list for variable [%s] with [%s]\n\n", the_variable->name, par_list);
    string_list* entries = string_list_create_bsep(par_list, C_COMMA), *q ;
    q = entries;
    while(q)
    {
    int i=0;
    char* par_type = new_string(q->len);
    char* par_name = new_string(q->len);
    int j = 0;
        while(i < q->len && is_identifier_char(q->str[i]))    /* & and [] are not allowed in variable templ. parameter*/
        {
            par_type[j++] = q->str[i++];
        }
        /* now par_type contains the type of the parameter */
        while(i < q->len && is_whitespace(q->str[i])) i++;
        if(C_AND == q->str[i])
        {
            throw_error(E0031_NOREFHERE, the_variable->name, NULL);
        }
        if(C_SQPAR_CL == q->str[i] || C_SQPAR_OP == q->str[i])
        {
            throw_error(E0032_NOARRHERE, the_variable->name, NULL);
        }

        j = 0;
        while(i < q->len)    par_name[j++] = q->str[i++];
    parameter* new_par_decl = variable_add_template_parameter(the_variable, trim(par_name), trim(par_type), the_method, cc, expwloc);

        /* here we should identify the dimension of the parameter */
        if(strchr(par_type, C_SQPAR_CL) && strchr(par_type, C_SQPAR_OP))
        {
            new_par_decl->simple_value = 0;
        }

        q=q->next;
    }
}

void variable_resolve_templates(variable* the_variable,  method* the_method,  call_context* cc, const expression_with_location* expwloc)
{
char* templ_pars = strchr(the_variable->name, C_PAR_OP);
    if(!templ_pars)
    {
        return;
    }
    if(!strchr(the_variable->name, C_PAR_CL))
    {
        throw_error(E0033_INCORRDEF, the_variable->name, NULL);
    }
    *templ_pars = 0;
    the_variable->name = trim(the_variable->name);
    templ_pars ++;
int tplen = strlen(templ_pars);
    templ_pars[tplen - 1] = 0;

    variable_feed_parameter_list(the_variable, templ_pars, the_method, cc, expwloc);
}

/**
 * Creates a new variable template reference object
 */
variable_template_reference* new_variable_template_reference(variable* var, parameter_list* pars)
{
variable_template_reference* tmp = alloc_mem(variable_template_reference,1);
    tmp->templ_pars = pars;
    tmp->the_variable = var;
    return tmp;
}
