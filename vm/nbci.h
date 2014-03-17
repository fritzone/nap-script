#ifndef _NBCI_H_
#define _NBCI_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Check windows */
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

/* Check GCC */
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#include <stdio.h>
#ifdef _WINDOWS
 #define PRINT_d "lld"
 #define PRINT_u "lld"
 #define PRINT_x "x"
 #define PRINT_st "u"
 #pragma warning (disable : 4127)
#else
 #define __STDC_FORMAT_MACROS
 #include <inttypes.h>
 #ifdef ENVIRONMENT64
  #define PRINT_d PRId64
  #define PRINT_u PRIu64
  #define PRINT_x PRIx64
 #define PRINT_st PRId64
 #else
  #define PRINT_d PRId64
  #define PRINT_u PRIu64
  #define PRINT_x PRIx64
  #define PRINT_st "zu"
 #endif
#endif

#include "nap_types.h"
#include "nap_structs.h"

#ifdef _MEM_DEBUG_
#define MEM_FREE(x) if((x)) { fprintf(stderr, "free:%p (%s:%d)\n", (x), __FILE__, __LINE__);  free((x)); }
#else
#define NAP_MEM_FREE(x) if(x){ free((x)); }
#endif

#define REGISTER_COUNT      255        /* number of registers in the VM*/
#define STACK_INIT          4096       /* initially 4096 entries  */
#define DEEPEST_RECURSION   4096       /* how deep we can dwelve into recursion */
#define MAX_BYTECODE_CHUNKS 255        /* the number of bytecode chunks that can be allocated */
#define OPCODE_COUNT        255        /* the number of possible opcodes */
#define INTERRUPT_COUNT     255        /* the number of possible interrupts */

/* the lbf initially is undecided, the first operation sets it, and it is
 * AND-ed with the result of the next boolean operations as long as it is
 * not cleared by a jlbf or clbf */
enum flag_status
{
    UNDECIDED = -1,
    FALSE     =  0,
    TRUE      =  1
};

/* whether this virtual machine runs in and EMBEDDED environement (as invoked
 * by the nap_runtime framework or in a STANDALONE environment (as invoked from
 * the command line) or if it was invoked while executing an interrupt */
enum environments
{
    EMBEDDED   = 0,
    STANDALONE = 1,
    INTERRUPT  = 2,
    FUN_CALL   = 3
};

/* the type of a label as present in the jump table
 */
enum label_type
{
    JUMP_DESTINATION = 0,
    FUNCTION         = 1,
    CLASS_METHOD     = 2,
    PARENT_FUNCTION  = 3
};

struct nap_vm;
/* the function pointer for an interrupt function */
typedef uint8_t (*interrupt)(struct nap_vm*);
/* the function pointer for the handling of a byteocde operation */
typedef int (*nap_op_handler)(struct nap_vm* vm);

/**
 * The nap_vm struct represents an instance of a virtual machine.
 * This structure contains all the necessary components that are used to
 * create a fully functional virtual machine. This actually is a linked list
 * of NAP Vm structures, each representing a virtual machine for a piece of code
 * that was started with the nap command "nap_execute()"
 */
struct nap_vm
{
    uint64_t cc;                            /* the instruction pointer */

    /* Registers section */

    nap_byte_t       regb    [REGISTER_COUNT]; /* the byte registers             */
    nap_int_t        regi    [REGISTER_COUNT]; /* the integer registers             */
    char*            regs    [REGISTER_COUNT]; /* the string registers, UTF-32BE    */
    nap_int_t        regidx  [REGISTER_COUNT]; /* the register indexes              */
    enum flag_status lbf;                     /* the last boolean flag             */
    uint8_t          mrc;                      /* number of registers of the VM. Used by pushall/popall  */
    size_t           regslens[REGISTER_COUNT]; /* the length of the string registers*/

    /* return values */
    nap_int_t rvi;                          /* the integer return value      */
    nap_byte_t rvb;                         /* the byte return value         */
    nap_real_t rvr;                         /* the real return value      */
    nap_string_t rvs;                       /* the string return value       */
    size_t rvl;                             /* the string return value's length */

    /* variables regarding the execution flow */

    uint8_t* content;                       /* the content of the file (ie the bytecodes)*/
    uint64_t call_frames[DEEPEST_RECURSION];/* the jump back points, the first address after the calls' index */
    uint32_t cfsize;                        /* the size of the call frames vector */
    uint8_t  current_opcode;                /* the current opcode */

    /* variables about the structure of the file */

    uint32_t  meta_location;                /* the location of the metatable in the file */
    uint32_t  stringtable_location;         /* the location of the stringtable in the file */
    uint32_t  jumptable_location;           /* the location of the jumptable in the file */
    uint32_t  funtable_location;            /* the location of the fun table in the file */
    uint8_t   file_bitsize;                 /* the bit size: 0x32, 0x64*/


    /* variables for the meta table */
    struct variable_entry** metatable;      /* the variables */
    size_t meta_size;                       /* the size of  the variables vector */

    /* variables for the stack */
    struct stack_entry** stack;             /* in this stack */
    uint64_t stack_size;                    /* initial stack size */
    int64_t stack_pointer;                  /* the stack pointer, starts from 0, grows */

    /* variables for the jumptable */
    struct jumptable_entry** jumptable;     /* the jumptable itself */
    size_t jumptable_size;                  /* the size of the jumptable */
    uint32_t jmpc;                          /* counts the jumptable entries on loading*/

