#ifndef FUNTABLE_H
#define FUNTABLE_H

#include <stdint.h>

/* an entry in the function table loaded from the file */
struct funtable_entry
{
    /* the index of this method in the jumptable */
    uint32_t jmptable_index;

    /* the name of the function. ASCII encoding */
    char* function_name;

    /* the number of parameters this function takes */
    uint8_t parameter_count;

    /* the parameter types. Allocated to parameter_count */
    uint8_t* parameter_types;
};


/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
int interpret_funtable(struct nap_vm* vm,
                       uint8_t* start_location,
                       uint32_t len);
#endif
