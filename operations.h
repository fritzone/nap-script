#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

#include "envelope.h"
#include "typedefs.h"
#include "method.h"

void do_list_assignment( envelope* rvalue, variable* var, int level, const method* the_method, call_context* cc, int reqd_type);

#endif
