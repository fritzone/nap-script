#ifndef _NAP_STRUCTS_H_
#define _NAP_STRUCTS_H_

#include <stdint.h>
#include <stddef.h>

/* The bytecode chunk which is created as a result of the compile command.
 * Used either in the runtime or in the VM as a result of the interrupt
 * mechanism which will compile and execute code runtime */
struct nap_bytecode_chunk
{
    /* the length of the bytecode */
    size_t length;

    /* the bytecode */
    uint8_t* code;

    /* the name of the bytecode chunk */
    char* name;
};

#endif
