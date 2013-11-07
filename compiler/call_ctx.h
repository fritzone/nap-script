#ifndef _CALL_CTX_H_
#define _CALL_CTX_H_

#include "method.h"
#include "type.h"

#include <vector>


/**
 * @brief call_context_add_variable adds a variable to the call context
 * @param cc - the call context
 * @param name - the name of the variable
 * @param type - the tye of the variable
 * @param dimension - the dimension of the variable
 * @param expwloc - the location of the piece of code for this
 * @return - the newly created variable
 */
variable *call_context_add_variable(call_context *cc,
                                    const char *name,
                                    const char *type,
                                    int dimension,
                                    const expression_with_location *expwloc);

/**
 * @brief call_context_add_compiled_expression adds a compiled expression to
 * the call context
 * @param cc - the call context
 * @param co_expr - the compiled expression
 * @param expr - the textual expression
 */
void call_context_add_compiled_expression(call_context *cc,
                                          const expression_tree *co_expr,
                                          const char *expr);

/**
 * @brief call_context_add_label adds a label to the call context
 * @param cc - the call context
 * @param position - the poisition where to add it
 * @param name - the name of the label
 * @return how many labels are there in this call context
 */
long call_context_add_label(call_context *cc, long position, const std::string &name);

/**
 * @brief call_context_add_break_label add a "break" label to this call context
 * @param cc - the call context
 * @param position - where to add the label
 * @param name - the name of the label
 * @return - the bytecode label object of the new label
 */
bytecode_label *call_context_add_break_label(call_context *cc,
                                             long position,
                                             const std::string &name);

/**
 * @brief call_context_add_new_expression add a new expression to this call context
 * @param cc - the call context
 * @param expr - the expression tah will be addedd
 * @param expwloc - the location of the expression in the file
 * @return the new expression_tree object
 */
expression_tree *call_context_add_new_expression(call_context *cc,
                                                 const char *expr,
                                                 const expression_with_location *expwloc);

/**
 * @brief call_context_get_class_declaration return a clas declaration from the
 * given call context if any
 * @param cc - the call context
 * @param required_name - the name of the call context
 * @return
 */
class_declaration *call_context_get_class_declaration(const call_context *cc,
                                                      const char *required_name);
/**
 * Runs the global call context. This is equivalent to:
 * 1. create the static objects as defined in the call context, in the order they are found (if possible)
 * 2. create the global objects
 * 2. find the method int main(string[] parameters) and execute it
 */
void call_context_compile(call_context *cc);

/**
 * Runs the given call context as being part of a full method run, or as a newly started call context.
 * This method of running does not run the call contexts of 'if', 'while', etc...
 * This can not (may not) deal with keywords break/continue.
 */
struct envelope *call_context_run_complete(call_context *cc);

/**
 * Runs this call context, the call context is supposed to be an "inner" call cotnext (mostly nameless call cotnext)
 */
void call_context_run_inner(call_context *cc, int level, int reqd_type, int forced_mov);

/**
 * Returns the given variable object from the call_context, or NULL if nothing found
 */
variable *call_context_get_variable(call_context *cc, const char *v_name);

/**
 * Creates a new label structure and inserts it into the vector of the bytecode labels of this call_context
 * @param break_label is 1 if the label is a destination for the break of the call context
 */
bytecode_label *call_context_provide_label(call_context *cc);

#endif
