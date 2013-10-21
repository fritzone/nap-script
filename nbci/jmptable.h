#ifndef _JMPTABLE_H_
#define _JMPTABLE_H_

#include <stdint.h>
#include <stdio.h>

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

/* variables for the jumptable */
extern struct jumptable_entry** jumptable;   /* the jumptable itself */
extern uint64_t jumptable_size;              /* the size of the jumptable */
extern uint32_t jmpc;               /* counts the jumptable entries on loading*/
/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
void read_jumptable(FILE* fp, uint64_t stringtable_location);

#endif
