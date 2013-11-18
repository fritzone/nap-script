#ifndef _variable_H_
#define _variable_H_

#include "parser.h"
#include "common_structs.h"

/**
 * This structure defines a variable. The following things are characterizing a struct variable:
 * . their name
 * . their type as a string
 * . their dimension, meaning, they can be a vector, or a single struct variable
 * . their type as an int, from the famous BASIC_TYPE... list
 *
 * A variable can be bound together with a C/C++ variable of type long, double and with any dimensions
 * suitable. When binding with multi dimensions, each variable's number* to_interpret's location will
 * point to the specific index in the array passed in.
 */
struct variable
{

    variable(int pdimension, int type);

    /* the name of the variable */
    char *name;

    /* the size of the variable (ie. dimension) */
    int dimension;

    /* the type of the variable */
    char *c_type;

    /* the type of the variable */
    int i_type;

    /* the dimension definition of this variable */
    struct multi_dimension_def *mult_dim_def;

    /* the number of indexes if this is a multi-dim variable */
    int multi_dim_count;

    /* populated with NULL if this is not a function parameter, or the actual function parameter if it is */
    struct parameter *func_par;

    /* the template parameters of the variable if any */
    std::vector<parameter*> templ_parameters;

    /* used by the templ_parameters above */
    std::vector<variable*> templ_variables;

    /* whether this variable is static or not*/
    char static_var;

    /* whether this variable represents an environment variable or not */
    char environment_variable;

    /* 1 if this variable has dynamic dimensions, 0 if not*/
    char dynamic_dimension;

    /* the call context in which this variable is to be found */
    call_context *cc;
};

/**
 * Creates a new multi dimension index
 * @param indx_id - is the actual index
 * @return a new structure with this index in it
 */
multi_dimension_index* new_multi_dimension_index(const char* indx_id, const nap_compiler* _compiler);

/**
 * Creates a new templated variable reference for the give variable
 * and parameters
 */
variable_template_reference* new_variable_template_reference(variable* var, std::vector<parameter*> pars, const nap_compiler *_compiler);

/**
 * @brief variable_resolve_templates Resolves the templates of the given variable
 * @param the_variable - this is the variable
 * @param the_method - the method in which this variable is
 * @param cc - the "closer" call context of the variable
 * @param expwloc - the expression with location object
 */
void variable_resolve_templates(variable* the_variable, method* the_method, call_context* cc, const expression_with_location* expwloc);

#endif
