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
 #define PRINT_x "llX"
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

#define REGISTER_COUNT      255        /* number of registers in the VM*/
#define STACK_INIT          4096       /* initially 4096 entries  */
#define DEEPEST_RECURSION   1024       /* how deep we can dwelve into recursion */
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
typedef uint16_t (*interrupt)(struct nap_vm*);

/* the function pointer for the handling of a byteocde operation */
typedef int (*nap_op_handler)(struct nap_vm* vm);

/* the function pointer for handling various mov targets */
typedef int (*nap_mov_handler)(struct nap_vm*);

/* a structure representing a string register */
struct nap_string_register
{
    /* the length of the string register. Real length, not the UTF-32BE length */
    size_t l;

    /* the actual string in UTF-32 BE representation, not NULL terminated */
    char *s;
};

/**
 * The startup_configuration struct provides configuration options for the
 * virtual machine that will be used when creating it.
 */
struct startup_configuration
{
    /* the size of  the stack.
     * Command line parameter: -s NUMBER where NUMBER is the total
     * number of items that can be placed in the stack */
    size_t stack_size;

    /* How deep can we go into recursion. Maximum 65,535. */
    uint16_t deepest_recursion;
};

/**
 * The execution context of a virtual machine is the data structure holding the
 * information about the currently executed thread (of an application). In a nap
 * virtual machine each thread has its own instruction pointer, set of registers
 * return values, call frames and stack. Upon a thread creation the parents' stack
 * (which can be either a thread or the main app) is duplicated into the stack
 * of the newly created thread and the stack pointer updated as required to
 * point to the top of the threads' stack.
 *
 * Upon execution of an instruction the thread scheduler might switch to another
 * thread. This is a fake multithreading mechanism since there is always one
 * active thread which is being executed.
 *
 * The "thread" executor is watching the "vm->crs" flag and upon a change in its
 * status from 0 to 1:
 *  - it takes the next execution context from the vm->ecs
 *  - sets the flag back to 0
 *
 * The thread executor decides upon switching to a new context based on the
 * following:
 *  - after each completed instruction?
 *  - upon a specific timer after each completed instruction?
 *  - always upon the creation of a new execution context
 */
struct nap_execution_context
{

    uint64_t         cc;                       /* the instruction pointer     */
    uint64_t        lia;                       /* last instruction address    */

    /* registers ofthe VM */
    nap_byte_t                 regb  [REGISTER_COUNT]; /* the byte registers  */
    nap_int_t                  regi  [REGISTER_COUNT]; /* the int registers   */
    nap_real_t                 regr  [REGISTER_COUNT]; /* the real registers  */
    nap_int_t                  regidx[REGISTER_COUNT]; /* the index register  */
    struct nap_string_register regs  [REGISTER_COUNT]; /* the string registers*/
    enum flag_status lbf;                              /* the last bool flag  */

    /* return values */
    nap_string_t rvs;                     /* the string return value          */
    size_t       rvl;                     /* the string return value's length */
    nap_int_t    rvi;                     /* the integer return value         */
    nap_byte_t   rvb;                     /* the byte return value            */
    nap_real_t   rvr;                     /* the real return value            */

    /* variables regarding the execution flow */
    uint8_t  current_opcode;                /* the current opcode */
#ifndef PREFER_DYNAMIC_ALLOCATION
    uint64_t call_frames[DEEPEST_RECURSION];/* the jump back points, the first address after the calls' index */
    int64_t ebp_stack[DEEPEST_RECURSION];
#else
    uint64_t* call_frames;                  /* the jump back points, the first address after the calls' index */
    int64_t* ebp_stack;                     /* the stack of the BP register */
#endif

    uint32_t cfsize;                        /* the size of the call frames vector */

    /* variables for the stack */
#ifndef PREFER_DYNAMIC_ALLOCATION
    struct stack_entry* stack[STACK_INIT];  /* The stack of the VM */
#else
    struct stack_entry** stack;             /* The stack of the VM */
#endif

    int64_t stack_pointer;                  /* the stack pointer, starts from 0, grows */
    int64_t bp;                             /* the base pointer, initialized to SP on a function call */

};

/**
 * The nap_vm struct represents an instance of a virtual machine.
 * This structure contains all the necessary components that are used to
 * create a fully functional virtual machine. This actually is a linked list
 * of NAP Vm structures, each representing a virtual machine for a piece of code
 * that was started with the nap command "nap_execute()"
 */
struct nap_vm
{
    /* The current execution context. This is automatically modified by the
     * thread scheduler upon more than one threads */
    struct nap_execution_context* cec;
    struct nap_execution_context** ecs;     /* The list of execution contexts */
    size_t ecs_cnt;                  /* How many execution contexts are there */
    size_t cec_idx;                  /* The index of the curent execution context */

    uint8_t csr; /* Context Switch Requested? 1 if it was requested, 0 otherwise */

