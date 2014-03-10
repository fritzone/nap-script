#ifndef _NAP_STRUCTS_H_
#define _NAP_STRUCTS_H_

#include <stdint.h>
#include <stddef.h>

/**
 * @brief The bytecode chunk which is created as a result of a succesful compilation.
 *
 * The format of the bytes in the \c code member of a \c nap_bytecode_chunk is
 * identical to the one in the compiled bytecode file.
 *
 * The following places are using this sutrcture:
 *
 * 1. The runtime is provided as a result of the \c nap_runtime_compile()
 *    API call, which compiles nap-script commands into bytecode.
 * 2. in the VM as a result of interrupt 2, which allows run-time compilation
 *    of nap-script commands and execution via interrupt 3.
 */
struct nap_bytecode_chunk
{
    /** the length of the bytecode */
    size_t length;

    /** the bytes itself */
    uint8_t* code;

    /** the name of the bytecode chunk, can be specified in \c nap_runtime_compile
     */
    char* name;
};

#endif
