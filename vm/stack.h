#ifndef _STACK_H_
#define _STACK_H_

#include <stddef.h>

/******************************************************************************/
/*                             Stack section                                  */
/******************************************************************************/

typedef enum TStackEntryType
{
    STACK_ENTRY_INVALID       = 0,
    STACK_ENTRY_INT           = 1,                /* same as OPCODE_INT */
    STACK_ENTRY_REAL          = 2,                /* same as OPCODE_FLOAT */
    STACK_ENTRY_STRING        = 3,                /* same as OPCODE_STRING */
    STACK_ENTRY_BYTE          = 4,                /* same as OPCODE_BYTE */
    STACK_ENTRY_IDX           = 5,
    STACK_ENTRY_CHAR          = 6,                /* same as OPCODE_CHAR */
    STACK_ENTRY_IMMEDIATE_INT = 7,
    STACK_ENTRY_MARKER_NAME   = 8,
    STACK_ENTRY_MARKER        = 9,

    STACK_ENTRY_LAST
} StackEntryType;

/**
 * Structure representing a stack entry. The same structure is used in the
 * instantiation of a variable_entry. An object of this type holds the copy
 * of the data that was pushed on the stack, ie: if we push a register no
 * index to the register is there, but the actual value of the register.
 * If we push a string the value will point to a copy of the string, not an
 * index to the string table entry.
 **/
struct stack_entry
{
    /* the type of the entry */
    StackEntryType type;

    /* the value of it. At this address usually there is:
     *
     * 1. for immediate values (such as pushing a number, a string)
     *     - an nap_int_t            (type = STACK_ENTRY_INT)
     *     - a nap_real_t            (type = STACK_ENTRY_REAL)
     *     - a UTF32-BE string       (type = STACK_ENTRY_STRING)
     *     - a nap_byte_t            (type = STACK_ENTRY_BYTE)
     *    resulted from: push 23, push "abc", push reg int(2), etc...
     *
     * 2. for pushing an object on the stack:
     *     - an object descriptor (TODO)
     *
     * 3. for pushing a full indexed value (array)
     *     - a structure with a member being a contiguos memory area
     *       allocated for an indexed variable and another member being the
     *       index itself. In this case the type member of this tells us the
     *       type of the data being pushed. (TODO)
     *
     * 2. for variables (pushed on the stack):
     *     - a variable entry object
     *    resulted from: push global.var
     */
    void* value;

    /* if this contains a
     *  1. string
     *  2. an element with indexes
     * this is the length of the allocated memory */
    size_t len;
};

#endif
