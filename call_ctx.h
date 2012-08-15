#ifndef _CALL_CTX_H_
#define _CALL_CTX_H_

#include "method.h"

#include <vector>
using namespace std;

/**
 * Class, describing a label, a jump location.
 */
struct bytecode_label
{
	/* the name of the label */
	char* name;

	/* the location of the label in the bytecode stream */
	long bytecode_location;

	/* the type of the location, can be 0 if this is just a plain label, 1 if this is a "break" location label, 2 if this is a "continue" location label */
	int type;
};

/*
 * The call context is something like a namespace and/or code-block ... See the code
 * below, to understand:
 *
 * // by default a 'global' namespace starts here, with name 'global'.
 * int myGlobalWarming;		// global variable, will be visible everywhere in the program
 * namespace my_precious { // new call context starts here with name 'my_precious'
 * int x;
 * int func()
 * {				// another call context, which can see the x from above
 * int a;			
 *     {			// just a local nameless call context
 *     int b;		// b is a local variable, a still visible above
 *     }			// b is not visible anymore, it was destroyed at the end of the nameless call context
 * }
 * }				// x is destroyed here by default
 */

struct call_context;

/**
 * This holds a list of call contexts.
 */
struct call_context_list
{
	struct call_context_list* next;
	struct call_context* ctx;
};

/**
 * The call context is something like a namespace, or a class... basically its role is to group the methods
 * that belong to the same call context, and makes visible to each other. There is always a 'global' call context.
 * Variables by default belong to a call cotnext and they are destroyed when the call context ends
 */
struct call_context
{
	/* the type of the call context: 0 - global, 1 - named, 2 - class*/
	char type;

	/* the name of the call context */
	char* name;

	/* the methods of this call context */
	struct method_list* methods;

	/* the list of variable that have been defined in this call context */
	struct variable_list* variables;

	/* contains the list of expressions */
	struct expression_tree_list* expressions;

	/* these are the child call contexts */
	struct call_context_list* child_call_contexts;

	/* the father call context of this */
	struct call_context* father;

	/* if the call context is from a method this is that method */
	struct method* ccs_method;

	/* this is a vector of label locations for this*/
	vector<struct bytecode_label*>* labels;

	struct bytecode_label* break_label;
};

extern call_context* global_cc;

/**
 * Creates a new call context
 * @param int - the type of the call context
 * @param name - the name of the call context
 * @param the_method - the method in which the call context can be found. NULL if global
 * @param father - the father call context of this. NULL if this is the global CC
 * @return the newly created and initialized call context
 */
struct call_context* call_context_create(int type, const char* name, struct method* the_method, struct call_context* father);

/**
 * Adds a new method to the call context
 * @param cc - the call context
 * @param the_method - the method to add
 * @return the position of the new method
 */
struct method_list* call_context_add_method(struct call_context* cc, method* the_method);

/**
 * Returns the method from the given call context if any, NULL if nothing was found
 * @param cc - the call context
 * @param name - the name of the method we're looking fo
 * @return the method if found, NULL if nothing was found
 */
struct method* call_context_get_method(struct call_context* cc, const char* name);

/**
 * Adds a new variable to the call context
 * @param cc - the call context to update
 * @param name - the name of the variable
 * @param type - the type
 * @param dimension - the dimension of the variable
 */
struct variable* call_context_add_variable(struct call_context* cc, const char* name, const char* type, int dimension, const struct expression_with_location* expwloc);

/**
 * Adds a new compiled expression to the given call context
 * @param cc - the call cotnext to update
 * @param co_expr - the compiled expression
 * @param expr - the expression as string
 */
void call_context_add_compiled_expression(struct call_context* cc, const struct expression_tree* co_expr, const char* expr);

/**
 * Adds a label into the bytecode compilation of this call context
 * This method creates a new bytecode_label structure, with the default label type (0)
 * @param cc - the call context
 * @param position - the position of the label. Initially this is -1, the bytecode generator will generate the correct position
 * @param name - the name of the label (mostly used when generating assembly output)
 */
long call_context_add_label(struct call_context* cc, long position, const char* name);

/**
 * Adds a label into the bytecode compilation of this call context
 * This method creates a new bytecode_label structure, with the label type set to 1, ie. a location where the BREAK of a while or a for will jump
 * @param cc - the call context
 * @param position - the position of the label. Initially this is -1, the bytecode generator will generate the correct position
 * @param name - the name of the label (mostly used when generating assembly output)
 * @returns the bytecode label object of the location where the break will jump
 */
struct bytecode_label* call_context_add_break_label(struct call_context* cc, long position, const char* name);

/**
 * Adds a new expression to the given call context
 */
struct expression_tree* call_context_add_new_expression(struct call_context* cc,
                const char* expr,
                const struct expression_with_location* expwloc);

/**
 * Runs the global call context. This is equivalent to:
 * 1. create the static objects as defined in the call context, in the order they are found (if possible)
 * 2. create the global objects
 * 2. find the method int main(string[] parameters) and execute it
 */
void call_context_compile(struct call_context* cc, char* envp[]);

/**
 * Runs the given call context as being part of a full method run, or as a newly started call context.
 * This method of running does not run the call contexts of 'if', 'while', etc...
 * This can not (may not) deal with keywords break/continue.
 */
struct envelope* call_context_run_complete(struct call_context* cc);

/**
 * Runs this call context, the call context is supposed to be an "inner" call cotnext (mostly nameless call cotnext)
 */
void call_context_run_inner(struct call_context* cc, int level, int reqd_type, int forced_mov);

/**
 * Runs the given call context (cc) when the given call context is an if/else/while etc... call context
 * The call context cc is placed in the parente call context (parent_cc) which can be for example a method's call context
 */
envelope* call_context_run_simple(struct call_context* cc, struct call_context* parent_cc);

/**
 * Adds a new call context to the list of child call contexts
 */
void call_context_add_child_call_context(struct call_context* father, struct call_context* child);

/**
 * Returns the given variable object from the call_context, or NULL if nothing found
 */
variable* call_context_get_variable(struct call_context* cc, const char* v_name);

/**
 * Creates a new label structure and inserts it into the vector of the bytecode labels of this call_context
 * @param break_label is 1 if the label is a destination for the break of the call context
 */
struct bytecode_label* call_context_provide_label(struct call_context* cc, int break_label);

#endif