    /* variables for the stringtable */
    struct strtable_entry** stringtable;    /* the stringtable itself */
    size_t strt_size;                       /* the size of the stringtable */

    /* variables for the funtable */
    struct funtable_entry** funtable;       /* the function table */
    size_t funtable_entries;                /* how many entries in the function table */

    /* other variables */
    enum environments environment;          /* whether this is run as embedded in an app or a standalone application */
    uint8_t flags;                          /* Some flags used by the VM, not used right now */

    /* runtime code execution support */
    struct nap_bytecode_chunk** btyecode_chunks; /* holds all the compiled bytechunks */
    struct nap_vm* parent;                  /* the parent VM of this. NULL if main VM */
    size_t chunk_counter;                   /* counts the number of chunks */
    size_t allocated_chunks ;               /* how many chunks have we allocated */

    /* the interrupt vectors of the implementation */
    interrupt interrupts[INTERRUPT_COUNT];

    /* the array of opcode handlers */
    nap_op_handler opcode_handlers[OPCODE_COUNT];

    /* what error each opcode will report if not succesfully executed */
    int opcode_error_codes[OPCODE_COUNT];

    /* error handling */

    /* The last error message. Must NOT be freed on cleanup. In case of a secondary
     * error it contains it's description not the main erros description. */
    char* error_message;

    /* sometimes there is an error description too, not only an error message.
     * This must be freed, since it was allocated internally by methods wishing
     * to report something funny. */
    char* error_description;

    /* The last error code. 0 if no error.
     * First word (bits: 16->32): secondary error code (or 0 if no secondary
     * error is present).
     * Last word (bits 0->16): the main error code.
     * A Word is 16 bits.
     * Error codes in nap_consts.h or use nap_get_error_description(int) */
    uint32_t error_code;
};

/**
 * Does a system cleanup of the virtual machine (free memory, etc...)
 * @param vm - the  virtual machine
 */
void nap_vm_cleanup(struct nap_vm* vm);

/**
 * Dumps the stage of the VM into the given descriptor
 * @param vm
 */
void dump(struct nap_vm* vm, FILE *fp);

/**
 * Loads the given bytecode file and creates a new virtual machine
 * @param filename - the compiled bytecode
 * @return - the
 */
struct nap_vm* nap_vm_load(const char* filename);

/**
 * Starts the processing cycle of the virtual machine, executes the bytecode
 * @param vm - the VM to run
 */
void nap_vm_run(struct nap_vm* vm);

/**
 * @brief creates a new virtual machine and injects the given bytecode
 * @param bytecode
 * @param bytecode_len
 * @return
 */
struct nap_vm* nap_vm_inject(uint8_t* bytecode, int bytecode_len, enum environments target);

/**
 * @brief nap_vm_get_int returns the value of the int variable called "name"
 *
 * The variable must have been defined in the global namespace in the script
 * otherwise it is not relevant.
 *
 * The method will populate the *found with 1 if the variable was found and with
 * 0 if the variable was not found.
 *
 * The method will return the value of the variable if found or NAP_NO_VALUE if
 * not found.
 *
 * The *found parameter should be used for error checking, not the return value.
 *
 * @param vm - the VM in which this runs
 * @param name - the name of the variable
 * @param [out] found - if we found it or not. Set to 1 or 0.
 * @return the value, or NAP_NO_VALUE if not found
 */
nap_int_t nap_vm_get_int(struct nap_vm* vm, char* name, int* found);

/**
 * @brief nap_vm_get_byte returns the value of the byte variable called "name"
 *
 * The variable must have been defined in the global namespace in the script
 * otherwise it is not relevant.
 *
 * The method will populate the *found with 1 if the variable was found and with
 * 0 if the variable was not found.
 *
 * The method will return the value of the variable if found or 0x0F if
 * not found.
 *
 * The *found parameter should be used for error checking, not the return value.
 *
 * @param vm - the VM in which this runs
 * @param name - the name of the variable
 * @param [out] found - if we found it or not. Set to 1 or 0.
 * @return the value, or NAP_NO_VALUE if not found
 */
nap_byte_t nap_vm_get_byte(struct nap_vm* vm, char* name, int* found);

/**
 * @brief nap_vm_get_string return the value of the variable called "name"
 *
 * The variable must have been defined in the global namespace in the script
 * otherwise it is not relevant.
 *
 * The method will populate the *found with 1 if the variable was found and with
 * 0 if the variable was not found.
 *
 * The nap VM internally stores the strings as UTF-32BE, however this method
 * converts the internal string into the host encoding and returns that value,
 * i.e. a \0 terminated local string. The user must free the returned value to
 * avoid memory leaks. The string is allocated with calloc(), please use the
 * corresponding free() call.
 *
 * @param vm - the VM in which this runs
 * @param name - the name of the variable
 * @param [out] found - if we found it or not. Set to 1 or 0.
 *
 * @return the value, or NULL if not found
 */
char* nap_vm_get_string(struct nap_vm* vm, char* name, int* found);

/**
 * @brief Sets the error description of the virtual machine.
 *
 * @param vm the target virtual machine
 * @param error_desc the error description
 *
 * The method takes a copy of the error_desc string.
 *
 * @return this function always return NAP_FAILURE
 */
int nap_vm_set_error_description(struct nap_vm *vm, const char* error_desc);

struct funtable_entry* nap_vm_get_method(struct nap_vm* vm, const char* method_name);

#ifdef __cplusplus
}
#endif

#endif
