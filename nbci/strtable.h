#ifndef _STRTABLE_H_
#define _STRTABLE_H_

#include <stdint.h>
#include <stdio.h>

/******************************************************************************/
/*                             Stringtable section                            */
/******************************************************************************/

/**
 * Structure for holding a stringtable entry.
 */
struct strtable_entry
{
    /* the index of the string as referred in the btyecode */
    uint64_t index;

    /* the length of th string*/
    uint32_t len;

    /* the string itself */
    char* string;
};
/* variables for the stringtable */
extern struct strtable_entry** stringtable;  /* the stringtable itself */
extern uint64_t strt_size;                 /* the size of the stringtable */

/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
void read_stringtable(FILE* fp, uint64_t stringtable_location);

#endif
