#ifndef _NAP_RUNTIME_H_
#define _NAP_RUNTIME_H_

/**
 * @file nap_runtime.h
 *
 * The Nap Runtime header
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "nap_types.h"
#include "nap_structs.h"
#include "nap_consts.h"

#ifdef _WINDOWS
#define NAP_EXPORTS extern "C" __declspec(dllexport)
#else
#define NAP_EXPORTS extern "C"
#endif

#ifdef _WINDOWS
#include "nap_rt_exp.h"
#else
#define NAP_LIB_API
#endif

/* The real runtime hidden in an opaque pointer  */
struct nap_runtime;

/*!
 * @brief Create a new nap runtime environment.
 *
 * Creates a new runtime environment which can execute nap-script code. 
 * Optionally you can specify a name for the new environment, if nothing is
 * specified, "naprt_"X will be used where X is an increasing counter starting
 * from 1.
 *
 * The following important parts are in a \c nap_runtime object:
 *
 *  1. a nap compiler - this is used to compile nap-script into nap-bytecode.
 *                      The compiler is created automatically by the call to the
 *                      \c nap_runtime_create method
 *
 *  2. a nap virtual machine - this is created by the \c nap_runtime_execute
 *                      when a bytecode chunk is to be executed. The VM created
 *                      is alive till another call of the \c nap_runtime_execute
 *
 * The nap runtime keeps track of several bytecode chunks and each of them
 * can be executed separately from the other using \c nap_runtime_execute,
 * however they are sealed from each other (they cannot call methods or
 * reference variables from another bytecode chunk).
 *
 * All the bytecode chunks are being executed by the same VM, on execution the
 * data of the previous bytecode chunk is purged, and the new one is injected
 * into the internal structures of the nap virtual machine.
 *
 * The \c nap_runtime_create does not
 *
 * @param[in] name The name of the environment. If NULL means not used.
 *
 * @return a new runtime environment or NULL (0) in case of error
 */
NAP_LIB_API nap_runtime* nap_runtime_create(const char *name);

/******************************************************************************/

/**
 * @brief Compiles the commands and creates a bytecode chunk for them.
 *
 * Compiles the nap-script commands and creates a new code chunk object for 
 * the compiled bytecode. The runtime takes care of the compiled bytecode 
 * and you do NOT need to free it. The function will return NULL in case of
 * failure. In this case use the \c nap_runtime_last_error to get the text
 * of the error that occured.
 *
 * After compilation you can not access yet the variables and call the methods
 * from bytecode chunk using the respective APIs, first you need to execute the
 * resulted bytecode chunk using ::nap_runtime_execute. By executing the
 * bytecode the VM sets up its internal structures, creates the variables
 * on the VM's stack, and executes the instructions found in the global call
 * context of the script.
 *
 * The format of the returned bytcode chunk is the same as of a compiled 
 * nap bytecode file.
 *
 * @param[in] runtime   The runtime on which the commands should be compiled
 * @param[in] commands  The commands the runtime will execute. These are
 *                      standard nap-script commands, you would place in a
 *                      .nap source file.
 * @param[in] name      An identifier which will be assigned to the bytecode
 *                      chunk in case of success. Optional, pass in NULL if
 *                      you don't need this functionality.
 *
 * @return a \c nap_bytecode_chunk object containing the compiled bytecode or
 * NULL (0) in case of an error.
 **/
NAP_LIB_API struct nap_bytecode_chunk* nap_runtime_compile(
                                                    struct nap_runtime *runtime,
                                                    const char *commands,
                                                    const char* name);

/**
 * @brief Loads a nap-script file from the disk, and compiles it.
 *
 * Loads a nap-script file from the location given in the \c file parameter
 * compiles it
 *
 * @param[in] runtime  The runtime on which the file will be compiled
 * @param[in] file     The name of the script file
 *
 * @return a \c nap_bytecode_chunk object containing the compiled bytecode or
 * NULL (0) in case of an error.
 **/
NAP_LIB_API struct nap_bytecode_chunk* nap_runtime_load_script(
                                                 struct nap_runtime *runtime,
                                                 const char *file);

/******************************************************************************/

/**
 * @brief The nap_runtime_execute executes the given bytecode chunk in the
 *        virtual machine of the runtime.
 *
 * You can obtain \c nap_bytecode_chunk objects by compiling nap-script using
 * @see nap_runtime_compile or by loading already compiled bytecode files from
 * the disk using @see nap_runtime_load_script
 *
 * @param[in] runtime The runtime which will execute the bytecode
 * @param[in] bytecode The bytecode that will be executed.
 *
 * @return NAP_EXECUTE_SUCCESS (1) in case of succes, or NAP_EXECUTE_FAILURE (0)
 * in case of failure. In case of failure you can call the method
 * nap_runtime_last_error() to get the last error
 **/ 
NAP_LIB_API int nap_runtime_execute(struct nap_runtime *runtime,
                                    struct nap_bytecode_chunk *bytecode);

/******************************************************************************/

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
 * @return The value of the variable, or NAP_NO_VALUE if the
 *         variable was not found. If the runtime returns the NAP_NO_VALUE then
 *         also the found parameter should be populated to the value of
 *         NAP_VARIABLE_NOT_FOUND in order to confirm that there is indeed no
 *         such variable, otherwise the value indeed is NAP_NO_VALUE.
 **/
NAP_LIB_API nap_int_t nap_runtime_get_int(struct nap_runtime* runtime,
                              const char *variable_name,
                              int* found);

