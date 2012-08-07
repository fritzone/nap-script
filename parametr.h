#ifndef _PARAMETR_H_
#define _PARAMETR_H_

/*
 * Declarations of function parameter
 */

struct method;
struct envelope;
struct expression_tree;

/**
 * This struct represents a function's parameter. The implementation of this feature is like:
 * each parameter in its initial definition stage is a variable; When the function is called this variable
 * is initialized to the value it was called with
 */
struct parameter
{
	/* 1 if the parameter was sent as a reference */
	int modifiable;

	/* the name of the parameter (as used on te function's side). This is not used on the client side*/
	char* name;

	/* the actual implementation of the parameter. it will contain a variable */
	envelope* value;

	/* the expression of this parameter as read from the source file. Used at function calling */
	expression_tree* expr;

	/* the initial value of the parameter, like: int f (int a=10, b)*/
	expression_tree* initial_value;

	/* 1 if the value is simpl (meaning, no dimensions) 0 if the value is not so simple, meaning dimensions */
	int simple_value;
};


/** 
 * A list parameters  
 */
struct parameter_list
{
	/* the link to the next */
	parameter_list* next;

	/* the atual parameter */
	parameter* param;
};



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
struct envelope* parameter_list_get_named_parameters_value(const parameter_list* pl, const char* name);

/**
 * Returns the initial value for the parameter with the given name.
 */
struct envelope* parameter_list_get_named_parameters_initial_value(const parameter_list* pl, const char* name, const method* the_method, struct call_context* cc);

/**
 * Returns the very basic type of the parameter
 */
int parameter_list_get_named_parameters_type(const parameter_list* pl, const char* name);

#endif
