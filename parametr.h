#ifndef _PARAMETR_H_
#define _PARAMETR_H_

#include "common_structs.h"

/**
 * Creates a new, empty function parameter
 * @return the newly created parameter
 */
struct parameter* new_parameter();

/**
 * Adds a new parameter in the function parameter list
 * @param fpl - is the address of a parameter list 
 * @param par - is the parameter that will be added to the list
 */
void parameter_list_add_parameter(parameter_list** fpl, parameter* par);

/**
 * Returns the value of the parameter with the given name
 * @param pl - the parameter list
 * @param name - the name of the parameter whose value is retrieved
 */
struct envelope* parameter_list_get_named_parameters_value(
                                    const parameter_list* pl, 
                                    const char* name);

/**
 * Returns the initial value for the parameter with the given name.
 */
struct envelope* parameter_list_get_named_parameters_initial_value(
                                    const parameter_list* pl, const char* name, 
                                    const method* the_method, 
                                    struct call_context* cc);

/**
 * Returns the very basic type of the parameter
 */
int parameter_list_get_named_parameters_type(const parameter_list* pl, 
                                             const char* name);

#endif
