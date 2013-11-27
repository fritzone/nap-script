#ifndef _PARAMETER_H_
#define _PARAMETER_H_

class expression_tree;
struct envelope;
struct call_context;
struct method;

#include <string>

/**
 * This struct represents a function's parameter. The implementation of this feature is like:
 * each parameter in its initial definition stage is a variable; When the function is called this variable
 * is initialized to the value it was called with
 */
struct parameter
{
    parameter(method* pthe_method, call_context* pcc);

    /* the name of the parameter (as used on te function's side). This is not used on the client side*/
    std::string name;

    /* the expression of this parameter as read from the source file. Used at function calling */
    expression_tree *expr;

    /* the initial value of the parameter, like: int f (int a=10, b)*/
    expression_tree *initial_value;

    /* 1 if the value is simpl (meaning, no dimensions) 0 if the value is not so simple, meaning dimensions */
    int simple_value;

    /* the method in which this parameter belongs */
    method *the_method;

    int type;
};
#endif
