#ifndef _STACK_H_
#define _STACK_H_

#include <stdint.h>

/******************************************************************************/
/*                             Stack section                                  */
/******************************************************************************/

typedef enum TStackEntryType
{
    STACK_ENTRY_INT = 1,                /* same as OPCODE_INT */
    STACK_ENTRY_REAL = 2,               /* same as OPCODE_FLOAT */
    STACK_ENTRY_STRING = 3,             /* same as OPCODE_STRING */
    STACK_ENTRY_CHAR = 4,               /* same as OPCODE_CHAR */
    STACK_ENTRY_MARKER = 5,
    STACK_ENTRY_IMMEDIATE_INT = 6,
    STACK_ENTRY_MARKER_NAME = 7
} StackEntryType;

/**
 * Structure representing a stack entry. The same structure is used in the
 * instantiation of a variable_entry
 **/
struct stack_entry
{
    /* the type of the entry */
    StackEntryType type;

    /* the value of it. At this address usually there is:
     * 1. for imemdiate values:
     *     - an int64_t (STACK_ENTRY_INT)
     *     - a double   (STACK_ENTRY_REAL)
     *     - a null terminated char* (STACK_ENTRY_STRING)
     *     - or an object descriptor (TODO)
     * 2. for variables:
     *     - a variable entry object
     */
    void* value;

    /* if this contains a string this is the length of it */
    uint32_t len;
};

#endif
