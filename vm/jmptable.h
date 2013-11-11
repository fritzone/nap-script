#ifndef _JMPTABLE_H_
#define _JMPTABLE_H_

#include <stdint.h>

/******************************************************************************/
/*                             Jumptable section                              */
/******************************************************************************/

/*
 * Structure representing an entry in the jumptable
 */
struct jumptable_entry
{
    /* the location in the bytecode stream of the jump destination */
    uint32_t location;
};

#endif
