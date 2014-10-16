#ifndef _METATBL_H_
#define _METATBL_H_

#include "strtable.h"
#include "stack.h"
#include "nbci.h"

#include <stdint.h>

/******************************************************************************/
/*                             Metatable section                              */
/******************************************************************************/

enum VAR_TYPE
{
    OWN_VAR = 0,
    EXTERN_VAR = 1
};

/**
 * Structure containing the definition of a variable
 **/
struct variable_entry
{
    /* the index of the variable */
    uint32_t index;

    /* the type of the variable:
     * 0 - variable defined in this VM
     * 1 - variable defiend in a parent VM */
    enum VAR_TYPE type;

    /* the name of the variable */
    char* name;

    /* the actual value of the variable
     * The "value" of the stack_entry will contain the value of this variable
     * and the type of the instantiation will hold the type of the variable.
     * The \c var_def member of the stack_entry is pointig to this object.
     */
    struct stack_entry* instantiation;

    /* The previous instantiation of the variable in case of recursive calls.*/
    struct stack_entry* instantiation_stack[DEEPEST_RECURSION];
    /* The instantiation stack pointer */
    int16_t is_p;

    /* the dimensions of this variable. The scanning for dimensions begins at
     * the first element ([0]) and as long as there is a positive value (>0) we
     * increment a dimension counter. If any of the elements is -1 the variable
     * is considered to be with dynamic dimensions, and on runtime it is
     * resized as per needs of the index. The scanning stops on the first 0 index
     */
    nap_int_t dimensions[256];

    /* just stores the number of dimensions for easier access */
    uint8_t dimension_count;

    /* the size of the data for which we have allocated memory in the
     * instantiation, ie: the size of an uint8_t, uint16_t, etc... */
    uint8_t data_size;
};

/*
 * Read the metatable from the given location
 */
int interpret_metatable(struct nap_vm* vm,
                         uint8_t* start_location,
                         uint32_t len);


/**
 * @brief push_variable_instantiation pushes a new instantiation for the given
 * variable_entry on its internal stack. This is done because of recursive
 * functions might overwrite variables and parameters. This must be done for
 * all variables when a function is called.
 * @param ve
 */
void push_variable_instantiation(struct variable_entry* ve);

/**
 * @brief pop_variable_instantiation pops the variable's instantiation from the
 * variable's internal stack.
 * @param ve
 */
void pop_variable_instantiation(struct variable_entry* ve);

#endif
