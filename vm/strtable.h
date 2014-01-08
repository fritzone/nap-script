#ifndef _STRTABLE_H_
#define _STRTABLE_H_

#include <stdint.h>
#include <stdio.h>

struct nap_vm;

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

    /* the string itself, encoded into UTF-32BE */
    char* string;
};


/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
int interpret_stringtable(struct nap_vm* vm,
                           uint8_t* start_location,
                           uint32_t len);

#endif
