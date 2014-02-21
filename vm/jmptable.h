#ifndef _JMPTABLE_H_
#define _JMPTABLE_H_

#include <stdint.h>
#include <stdio.h>

struct nap_vm;

enum JmptableEntryType
{
    JMPTABLE_ENTRY_TYPE_SIMPLE      = 0,
    JMPTABLE_ENTRY_TYPE_FUNCTION    = 1,
    JMPTABLE_ENTRY_TYPE_CLASSMETHOD = 2,
    JMPTABLE_ENTRY_TYPE_FROM_PARENT = 3
};

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

    /* the type of this label:
     * 0 - simple jump label
     * 1 - a function
     * 2 - a method of a class
     * 3 - a method from the VM chain (the parent VM) */
    enum JmptableEntryType type;

    /* the name of the label if it is a function or method */
    char* label_name;
};


/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
int interpret_jumptable(struct nap_vm* vm,
                         uint8_t* start_location,
                         uint32_t len);
#endif
