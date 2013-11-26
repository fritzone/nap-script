#ifndef _JMPTABLE_H_
#define _JMPTABLE_H_

#include <stdint.h>
#include <stdio.h>

struct nap_vm;

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


/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
void read_jumptable(struct nap_vm *vm, FILE* fp);
void interpret_jumptable(struct nap_vm* vm, uint8_t* start_location, uint32_t len);
#endif
