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
struct nap_string_register;

/* The directions in which the IP can move */
#define BACKWARD -1
#define FORWARD   1

#if defined(_MSC_VER)
#define SNPRINTF _snprintf
#define strtoll _strtoi64
#else
#define SNPRINTF snprintf
#endif

/* Macro for freeing a piece of memory */
#ifdef NAP_MEM_DEBUG
#define NAP_MEM_FREE(x) if((x)) { fprintf(stderr, "free:%p (%s:%d)\n", (x), __FILE__, __LINE__);  free((x)); }
#else
#define NAP_MEM_FREE(x) if(x){ free((x)); }
#endif

#define NAP_REPORT_ERROR(vm, error)                                            \
    do                                                                         \
    {                                                                          \
    if(vm->environment == STANDALONE)                                          \
    {                                                                          \
        fprintf(stderr, "%s\n", error);                                        \
        nap_vm_cleanup(vm);                                                    \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        nap_vm_set_error_description(vm, error);                               \
        return NAP_FAILURE;                                                    \
    }                                                                          \
    } while(0);

/* Macro for creating an object */
#define NAP_MEM_ALLOC(count, type) (type*)calloc( (count), sizeof(type))

/* Macro for asserting a non-NULL variable and setting the VM's error in case*/
#define NAP_NN_ASSERT(vm,var)                                                  \
    do                                                                         \
    {                                                                          \
        if(var == NULL)                                                        \
        {                                                                      \
            char t[256];                                                       \
            SNPRINTF(t, 256, "MEM: out of memory file:[%s] line [%d] var:[%s]",\
                     __FILE__, __LINE__, #var);                                \
            NAP_REPORT_ERROR(vm, t);                                           \
        }                                                                      \
    } while(0);


/* Macro for creating a nap string. We need this in order to be able to support
   the UTF-32 BE without too much hassle */
#define NAP_STRING_ALLOC(vm, var, count) do {                                  \
   var = (char*)calloc( (count) * CC_MUL, sizeof(char));                       \
   NAP_NN_ASSERT(vm, var);                                                     \
   } while(0);

/* to calculate the length of a string without using too much CC_MUL*/
#define NAP_STRING_LEN(len) ((len) * CC_MUL)

/* to make less use of the CC_MUL in the code */
#define NAP_STRING_COPY(dest,src,count) memcpy(dest,src,NAP_STRING_LEN(count));


/* Macro for leaving the application in case of an unimplemented opcode */
#define NAP_NOT_IMPLEMENTED                                                    \
    do {                                                                       \
    char t[256];                                                               \
    SNPRINTF(t, 256, "NI: file [%s] line [%d] instr [%x] "                     \
                    "opcode [%x] at %" PRINT_u " (%" PRINT_x ")\n\n",          \
            __FILE__, __LINE__, vm->content[nap_ip(vm) - 1],                   \
            vm->cec->current_opcode, nap_ip(vm) - 1, nap_ip(vm) - 1);          \
    NAP_REPORT_ERROR(vm, t);                                                   \
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
        char s[512];                                                           \
        SNPRINTF(s, 512, "Variable [%s] not initialised correctly. "           \
                   "It has no instantiation.", var->name);                     \
        vm->error_description = s;                                             \
        return NAP_FAILURE;                                                    \
    }

#define CHECK_VARIABLE_TYPE(var, REQ_TYPE_CODE)                                \
    if(var->instantiation->type != REQ_TYPE_CODE)                              \
    {                                                                          \
        char s[512];                                                           \
        SNPRINTF(s, 512, "Variable [%s] has wrong type."                       \
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

/* Macros for handling the real numbers */
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

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
nap_int_t nap_read_immediate_int(struct nap_vm* vm, int* success);

/**
 * @brief nap_read_immediate_real
 * @param vm
 * @param success
 * @return
 */
nap_real_t nap_read_immediate_real(struct nap_vm* vm);

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
 */
char* convert_string_from_bytecode_file(struct nap_vm* vm,
                                        const char *src,
                                        size_t len,
                                        size_t dest_len,
                                        size_t *real_len);

/** Copies the return values from source to dest */
int nap_copy_return_values(const struct nap_vm* src, struct nap_vm* dst);

/*
 * Returns the current IP and will step the IP of the given virtual machine.
 */
uint64_t nap_step_ip(struct nap_vm* vm);

/*
 * Returns the current IP of the VM. Does not step the IP.
 */
uint64_t nap_ip(const struct nap_vm* vm);

/*
 * Sets the IP of  the VM to be the new value. The old IP is discarded,
 * thrown away.
 */
void nap_set_ip(struct nap_vm* vm, uint64_t new_ip);

/**
 * @brief nap_move_ip Moves the IP of th virtual machine
 * @param vm The virtual machine
 * @param delta The difference between the current and the new IP
 * @param direction 1 forward, -1 backward
 */
void nap_move_ip(struct nap_vm* vm, uint64_t delta, signed char direction);

/* Sets the given byte register in the VM to the specified value */
void nap_set_regb(struct nap_vm* vm, uint8_t register_index, nap_byte_t v);

/* Returns the given byte register from the given VM */
nap_byte_t nap_regb(struct nap_vm* vm, uint8_t register_index);

/* Sets the given int register in the VM to the specified value */
void nap_set_regi(struct nap_vm* vm, uint8_t register_index, nap_int_t v);

/* Returns the given int register from the given VM */
nap_int_t nap_regi(struct nap_vm* vm, uint8_t register_index);

/* Returns the given int register from the given VM */
nap_real_t nap_regr(struct nap_vm* vm, uint8_t register_index);

/* Sets the given regsiter to the given value */
void nap_set_regr(struct nap_vm* vm, uint8_t register_index, nap_real_t v);

/* Sets the given regsiter to the given value */
void nap_set_regidx(struct nap_vm* vm, uint8_t register_index, nap_int_t v);

/* Return the nap register index from the given position */
nap_int_t nap_regidx(struct nap_vm* vm, uint8_t register_index);

/* Returns the stack pointer */
int64_t nap_sp(struct nap_vm* vm);

/**
 * @brief init_string_register initializes the string register to a given value.
 *
 * Firstly it deallocates the memory (if any) which was previously allocated to
 * this string register. Then copies the string into the register. Also sets
 * the length registers.
 *
 * In case of error it preserves the value of the register.
 *
 * @param vm - the VM in which all this is happening
 * @param reg_idx - the target register index
 * @param target - the target string (UTF-32BE)
 * @param target_len - the length of the target string (string len, not memory
 * area length)
 *
 * @return NULL in case of error, or the new value of vm->regs[reg_idx]
 */
int nap_set_regs(struct nap_vm* vm, uint8_t register_index,
                                  const char* target, size_t target_len);

/* Returns the string register's value */
struct nap_string_register* nap_regs(struct nap_vm* vm, uint8_t register_index);

/* returns a real from the packed unsignd int */
nap_real_t unpack754(uint64_t i, unsigned bits, unsigned expbits);

#ifdef __cplusplus
}
#endif

#endif

