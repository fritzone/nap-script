#ifndef _ENVELOPE_H_
#define _ENVELOPE_H_

#include "utils.h"
#include "common_structs.h"

/**
 * Creates a new struct envelope for the given type with the given data
 */
static struct envelope* new_envelope(void* data, int env_type)
{
 envelope* tmp = alloc_mem(envelope,1);
        tmp->to_interpret = data;
        tmp->type = env_type;
        return tmp;
}


#endif
