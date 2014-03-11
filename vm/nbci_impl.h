#ifndef _NBCI_IMPL_H_
#define _NBCI_IMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "nap_types.h"
#include "stack.h"

#include <stddef.h>


/* types for manipulating the addresses, indexes, etc */
typedef uint32_t nap_addr_t;    /* the type of a NAP address*/
typedef uint32_t nap_mark_t;    /* the type of a marker pushed on the stack */
typedef uint32_t nap_index_t;   /* the type of an index */

struct nap_vm;

#if defined(_MSC_VER)
#define SNPRINTF _snprintf
#define strtoll _strtoi64
#else
#define SNPRINTF snprintf
#endif

/* Macro for leaving the application in case of an unimplemented opcode */
#define _NOT_IMPLEMENTED                                                       \
    do {                                                                       \
    fprintf(stderr, "NI: file [%s] line [%d] instr [%x] "                      \
                    "opcode [%x] at %" PRINT_u " (%" PRINT_x ")\n\n",          \
            __FILE__, __LINE__, vm->content[vm->cc - 1],                       \
            vm->current_opcode, vm->cc - 1, vm->cc - 1);                       \
    exit(EXIT_FAILURE);                                                        \
    } while(0);

/* macro to try to call a function and leave the app in case of error with the
 * given error code */
#define TRY_CALL(func, err)                                                    \
    do                                                                         \
    {                                                                          \
        if(func(vm) == NAP_FAILURE)                                            \
        {                                                                      \
            if(vm->error_code == 0) nap_set_error(vm, err);                    \
            else vm->error_code = (vm->error_code << 16) + err;                \
            if(vm->environment == STANDALONE)                                  \
            {                                                                  \
                fprintf(stderr, "%s\n", vm->error_message);                    \
                if(vm->error_description)                                      \
                    fprintf(stderr, "%s\n", vm->error_description);            \
                nap_vm_cleanup(vm);                                            \
                exit(0);                                                       \
            }                                                                  \
            else                                                               \
            {                                                                  \
                return;                                                        \
            }                                                                  \
        }                                                                      \
    } while(0);

/* macro for checking that a variable is not null */
#define ASSERT_NOT_NULL_VAR(ve)                                                \
    if(NULL == ve)                                                             \
    {                                                                          \
        nap_set_error(vm, ERR_VM_0008);                                        \
        return NAP_FAILURE;                                                    \
    }

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
  /* TODO figure out which to use. */
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
 * Read an immediate byte from the bytecode stream and return it
 * @param vm
 * @return
 */
nap_byte_t nap_read_byte(struct nap_vm* vm);


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
 * @brief convert_string_from_bytecode_file converts a string from the bytecode
 * file's UTF-32BE format to the system's internal format.
 * @param src
 * @param len
 * @return
 */
char* convert_string_from_bytecode_file(const char *src, size_t len, size_t dest_len, size_t *real_len);

#ifdef __cplusplus
}
#endif

#endif

