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
     * and the type of the instantioation will hold the type of the variable.
     * The \c var_def member of the stack_entry is pointig to this object.
     */
    struct stack_entry* instantiation;

    /* the dimensions of this variable. The scanning for dimensions begins at
     * the first element ([0]) and as long as there is a p0sitive value (>0) we
     * increment a dimension counter. If any of the elements is -1 the variable
     * is considered to be with dynamic dimensions, and on runtime it is
     * resized as per needs ofthe index. The scanning stops on the first 0 index
     */
    int32_t dimensions[256];

    /* just stores the number of dimensions for easier access */
    uint8_t dimension_count;

    /* the size of the data for which we have allocated memory in the instantiation */
    uint8_t data_size;
};

/*
 * Read the metatable from the given location
 */
int interpret_metatable(struct nap_vm* vm,
                         uint8_t* start_location,
                         uint32_t len);


#endif
