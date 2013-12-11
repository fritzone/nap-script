#ifndef _NAP_RUNTIME_H_
#define _NAP_RUNTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "nap_types.h"
#include "nap_structs.h"

/* The real runtime hidden in an opaque pointer  */
struct nap_runtime;

/**
 * @brief Create a new nap runtime environment.
 *
 * Creates a new runtime environment which can execute nap-script code. 
 * Optionally you can specify a name for the new environment.
 *
 * @param name - the name of the environment. If NULL means not used.
 *
 * @return a new runtime environment
 **/
nap_runtime* nap_runtime_create(const char* name);

/**
 * @brief Compiles the commands and creates a bytecode chunk for them.
 *
 * Compiles the nap-script commands and creates a new code chunk object for 
 * the compiled bytecode. The runtime takes care of the compiled bytecode 
 * and you do NOT need to free it. The function will return NULL in case of
 * failure. In this case use the nap_runtime_last_error() to get the text
 * of the error that occured.
 *
 * After compilation you can access the variables and the methods from the 
 * bytecode chunk using the respective APIs.
 *
 * The nap runtime keeps track of several bytecode chunks and each of them  
 * can be executed separately from the other, however they are sealed from 
 * each other. In case you want to merge two chunks use the merge() method.
 * This will create another chunk and will make it possible for you to run  
 * the two chunks as if they were one.
 *
 * The format of the returned bytcode chunk is the same as of a compiled 
 * nap bytecode file.
 *
 * @param runtime  - the runtime on which the commands should be compiled
 * @param commands - the commands the runtime will execute. These are 
 *                   standard nap-script commands, you would place in a 
 *                   .nap source file.
 *
 * @return a nap_bytecode_chunk object containing the compiled bytecode or 
 * NULL in case of an error.
 **/
nap_bytecode_chunk* nap_runtime_compile(struct nap_runtime* runtime,
                                               const char* commands);
/**
 * @brief Executes the given bytecode chunk in the VM.
 *
 * @return 1 in case of succes, 0 in case of failure
 **/ 
int nap_runtime_execute(struct nap_runtime* runtime, 
                        struct nap_bytecode_chunk* bytecode);

nap_int_t nap_runtime_get_int(struct nap_runtime* runtime,
                                 const char *variable_name);

nap_real_t nap_runtime_get_real(struct nap_runtime* runtime,
                                 const char* variable_name);

nap_string_t nap_runtime_get_string(struct nap_runtime* runtime,
                                 const char* variable_name);

/**
 * @brief Shuts down the runtime and frees the allocated memory.
 *
 * Frees the memory allocated by this runtime, shuts down its compiler, and
 * deallocates the allocated bytecode chunks. It is mandatory to call this
 * method when your application does not need it anymore in order to release
 * all the allocated memory.
 *
 * @param runtime - the address of a nap_runtime object
 */
void nap_runtime_shutdown(struct nap_runtime** runtime);

#ifdef __cplusplus
}
#endif

#endif
