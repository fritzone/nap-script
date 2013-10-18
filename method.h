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

/**
 * Contains the definition of a method
 */
struct method
{

    /**
     * @brief method Constructor
     * @param name
     * @param return_type
     * @param cc
     */
    method(char* name, char* return_type, call_context* cc);

    /* the name of the method */
    char *name;

    /* the return type of the function */
    char *return_type;

    /* the list of struct variables that are defined in the method. This list is used in two situations:
     * 1. in the interpret phase, when the code is built up, this is used as references to find the variables.
     * 2. in the running phase, but only for static variables.
     */
    std::vector<variable*> variables;

    /* the parameters of the method */
    parameter_list *parameters;

    /* the call contexts this method is taking part from */
    call_context_list *contained_ins;

    /* this is the call context of the method, created when the method is created */
    call_context *main_cc;

    /* the current call frame. stores all the variables of the method which are not static. At each calling of a method
     * this variable gets populated with the call frame that has been constructed for the current stage
     */
    call_frame_entry *cur_cf;

    /* the definition location 0 - normal, 1 - extern*/
    int def_loc;

    int ret_type;
};

/**
 * A constructor call structure for the "new" keyword
 **/
struct constructor_call : public method
{
    constructor_call(char* name, call_context* cc);

    class_declaration *the_class;
};

/**
 * The call frame structure represents the "stack" way the functions are being called.
 * Each time a function is called it puts it "signature" (the address of the_method)
 * and the parameters onto a global call_frame variable. There is always required to be
 * at least one element on the global call_frame stack (the main application).
 * Also this structure holds the variable of the method being called at the current stage.
 * If the main application exits then the call_frame will be empty.
 * (The best of all: This can be used at a (very) later stage when developing threading. Each
 * thread like object will have its own call frame, and they'll be free to run independently)
 */
struct call_frame_entry
{

    /* this is the method that was called */
    method *the_method;

    /* this are the parameters passed to the method */
    parameter_list *parameters;

    /* holds the previous call frame of the method... just in case*/
    call_frame_entry *previous_cf;

    call_frame_entry(method *the_method, parameter_list *pars)
    {
        parameters = pars;
        the_method = the_method;
        previous_cf = the_method->cur_cf;
    }
};

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

parameter* method_get_parameter(method* the_method, const char* varname);
parameter* method_get_parameter(method* the_method, int i);

#endif
