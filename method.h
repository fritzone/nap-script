#ifndef _METHOD_H_
#define _METHOD_H_

#include "parser.h"

/* forward declaration, we need this in the method */

struct call_frame_entry;
struct variable_list;
struct parameter_list;
struct call_context_list;
struct call_context;
struct call_frame_entry;
struct variable_definition_list;
struct variable_definition;
struct variable;

/**
 * Contains the definition of a method
 */
struct method
{
	/* the name of the method */
	char* name;

	/* the return type of the function */
	char* return_type;

	/* the list of struct variables that are defined in the method. This list is used in two situations:
	 * 1. in the interpret phase, when the code is built up, this is used as references to find the variables.
	 * 2. in the running phase, but only for static variables.
	 */
	variable_list* variables;

	/* the parameters of the method */
	parameter_list* parameters;

	/* the call contexts this method is taking part from */
	call_context_list* contained_ins;

	/* this is the call context of the method, created when the method is created */
	call_context* main_cc;

	/* the current call frame. stores all the variables of the method which are not static. At each calling of a method
	 * this variable gets populated with the call frame that has been constructed for the current stage
	 */
	call_frame_entry* cur_cf;

	/* the definition location 0 - normal, 1 - extern*/
	int def_loc;

    int ret_type;
};

/**
 * Contains a list of methods accessible from a given context
 */
struct method_list
{
	/* link to the next element */
	method_list* next;

	/* the actual method*/
	method* the_method;
};



/**
 * Inserts in the given method list a new method
 * @param list - the address of the list we're inserting into
 * @param meth - the method
 */
method_list* method_list_insert(method_list** list, method* meth);

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
	method* the_method;

	/* this are the parameters passed to the method */
	parameter_list* parameters;

	/* holds the previous call frame of the method... just in case*/
	call_frame_entry* previous_cf;
};

/**
 * Creates a new call frame entry
 */
struct call_frame_entry* new_call_frame_entry(struct method* the_method, struct parameter_list* pars);

/**
 * Represents the list of active call frames (Call Stack). For easier access this is a double linked list
 */
struct call_frame_list
{
	struct call_frame_list* next;
	struct call_frame_list* prev;
	struct call_frame_entry* entry;
};

/**
 * Create a new call frame list object
 */
struct call_frame_list* new_call_frame_list(struct call_frame_entry* entry);

/* the main call frame, and two helper variables. First one to point to the last element, second one to the last - 1*/
//static struct call_frame_list* main_call_frame = 0, *cflq = 0, *cflq1 = 0;

/**
 * Creates a new struct method 
 * @param name the name of the new struct method
 */
struct method* new_method(char* name, char* return_type, struct call_context* cc);

/**
 * Frees the memory allocated by the given struct method
 */
void method_destroy(struct method* the_method);

/**
 * Adds a new expression to the struct method
 */
struct expression_tree* method_add_new_expression(struct method* the_method,  char* expr, struct call_context* cc);

/**
 * Adds a new struct variable to the struct method ...
 */
struct variable* method_add_new_variable(struct method* the_method,  char* name,  char* type, int dimension, const expression_with_location* expwloc);

/**
 * Runs the struct method
 */
struct envelope* method_run_method(struct method* the_method, struct parameter_list* calling_parameters);

/**
 * Adds a new parameter to the method
 * @param the_method - the method to be worked on
 * @param name - the name of the parameter
 * @param type - the type of the parameter (as string)
 * @param modifiable - if the parameter is a reference or not (C++ rulez :) )
 */
struct parameter* method_add_parameter(struct method* the_method,  char* name,  char* type, int dimension, int modifiable, const expression_with_location* expwloc);

/**
 * Feeds in the given parameter list to the method for it to create its parameter structures
 */
void method_feed_parameter_list(struct method* the_method, char* par_list, const expression_with_location* expwloc);

/**
 * Gets the struct variable from the struct method
 */
struct variable* method_has_variable(struct method* the_method, struct call_context* cc, char* varname, int* templed, int* env_var);

struct parameter* method_get_parameter(struct method* the_method, const char* varname);
struct parameter* method_get_parameter(struct method* the_method, int i);

#endif