    uint8_t mrc;    /* number of registers of the VM. Used by pushall/popall  */

    uint8_t* content;                       /* the content of the file (ie the bytecodes)*/

    /* variables about the structure of the file */

    uint32_t  meta_location;                /* the location of the metatable in the file */
    uint32_t  stringtable_location;         /* the location of the stringtable in the file */
    uint32_t  jumptable_location;           /* the location of the jumptable in the file */
    uint32_t  funtable_location;            /* the location of the fun table in the file */
    uint32_t  classtable_location;          /* the location of the class table in the file */
    uint8_t   file_bitsize;                 /* the bit size: 0x32, 0x64*/
    uint32_t  max_marks;                    /* the number of maximum marks in the bytecode */

    struct stack_entry** marks_list;        /* the list of marks in the bytecode */

    /* variables for the meta table */
    struct variable_entry** metatable;      /* the variables */
    size_t meta_size;                       /* the size of  the variables vector */

    /* variables for the jumptable */
    struct jumptable_entry** jumptable;     /* the jumptable itself */
    size_t jumptable_size;                  /* the size of the jumptable */
    uint32_t jmpc;                          /* counts the jumptable entries on loading*/

    /* variables for the stringtable */
    struct strtable_entry** stringtable;    /* the stringtable itself */
    size_t strt_size;                       /* the size of the stringtable */

    /* variables for the funtable */
    struct funtable_entry** funtable;       /* the function table */
    size_t funtable_size;                   /* how many entries in the function table */

    /* variables for the classtable */
    struct funtable_entry** classtable;     /* the function table */
    size_t classtable_size;                 /* how many entries in the function table */

    /* other variables */
    enum environments environment;          /* whether this is run as embedded in an app or a standalone application */
    uint8_t flags;                          /* Some flags used by the VM, not used right now */

    /* runtime code execution support */
    struct nap_bytecode_chunk** btyecode_chunks; /* holds all the compiled bytechunks */
    struct nap_vm* parent;                  /* the parent VM of this. NULL if main VM */
    size_t chunk_counter;                   /* counts the number of chunks */
    size_t allocated_chunks;                /* how many chunks have we allocated */

    /* the interrupt vectors of the implementation */
    interrupt interrupts[INTERRUPT_COUNT];

    /* the array of opcode handlers */
    nap_op_handler opcode_handlers[OPCODE_COUNT];

    /* the array of mov target handlers */
    nap_mov_handler mov_handlers[OPCODE_COUNT];

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

    /* The internal register stacks' index */
    size_t reg_stack_idx;
#ifndef PREFER_DYNAMIC_ALLOCATION
    /* the register "stack" used to save and restore the registers */
    nap_int_t regi_stack[DEEPEST_RECURSION][REGISTER_COUNT];
    nap_byte_t regb_stack[DEEPEST_RECURSION][REGISTER_COUNT];
#else
    nap_int_t** regi_stack;
    nap_byte_t** regb_stack;
#endif

    /* the startup configuration of the VM */
    struct startup_configuration* config;
};

/**
 * Does a system cleanup of the virtual machine (free memory, etc...)
 * @param vm - the  virtual machine
 */
void nap_vm_cleanup(struct nap_vm* vm);

/**
 * Dumps the variables of the VM into the given file descriptor
 *
 * @param vm the virtual machine to dump
 * @param fp the file in which to dump the textual data
 */
void nap_vm_dump(struct nap_vm* vm, FILE *fp);

/**
 * Dumps the stack of the VM into the given descriptor
 *
 * @param vm the virtual machine to dump
 * @param fp the file in which to dump the textual data
 */
void dump_stack(struct nap_vm* vm, FILE *fp);

/**
 * Loads the given bytecode file and creates a new virtual machine
 * @param filename - the compiled bytecode
 * @return - the
 */
struct nap_vm* nap_vm_load(const char* filename, struct startup_configuration *config);

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
struct nap_vm* nap_vm_inject(uint8_t* bytecode, int bytecode_len, enum environments target, struct startup_configuration *config);

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
 * @brief nap_vm_get_real returns the value of the int variable called "name"
 *
 * The variable must have been defined in the global namespace in the script
 * otherwise it is not relevant.
 *
 * The method will populate the *found with 1 if the variable was found and with
 * 0 if the variable was not found.
 *
 * The method will return the value of the variable if found or \c NAP_NO_VALUE
 * if not found.
 *
 * The *found parameter should be used for error checking, not the return value.
 *
 * @param vm - the VM in which this runs
 * @param name - the name of the variable
 * @param [out] found - if we found it or not. Set to 1 or 0.
 * @return the value, or NAP_NO_VALUE if not found
 */
nap_real_t nap_vm_get_real(struct nap_vm* vm, char* name, int* found);

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
 * avoid memory leaks. The string is allocated with \c calloc(), please use the
 * corresponding \c free() call.
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
