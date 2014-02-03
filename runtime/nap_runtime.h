#ifndef _NAP_RUNTIME_H_
#define _NAP_RUNTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "nap_types.h"
#include "nap_structs.h"
#include "nap_consts.h"

	
#ifdef _WINDOWS
#include "nap_rt_exp.h"
#else
#define NAP_LIB_API
#endif

/* The real runtime hidden in an opaque pointer  */
struct nap_runtime;

/**
 * @brief Create a new nap runtime environment.
 *
 * Creates a new runtime environment which can execute nap-script code. 
 * Optionally you can specify a name for the new environment.
 *
 * @param[in] name The name of the environment. If NULL means not used.
 *
 * @return a new runtime environment or NULL (0) in case of error
 **/
NAP_LIB_API nap_runtime* nap_runtime_create(const char *name);

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
 * @param[in] runtime   The runtime on which the commands should be compiled
 * @param[in] commands  The commands the runtime will execute. These are
 *                      standard nap-script commands, you would place in a
 *                      .nap source file.
 *
 * @return a nap_bytecode_chunk object containing the compiled bytecode or 
 * NULL (0) in case of an error.
 **/
NAP_LIB_API nap_bytecode_chunk* nap_runtime_compile(struct nap_runtime *runtime,
                                        const char         *commands);
/**
 * @brief The nap_runtime_execute executes the given bytecode chunk in the
 *        virtual machine of the runtime.
 *
 * @param[in] runtime The runtime which will execute the bytecode
 * @param[in] bytecode The bytecode that will be executed.
 *
 * @return NAP_EXECUTE_SUCCESS (1) in case of succes, or NAP_EXECUTE_FAILURE (0)
 * in case of failure. In case of failure you can call the method
 * nap_runtime_last_error() to get the last error
 **/ 
NAP_LIB_API int nap_runtime_execute(struct nap_runtime        *runtime,
                        struct nap_bytecode_chunk *bytecode);

/**
 * @brief The nap_runtime_get_int returns the value of the integer type variable
 *        called variable_name.
 *
 * The method returns the value of the integer type variable called
 * variable_name. Only variables from the global namespace can be retrieved.
 *
 * @param[in]  runtime The runtime which has the variable defined.
 * @param[in]  name    The name of the variable.
 * @param[out] found   Populated to NAP_VARIABLE_FOUND (1) if the variable was
 *                     found in the runtime or to NAP_VARIABLE_NOT_FOUND (0) if
 *                     the variable was not found.
 *
 * @return The value of the variable, or NAP_NO_VALUE (NAP_NO_VALUE) if the
 *         variable was not found. If the runtime returns the NAP_NO_VALUE then
 *         also the found parameter should be populated to the value of
 *         NAP_VARIABLE_NOT_FOUND in order to confirm that there is indeed no
 *         such variable, otherwise the value indeed is NAP_NO_VALUE.
 */
NAP_LIB_API nap_int_t nap_runtime_get_int(struct nap_runtime* runtime,
                              const char *variable_name,
                              int* found);

/**
 * @brief The nap_runtime_get_byte returns the value of the byte type variable
 *        called variable_name.
 *
 * The method returns the value of the byte type variable called
 * variable_name. Only variables from the global namespace can be retrieved.
 *
 * @param[in]  runtime The runtime which has the variable defined.
 * @param[in]  name    The name of the variable.
 * @param[out] found   Populated to NAP_VARIABLE_FOUND (1) if the variable was
 *                     found in the runtime or to NAP_VARIABLE_NOT_FOUND (0) if
 *                     the variable was not found.
 *
 * @return The value of the variable, or NAP_NO_VALUE (NAP_NO_VALUE) if the
 *         variable was not found. If the runtime returns the NAP_NO_VALUE then
 *         also the found parameter should be populated to the value of
 *         NAP_VARIABLE_NOT_FOUND in order to confirm that there is indeed no
 *         such variable, otherwise the value indeed is NAP_NO_VALUE.
 */
NAP_LIB_API nap_byte_t nap_runtime_get_byte(struct nap_runtime* runtime,
                              const char *variable_name,
                              int* found);

NAP_LIB_API nap_real_t nap_runtime_get_real(struct nap_runtime* runtime,
                                 const char* variable_name);

NAP_LIB_API nap_string_t nap_runtime_get_string(struct nap_runtime* runtime,
                                 const char* variable_name, int* found);

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
NAP_LIB_API void nap_runtime_shutdown(struct nap_runtime** runtime);

#ifdef __cplusplus
}
#endif

#endif
