#ifndef _NBCI_IMPL_H_
#define _NBCI_IMPL_H_

#define NAP_MEM_DEBUG
#undef NAP_MEM_DEBUG

#ifdef __cplusplus
extern "C" {
#endif

#include "nap_types.h"
#include "byte_order.h"
#include "stack.h"

#include <stddef.h>
#include <string.h>

/* types for manipulating the addresses, indexes, etc */
typedef uint32_t nap_addr_t;    /* the type of a NAP address*/
typedef uint32_t nap_mark_t;    /* the type of a marker pushed on the stack */
typedef uint32_t nap_index_t;   /* the type of an index */

struct nap_vm;
struct nap_string_register;

/* The directions in which the IP can move */
#define BACKWARD -1
#define FORWARD   1

#define REAL_COMPARISON_DELTA 0.00000001

#if defined(_MSC_VER)
#define SNPRINTF _snprintf_s
#define strtoll _strtoi64
#define MAX_BUF_SIZE(x) (x), _TRUNCATE
#else
#define SNPRINTF snprintf
#define MAX_BUF_SIZE(x) (x)
#endif

/* Macro for freeing a piece of memory */
#ifdef NAP_MEM_DEBUG
#define NAP_MEM_FREE(x) if((x)) { fprintf(stderr, "free:%p (%s:%d)\n", (x), __FILE__, __LINE__);  free((x)); }
#else
#define NAP_MEM_FREE(x) do { if(x){ free((x)); } } while(0);
#endif


/* Macro for creating an object */
#ifdef NAP_MEM_DEBUG
#include <stdlib.h>
void* allocator(size_t count, const char* fn, long line);
#define NAP_MEM_ALLOC(count, type) (type*)allocator( (count) * sizeof(type), __FILE__, __LINE__)
#else
#define NAP_MEM_ALLOC(count, type) (type*)calloc( (count), sizeof(type))
#endif

#define NAP_REPORT_ERROR(vm, error)                                            \
    do                                                                         \
    {                                                                          \
    if(vm->environment == STANDALONE)                                          \
    {                                                                          \
        fprintf(stderr, "%s\n", error);                                        \
        nap_vm_cleanup(vm);                                                    \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        nap_vm_set_error_description(vm, error);                               \
    }                                                                          \
    } while(0);


/* Macro for asserting a non-NULL variable and setting the VM's error in case*/
#define NAP_NN_ASSERT(vm,var)                                                  \
    do                                                                         \
    {                                                                          \
        if(var == NULL)                                                        \
        {                                                                      \
            char t[256] = {0};                                                 \
            SNPRINTF(t, MAX_BUF_SIZE(255), "MEM: out of memory file:[%s] line [%d] var:[%s]",\
                     __FILE__, __LINE__, #var);                                \
            NAP_REPORT_ERROR(vm, t);                                           \
            return NAP_FAILURE;                                                \
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

#ifdef _MSC_VER
#define SIZE_T_PRINT (unsigned long long)
#else
#define SIZE_T_PRINT
#endif

/* Check if the required index is allowed for the given variable */
#define ASSERT_VARIABLE_INDEX_ALLOWED(var, idx)                                \
    if((signed)idx < 0 || var->instantiation->len <= idx)                      \
    {                                                                          \
        char s[512] = {0};                                                     \
        SNPRINTF(s, MAX_BUF_SIZE(511), "Invalid index for variable [%s]. "     \
               "Req:[%"PRINT_d"] Avail:[%"PRINT_d"]", var->name,               \
               SIZE_T_PRINT idx,                                               \
               SIZE_T_PRINT var->instantiation->len);                          \
        vm->error_description = s;                                             \
        return NAP_FAILURE;                                                    \
    }

#define ASSERT_INDEX_RELATIONS(var, start_idx, end_idx)                        \
    if(start_idx > end_idx)                                                    \
    {                                                                          \
        char s[512] = {0};                                                     \
        SNPRINTF(s, MAX_BUF_SIZE(511), "Invalid: start index > end index for variable [%s]. "\
               "Start:[%"PRINT_d"] End:[%"PRINT_d"]", var->name, SIZE_T_PRINT start_idx, SIZE_T_PRINT end_idx);\
        vm->error_description = s;                                             \
        return NAP_FAILURE;                                                    \
    }


/* Macro for leaving the application in case of an unimplemented opcode */
#define NAP_NOT_IMPLEMENTED                                                    \
    do {                                                                       \
	char t[256] = {0}, offending_command[256] = {0}, tmp[32] = {0};            \
    uint64_t bc = 0;                                                           \
    for(bc = vm->cec->lia; bc != nap_ip(vm); bc++) {                           \
        SNPRINTF(tmp, MAX_BUF_SIZE(31), "%x ", vm->content[bc]);               \
        strcat(offending_command, tmp);                                        \
    }                                                                          \
    SNPRINTF(t, MAX_BUF_SIZE(255), "NI: file [%s] line [%d] instr [%x] "       \
                    "opcode [%x] at %" PRINT_u " (%" PRINT_x ") cmd: %s\n\n",  \
            __FILE__, __LINE__, vm->content[nap_ip(vm) - 1],                   \
            vm->cec->current_opcode, nap_ip(vm) - 1, nap_ip(vm) - 1,           \
            offending_command);                                                \
    dump_stack(vm, stderr);                                                    \
    nap_vm_dump(vm, stderr);                                                   \
    NAP_REPORT_ERROR(vm, t);                                                   \
    exit(EXIT_FAILURE);                                                        \
    return NAP_FAILURE;                                                        \
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
        char s[512] = {0};                                                     \
        SNPRINTF(s, MAX_BUF_SIZE(512), "Variable [%s] not initialised correctly. "\
                   "It has no instantiation.", var->name);                     \
        vm->error_description = s;                                             \
        return NAP_FAILURE;                                                    \
    }

#define CHECK_VARIABLE_TYPE(var, REQ_TYPE_CODE)                                \
    if(var->instantiation->type != REQ_TYPE_CODE)                              \
    {                                                                          \
        char s[512] = {0};                                                     \
        SNPRINTF(s, MAX_BUF_SIZE(512), "Variable [%s] has wrong type."         \
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

#ifndef VM_IPSP_AS_MACRO

/** Returns the current IP and will step the IP of the given virtual machine. */
extern 
#if !defined(_MSC_VER)
inline
#endif
uint64_t nap_step_ip(struct nap_vm* vm);

/* Returns the current IP of the VM. Does not step the IP. */
extern
#if !defined(_MSC_VER)
inline
#endif
uint64_t nap_ip(const struct nap_vm* vm);

/** Sets the IP of  the VM to be the new value. The old IP is discarded. */
extern
#if !defined(_MSC_VER)
inline
#endif
void nap_set_ip(struct nap_vm* vm, uint64_t new_ip);

/** Moves the IP of th virtual machine in the specified direction (1 forward, -1 backward) */
extern
#if !defined(_MSC_VER)
inline
#endif
void nap_move_ip(struct nap_vm* vm, uint64_t delta, signed char direction);

/* Returns the stack pointer of the virtual machine current execution context */
extern
#if !defined(_MSC_VER)
inline
#endif
int64_t nap_sp(struct nap_vm* vm);

/* Sets the given int register in the VM to the specified value */
extern
#if !defined(_MSC_VER)
inline
#endif
void nap_set_regi(struct nap_vm* vm, uint8_t register_index, nap_int_t v);

/** Fetch a marker from the bytecode stream */
nap_mark_t nap_fetch_mark(struct nap_vm* vm);

/** Fetch an index from the bytecode stream */
nap_index_t nap_fetch_index(struct nap_vm* vm);

/* Returns the given int register from the given VM */
nap_int_t nap_regi(struct nap_vm* vm, uint8_t register_index);

#else

#define nap_step_ip(vm) vm->cec->cc ++

#define nap_ip(vm) vm->cec->cc

#define nap_set_ip(vm, new_ip) vm->cec->cc = new_ip

#define nap_move_ip(vm, delta, direction) vm->cec->cc = vm->cec->cc + direction * delta

#define nap_sp(vm) vm->cec->stack_pointer

#define nap_set_regi(vm, register_index, v) vm->cec->regi[register_index] = v

#define nap_regi(vm, register_index) vm->cec->regi[register_index]

#define nap_fetch_index(vm) htovm_32( *(nap_index_t*)(vm->content + vm->cec->cc) ); vm->cec->cc = vm->cec->cc + sizeof(nap_index_t)

#define nap_fetch_mark(vm) *(nap_mark_t*)(vm->content + nap_ip(vm)); vm->cec->cc = vm->cec->cc + sizeof(nap_mark_t)

#endif


/* Sets the given byte register in the VM to the specified value */
void nap_set_regb(struct nap_vm* vm, uint8_t register_index, nap_byte_t v);

/* Returns the given byte register from the given VM */
nap_byte_t nap_regb(struct nap_vm* vm, uint8_t register_index);




/* Returns the given int register from the given VM */
nap_real_t nap_regr(struct nap_vm* vm, uint8_t register_index);

/* Sets the given regsiter to the given value */
void nap_set_regr(struct nap_vm* vm, uint8_t register_index, nap_real_t v);

/* Sets the given regsiter to the given value */
void nap_set_regidx(struct nap_vm* vm, uint8_t register_index, nap_int_t v);

/* Return the nap register index from the given position */
nap_int_t nap_regidx(struct nap_vm* vm, uint8_t register_index);

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

/**
 * @brief nap_int_to_string converts the given number to a nap string (UTF32-BE)
 *
 * The caller manages the string
 *
 * @param value [in]  the value to convert
 * @param len   [out] the length of the outgoing string
 *
 * @return the converted UTF32-BE string
 */
char* nap_int_to_string(nap_int_t value, size_t* len);

/**
 * @brief deliver_flat_index delivers a flat memory index for the given variable
 *
 * The calculation is done based on the value of the index registers of the VM.
 *
 * @param vm           [in] the host VM
 * @param ve           [in] the variable entry on which we are working
 * @param used_indexes [in] the number of used index
 * @param error        [out] the error if any.
 *
 * @return a flat location representing the index
 */
int64_t deliver_flat_index(struct nap_vm* vm,
                           const struct variable_entry* ve,
                           uint8_t used_indexes, char** error);

/**
 * @brief nap_string_to_number_int returns an int number from the given string
 *
 * @param to_conv this is the string that will be converted into a number.
 *        It is encoded with UTF-32BE
 * @param len the length of the string, not the length of the memory area
 * @param error [out] will be populated with NAP_SUCCESS in case of success
 *        or NAP_FAILURE in case of failure
 *
 * @return NAP_NO_VALUE in case of memory allocation error (in this case *error
 *        is populated to NAP_FAILURE) or the number as converted by strtoll and
 *        *error populated to NAP_FAILURE in case of strtoll failed, or the
 *        number as converted by strtoll and the *error set to NAP_SUCCESS in
 *        case of a succesfull conversion
 */
nap_int_t nap_string_to_number_int(struct nap_vm* vm,
                                   const char* to_conv,
                                   size_t len,
                                   int* error);

/**
 * @brief nap_string_to_number_real returns a real number from the given string
 *
 * @param to_conv this is the string that will be converted into a number.
 *        It is encoded with UTF-32BE
 * @param len the length of the string, not the length of the memory area
 * @param error [out] will be populated with NAP_SUCCESS in case of success
 *        or NAP_FAILURE in case of failure
 *
 * @return NAP_NO_VALUE in case of memory allocation error (in this case *error
 *        is populated to NAP_FAILURE) or the number as converted by strtof and
 *        *error populated to NAP_FAILURE in case of strtoll failed, or the
 *        number as converted by strtoll and the *error set to NAP_SUCCESS in
 *        case of a succesfull conversion
 */
nap_real_t nap_string_to_number_real(struct nap_vm* vm,
                                     const char* to_conv,
                                     size_t len,
                                     int* error);


/**
 * @brief opcode_name returns the name ofthe given opcode
 * @param opcode
 * @return
 */
const char* opcode_name(int opcode);


#ifdef __cplusplus
}
#endif

#endif

