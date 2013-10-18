#ifndef _METHOD_H_
#define _METHOD_H_

#include "parser.h"

/* forward declaration, we need this in the method */

struct call_frame_entry;
struct parameter_list;
struct call_context_list;
struct call_context;
struct call_frame_entry;
struct variable_definition_list;
struct variable_definition;
struct variable;
struct class_declaration;

/* the main call frame, and two helper variables. First one to point to the last element, second one to the last - 1*/
//static struct call_frame_list* main_call_frame = 0, *cflq = 0, *cflq1 = 0;

/**
 * Creates a new method 
 * @param name the name of the new method
 */
method* new_method(char* name, char* return_type, call_context* cc);
constructor_call* new_constructor_call(char* name, call_context* cc);

/**
 * Frees the memory allocated by the given method
 */
void method_destroy(method* the_method);

/**
 * Adds a new expression to the method
 */
struct expression_tree* method_add_new_expression(method* the_method,  char* expr, call_context* cc);

/**
 * Adds a new variable to the method ...
 */
variable* method_add_new_variable(method* the_method,  char* name,  char* type, int dimension, const expression_with_location* expwloc);

/**
 * Runs the method
 */
struct envelope* method_run_method(method* the_method, parameter_list* calling_parameters);

/**
 * Adds a new parameter to the method
 * @param the_method - the method to be worked on
 * @param name - the name of the parameter
 * @param type - the type of the parameter (as string)
 * @param modifiable - if the parameter is a reference or not (C++ rulez :) )
 */
struct parameter* method_add_parameter(method* the_method,  char* name,  char* type, int dimension, int modifiable, const expression_with_location* expwloc);

/**
 * Feeds in the given parameter list to the method for it to create its parameter structures
 */
void method_feed_parameter_list(method* the_method, char* par_list, const expression_with_location* expwloc);

/**
 * Gets the variable from the method
 */
variable* method_has_variable(method* the_method, call_context* cc, char* varname, int* templed, int* env_var);

struct parameter* method_get_parameter(method* the_method, const char* varname);
struct parameter* method_get_parameter(method* the_method, int i);

#endif
