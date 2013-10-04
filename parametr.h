#ifndef _PARAMETR_H_
#define _PARAMETR_H_

#include "common_structs.h"
#include "utils.h"

/**
 * Creates a new, empty function parameter
 * @return the newly created parameter
 */
static struct parameter* new_parameter()
{
    parameter* tmp = alloc_mem(parameter,1);
    tmp->simple_value = 1;
    return tmp;
}

#endif
