#ifndef _METATBL_H_
#define _METATBL_H_

#include "strtable.h"

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

    /* the name of the variable */
    char* name;

    /* the actual value of the variable */
    struct stack_entry* instantiation;

    /* in case this variable is initialized to a string this points to it */
    struct strtable_entry* string_initialization;
};

/*
 * Read the metatable of the bytecode file. Exits on error.
 */
void read_metatable(FILE* fp, uint64_t meta_location);


#endif
