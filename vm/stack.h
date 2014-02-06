#ifndef _STACK_H_
#define _STACK_H_

#include <stddef.h>

/******************************************************************************/
/*                             Stack section                                  */
/******************************************************************************/

typedef enum TStackEntryType
{
    STACK_ENTRY_INT           = 1,                /* same as OPCODE_INT */
    STACK_ENTRY_REAL          = 2,                /* same as OPCODE_FLOAT */
    STACK_ENTRY_STRING        = 3,                /* same as OPCODE_STRING */
    STACK_ENTRY_BYTE          = 4,                /* same as OPCODE_BYTE */
    STACK_ENTRY_IDX           = 5,
    STACK_ENTRY_CHAR          = 6,                /* same as OPCODE_CHAR */
    STACK_ENTRY_IMMEDIATE_INT = 7,
    STACK_ENTRY_MARKER_NAME   = 8,
    STACK_ENTRY_MARKER        = 9,

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
     *
     * 1. for imemdiate values, or variable instantiations:
     *     - an int64_t       (STACK_ENTRY_INT)
     *     - a double         (STACK_ENTRY_REAL)
     *     - a unicode string (STACK_ENTRY_STRING)
     *     - or an object descriptor (TODO)
     *     - or a contiguos memory area allocated for an indexed
     *       variable. The indexes however are stored in the variable_entry
     *       which is referencing this stack entry in its instantiation
     *
     * 2. for variables (pushed on the stack):
     *     - a variable entry object
     */
    void* value;

    /* if this contains a
     *  1. string
     *  2. an element with indexes
     * this is the length of the allocated memory */
    size_t len;
};

#endif
