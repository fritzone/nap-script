#ifndef _METATBL_H_
#define _METATBL_H_

#include "strtable.h"
#include "nbci.h"

#include <stdint.h>

/******************************************************************************/
/*                             Metatable section                              */
/******************************************************************************/

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
    uint8_t type;

    /* the name of the variable */
    char* name;

    /* the actual value of the variable */
    struct stack_entry* instantiation;

    /* in case this variable is initialized to a string this points to it */
    struct strtable_entry* string_initialization;
};

/*
 * Read the metatable from the given location
 */
void interpret_metatable(struct nap_vm* vm,
                         uint8_t* start_location,
                         uint32_t len);


#endif
