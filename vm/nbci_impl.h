#ifndef _NBCI_IMPL_H_
#define _NBCI_IMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "nap_types.h"
#include "stack.h"

#include <stddef.h>

struct nap_vm;

#if defined(_MSC_VER)
#define SNPRINTF _snprintf
#define strtoll _strtoi64
#else
#define SNPRINTF snprintf
#endif

#define CHECK_VARIABLE_INSTANTIATON(var)                                       \
    if(var->instantiation == 0)                                                \
    {                                                                          \
        char* s = (char*)calloc(128, sizeof(char));                             \
        SNPRINTF(s, 128, "Variable [%s] not initialised correctly. "\
                   "It has no instantiation.", var->name);                     \
        vm->error_description = s;                                             \
        return NAP_FAILURE;                                                    \
    }

#define CHECK_VARIABLE_TYPE(var, REQ_TYPE_CODE)                                \
    if(var->instantiation->type != REQ_TYPE_CODE)                              \
    {                                                                          \
        char* s = (char*)calloc(128, sizeof(char));                             \
        SNPRINTF(s, 128, "Variable [%s] has wrong type."            \
                   "Expected [%s] got[%s].", var->name,                        \
                    nap_get_type_description(REQ_TYPE_CODE),                   \
                    nap_get_type_description(var->instantiation->type));       \
        vm->error_description = s;                                             \
        return NAP_FAILURE;                                                    \
    }

#if defined(_MSC_VER)
  #define JL_SIZE_T_SPECIFIER    "%Iu"
  #define JL_SSIZE_T_SPECIFIER   "%Id"
  #define JL_PTRDIFF_T_SPECIFIER "%Id"
#elif defined(__GNUC__)
  #define JL_SIZE_T_SPECIFIER    "%zu"
  #define JL_SSIZE_T_SPECIFIER   "%zd"
  #define JL_PTRDIFF_T_SPECIFIER "%zd"
#else
  // TODO figure out which to use.
  #if NUMBITS == 32
    #define JL_SIZE_T_SPECIFIER    something_unsigned
    #define JL_SSIZE_T_SPECIFIER   something_signed
    #define JL_PTRDIFF_T_SPECIFIER something_signed
  #else
    #define JL_SIZE_T_SPECIFIER    something_bigger_unsigned
    #define JL_SSIZE_T_SPECIFIER   something_bigger_signed
    #define JL_PTRDIFF_T_SPECIFIER something-bigger_signed
  #endif
#endif

/**
 * Returns 1 if the vm has a variable with the given name and populate the
 * *type with the type of the variable. *type will be -1 if no variable
 * is found and zero is returned in this case
 **/
int nap_vmi_has_variable(const struct nap_vm *vm, const char* name, int *type);

/**
 * Returns the internal variable structure of the given variable or NULL if nothing is found
 **/
struct variable_entry* nap_vmi_get_variable(const struct nap_vm *vm, const char* name);

/**
 * Fetches the address at the current location in the bytecode stream
 * @param vm
 * @return
 */
nap_addr_t nap_fetch_address(struct nap_vm *vm);

/**
 * Fetch a marker from the bytecode stream
 * @param vm
 * @return
 */
nap_mark_t nap_fetch_mark(struct nap_vm* vm);

/**
 * Fetch an index from the bytecode stream
 * @param vm
 * @return
 */
nap_index_t nap_fetch_index(struct nap_vm* vm);

/**
 * @brief Fetch a variable of the VM, or from the parent VM
 * @param vm - the VM in whitch we are working
 * @param var_index - the index of teh variable
 * @return
 */
struct variable_entry* nap_fetch_variable(struct nap_vm* vm, nap_index_t var_index);

/**
 * Read an immediate value from the bytecode stream and return it
 * @param vm
 * @return
 */
nap_int_t nap_read_immediate(struct nap_vm* vm);

/**
 * Saves the registers. Happens automatically on a "call"
 */
int nap_save_registers(struct nap_vm*);

/**
 * Restores the registers. Happens automatically on a "leave"
 */
int nap_restore_registers(struct nap_vm*);

/**
 * Handles the interrupt which is next in the "content" of the VM
 */
int nap_handle_interrupt(struct nap_vm*);

/**
 * Delivers the error message with the code error_code in the error_message
 * field of the VM, ie: allocates memory for it and copies the string over.
 */
void nap_set_error(struct nap_vm*, int error_code);

/**
 * @brief Returns the description of the given type
 */
const char* nap_get_type_description(StackEntryType t);


/**
 * Sets the last boolean flag according to the operation found int current_opcode
 * @param vm - the virtual machine
 * @param reg - the registers value to check
 * @param immediate - against this value
 * @param current_opcode - the operation which is supposed to be executed
 */
void nap_vm_set_lbf_to_op_result(struct nap_vm* vm, nap_int_t reg, nap_int_t immediate, uint8_t opcode);

/**
 * Perform the given operation to be found in the opcode, stores the result in target
 * @param vm - the virtual machine
 * @param target - the target of the operation, and the first operand
 * @param operand - the operand on whcih we perform
 * @param opcode - the operation we perform
 * @throws a system error if the operation is division and the operand is zero
 */
void do_operation(struct nap_vm* vm, nap_int_t *target, nap_int_t operand, uint8_t opcode);

/**
 * @brief convert_string_from_bytecode_file converts a string from the bytecode
 * file's UTF-16BE format to the system's internal format.
 * @param src
 * @param len
 * @return
 */
char* convert_string_from_bytecode_file(const char *src, size_t len, size_t dest_len, size_t *real_len);

#ifdef __cplusplus
}
#endif

#endif

