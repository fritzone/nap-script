#ifndef _variable_H_
#define _variable_H_

#include "parser.h"
#include "common_structs.h"


/**
 * Creates a new call frame entry
 */
struct call_frame_entry* new_call_frame_entry(struct method* the_method, struct parameter_list* pars);

/**
 * Creates a new multi dimension index
 */
struct multi_dimension_index* new_multi_dimension_index(const char*);


/**
 * Creates a new templated variable reference for the give variable
 * and parameters
 */
struct variable_template_reference* new_variable_template_reference(variable* var, parameter_list* pars);

/**
 * Creates a new struct variable and returns it.
 * @param dimension - the dimension of the variable
 * @param type - the type of the variable
 * @return a newly initialized variable, with dimension and type initialized
 */
struct variable* new_variable(int dimension, int type);

/**
 * Returns the basic type ofthe variable
 */
int variable_get_basic_type(const variable* const var);

/**
 * Resolves the templates of the given variable
 */
void variable_resolve_templates(struct variable* the_variable, struct method* the_method, struct call_context* cc, const expression_with_location* expwloc);

#endif
