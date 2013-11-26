#ifndef _PARAMETR_H_
#define _PARAMETR_H_

#include "common_structs.h"
#include "utils.h"
#include "call_ctx.h"

/**
 * Creates a new, empty function parameter
 * @return the newly created parameter
 */
static struct parameter* new_parameter(method* the_method, call_context* cc)
{
    parameter* tmp = alloc_mem(parameter,1, cc->compiler());
    tmp->the_method = the_method;
    tmp->simple_value = 1;
    return tmp;
}

#endif
