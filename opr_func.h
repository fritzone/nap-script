#ifndef _OPR_FUNC_H_
#define _OPR_FUNC_H_

#include "call_ctx.h"

/*
 * this checks if an expression is a function or not (ex: sin(x) is s function)
 */
struct method* is_function_call(char *s, struct call_context* cc);


#endif