/******************************************************************************/

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
 * @return The value of the variable, or NAP_NO_VALUE if the
 *         variable was not found. If the runtime returns the NAP_NO_VALUE then
 *         also the found parameter should be populated to the value of
 *         NAP_VARIABLE_NOT_FOUND in order to confirm that there is indeed no
 *         such variable, otherwise the value indeed is NAP_NO_VALUE.
 **/
NAP_LIB_API nap_byte_t nap_runtime_get_byte(struct nap_runtime* runtime,
                                            const char *variable_name,
                                            int* found);

/******************************************************************************/

/**
 * @brief The nap_runtime_get_real returns the value of the real type variable
 *        called variable_name.
 *
 * The method returns the value of the real type variable called
 * variable_name. Only variables from the global namespace can be retrieved.
 *
 * @param[in]  runtime The runtime which has the variable defined.
 * @param[in]  name    The name of the variable.
 * @param[out] found   Populated to NAP_VARIABLE_FOUND (1) if the variable was
 *                     found in the runtime or to NAP_VARIABLE_NOT_FOUND (0) if
 *                     the variable was not found.
 *
 * @return The value of the variable, or NAP_NO_VALUE if the
 *         variable was not found. If the runtime returns the NAP_NO_VALUE then
 *         also the found parameter should be populated to the value of
 *         NAP_VARIABLE_NOT_FOUND in order to confirm that there is indeed no
 *         such variable, otherwise the value indeed is NAP_NO_VALUE.
 **/
NAP_LIB_API nap_real_t nap_runtime_get_real(struct nap_runtime* runtime,
                                            const char* variable_name);

/******************************************************************************/

/**
 * @brief The nap_runtime_get_byte returns the value of the string type variable
 *        called variable_name.
 *
 * The method returns the value of the string type variable called
 * variable_name. Only variables from the global namespace can be retrieved.
 *
 * @param[in]  runtime The runtime which has the variable defined.
 * @param[in]  name    The name of the variable.
 * @param[out] found   Populated to NAP_VARIABLE_FOUND (1) if the variable was
 *                     found in the runtime or to NAP_VARIABLE_NOT_FOUND (0) if
 *                     the variable was not found.
 *
 * @return The value of the variable, or NULL (0) if the
 *         variable was not found. If the runtime returns the NULL then
 *         also the found parameter should be populated to the value of
 *         NAP_VARIABLE_NOT_FOUND in order to confirm that there is indeed no
 *         such variable, otherwise the value indeed is NULL.
 **/
NAP_LIB_API nap_string_t nap_runtime_get_string(struct nap_runtime* runtime,
                                                const char* variable_name,
                                                int* found);

/******************************************************************************/

/**
 * @brief Executes a method from the given runtime's given chunk.
 *
 * The method executes a function which was declared in the given runtime.If the
 * method was forward declared in the runtime, the dependency will be resolved
 * if possible (ie: the method was defined at some other point). You also can
 * execute external (native) methods declared in the script via this mechanism.
 *
 * The return value must be the \b address of a:
 *
 *   - a \c nap_int_t in case the method returns an \c int
 *   - a \c nap_string_t in case the method returns a \c string (ie: \c char**)
 *   - a \c nap_real_t in case of a \c real return type from the method.
 *   - a \c nap_byte_t in case of a \c byte return type from the method.
 *
 * @param[in]  runtime      The runtime in which the method was defined.
 * @param[out] return_value The address of the return value.
 * @param[in]  method_name  The method to execute
 * @param[in]  ...          The arguments to be passed to the method.
 *
 * The arguments passed in to the method in the variablae arguments list
 * must be one of the following:
 *
 *   - a \c nap_int_t where the method expects an \c int
 *   - a \c nap_string_t where the method expects a \c string (ie: \c char*)
 *   - a \c nap_real_t where the method expects a \c real
 *   - a \c nap_byte_t where the method expects a \c byte
 *
 * This method uses variable arguments, and the values of the variable arguments
 * are determined based on the signature of the nap-script method. We recommend
 * to not to use immediate values in the parameter list, unless you cast them
 * to the speicific C/C++ type, such as: \c (nap_int_t)3
 *
 * @return NAP_EXECUTE_SUCCESS (1) in case of succes, or NAP_EXECUTE_FAILURE (0)
 * in case of failure. In case of failure you can call the method
 * nap_runtime_last_error() to get the last error
 **/
NAP_LIB_API int nap_execute_method(struct nap_runtime* runtime,
                                   void* return_value,
                                   const char* method_name,
                                   ...);

/**
 * @brief Executes the given nap commands (nap-script) in the runtime.
 *
 * @param[in] runtime The runtime which will run the code.
 * @param[in] script The code that will be executed.
 *
 * @return NAP_EXECUTE_SUCCESS (1) in case of succes, or NAP_EXECUTE_FAILURE (0)
 * in case of failure. In case of failure you can call the method
 * nap_runtime_last_error() to get the last error
 **/
NAP_LIB_API int nap_execute_code(struct nap_runtime* runtime,
                                 const char* script);

/******************************************************************************/

/**
 * @brief Shuts down the runtime and frees the allocated memory.
 *
 * Frees the memory allocated by this runtime, shuts down its compiler, and
 * deallocates the allocated bytecode chunks. It is mandatory to call this
 * method when your application does not need it anymore in order to release
 * all the allocated memory.
 *
 * @param runtime - the address of a nap_runtime object
 **/
NAP_LIB_API void nap_runtime_shutdown(struct nap_runtime** runtime);

#ifdef __cplusplus
}
#endif

#endif
