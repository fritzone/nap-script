#ifndef _CALL_CTX_H_
#define _CALL_CTX_H_

#include "method.h"
#include "type.h"

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
struct class_declaration;

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
    /* the type of the call context: 0 - global, 1 - named*/
    int type;

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
    
    /* the classes that are defined in this call context */
    vector<struct class_declaration*>* classes;
    
    /* the interfaces that are defined in this call context */
    vector<struct class_declaration*>* interfaces;
};

struct class_declaration : public call_context
{
    class_declaration* parent_class;
    vector<class_declaration*> implemented_interfaces;
};

extern call_context* global_cc;

call_context*       call_context_create                     (int type, const char* name, method* the_method, call_context* father);
class_declaration*  class_declaration_create                (const char* name, call_context* father);
method_list*        call_context_add_method                 (call_context* cc, method* the_method);
method*             call_context_get_method                 (call_context* cc, const char* name);
variable*           call_context_add_variable               (call_context* cc, const char* name, const char* type, int dimension, const struct expression_with_location* expwloc);
void                call_context_add_compiled_expression    (call_context* cc, const struct expression_tree* co_expr, const char* expr);
long                call_context_add_label                  (call_context* cc, long position, const char* name);
bytecode_label*     call_context_add_break_label            (call_context* cc, long position, const char* name);
expression_tree*    call_context_add_new_expression         (call_context* cc, const char* expr, const expression_with_location* expwloc);
class_declaration*  call_context_get_class_declaration      (const call_context* cc, const char* required_name);
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
