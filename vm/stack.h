#ifndef _STACK_H_
#define _STACK_H_

#include <stddef.h>

/******************************************************************************/
/*                             Stack section                                  */
/******************************************************************************/

/* Used to indicate that there is something special on the stack, such as an
 * array or a reference */
#define STACKT_MODIFIER_ARRAY  20
#define STACKT_MODIFIER_REF    30

typedef enum TStackEntryType
{
    STACK_ENTRY_INVALID       = 0,
    STACK_ENTRY_INT           = 1,                /* same as OPCODE_INT */
    STACK_ENTRY_REAL          = 2,                /* same as OPCODE_FLOAT */
    STACK_ENTRY_STRING        = 3,                /* same as OPCODE_STRING */
    STACK_ENTRY_BYTE          = 4,                /* same as OPCODE_BYTE */
    STACK_ENTRY_OBJECT        = 6,                /* same as OPCODE_GENERIC */
    STACK_ENTRY_MARKER_NAME   = 8,

    STACK_ENTRY_LAST
} StackEntryType;

#define CLASS_TYPES_START 10

struct variable_entry;
struct nap_vm;

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
     * 1. for something with a value (such as pushing a number, a string or
     *    a variabe)
     *     - an nap_int_t            (type = STACK_ENTRY_INT)
     *     - a nap_real_t            (type = STACK_ENTRY_REAL)
     *     - a UTF32-BE string       (type = STACK_ENTRY_STRING)
     *     - a nap_byte_t            (type = STACK_ENTRY_BYTE)
     *    resulted from: push 23, push "abc", push reg int(2), push global.a ...
     *
     * 2. for pushing an object on the stack:
     *     - a class_instantiation object (classes.h)
     *
     * 3. for pushing a full indexed value (array)
     *     - a structure with a member being a contiguos memory area
     *       allocated for an indexed variable and another member being the
     *       index definition itself. In this case the type member of this tells
     *       us the type of the data being pushed. (TODO)
     *
     * 2. for variable declarations (they are pushed on the stack):
     *     - there is always 0 pushed onto the stack as the initial value of the
     *       variable.
     *    resulted from: push int global.int_var
     * 4. for a marker: the value of the stack pointer where the marker resides
     *    used by clrs to faster move down the stack
     */
    void* value;

    /* This member has the variable entry in this case this object was created
     * due to a variable creation. It is useful only for class like objects to
     * know the destructor that must be called in order to delete the object.
     */
    struct variable_entry* var_def;

    /* if this contains a
     *  1. string
     *  2. an element with indexes
     * this is the length of the allocated memory.
     *
     * If this stack entry contains a marker, this is the marker code
     * */
    size_t len;

    /* This tells us if this stack entry was stored or not. If it was stored it
     * will stay on the stack in case of a stack rollback (clrsn) till the moment
     * a "restore" call will be executed, which will take over the instantiation
     * of it and erase the actual stack entry */
    char stored : 1;

    /* This is true if the value pushed holds an array. In this case the value
     * holds a variable_entry structure, which is simply taken from the
     * variable and it's not suppsoed to be freed by clrs*/
    char holds_array : 1;

};

/**
 * @brief relocate_stored_element moves the "store"d elements from the stack of
 * the given VM one down.
 *
 * @param vm the VM for which the stack needs adjustment
 */
void relocate_stored_elements(struct nap_vm* vm);

#endif
