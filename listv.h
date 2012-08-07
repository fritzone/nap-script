#ifndef _LISTV_H_
#define _LISTV_H_

#include "envelope.h"
// #include "method.h"
// #include "call_ctx.h"

// holds the data that is used in a list of values, { A, B, C} for example
// listv structures are stored in envelopes, with type LIST_VALUE
struct listv
{
	listv* next;
	envelope* val;
};

// prepares a list
// listv* listv_prepare_list(const char* src,  method* the_method, const char* orig_expr, call_context* cc, int* result, const expression_with_location* expwloc)

#endif
