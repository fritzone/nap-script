#ifndef _NBCI_H_
#define _NBCI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#ifdef _WINDOWS
#define PRId64 "lld"
#define PRIu64 "lld"
#define PRIx64 "x"
#pragma warning (disable : 4127)
#else
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#include "nap_types.h"
#include "nap_structs.h"

#ifdef _MEM_DEBUG_
#define _FREE(x) if((x)) { fprintf(stderr, "free:%p (%s:%d)\n", (x), __FILE__, __LINE__);  free((x)); }
#else
#define MEM_FREE(x) if(x){ free((x)); }
#endif

#define REGISTER_COUNT    256        /* number of registers in the VM*/
#define STACK_INIT        4096       /* initially 4096 entries  */
#define DEEPEST_RECURSION 4096       /* how deep we can dwelve into recursion */

/* Macro for leaving the application in case of an error */
#define _NOT_IMPLEMENTED \
    do {\
    fprintf(stderr, "NI: file [%s] line [%d] instr [%x] "\
                    "opcode [%x] at %"PRIu64" (%" PRIx64 ")\n\n", \
            __FILE__, __LINE__, vm->content[vm->cc - 1], \
            vm->current_opcode, vm->cc - 1, vm->cc - 1); \
    exit(EXIT_FAILURE);\
    } while(0);


/* the lbf initially is undecided, the first operation sets it, and it is
 * AND-ed with the result of the next boolean operations as long as it is
 * not cleared by a jlbf or clbf */
enum flag_staus
{
    UNDECIDED = -1,
    FALSE     =  0,
    TRUE      =  1
};

/* whether this virtual machine runs in and EMBEDDED environement (as invoked
 * by the nap_runtime framework or in a STANDALONE environment (as invoked from
 * the command line) */
enum environments
{
    EMBEDDED = 0,
    STANDALONE = 1
};

struct nap_vm;
/* the function pointer for an interrupt function */
typedef uint8_t (*interrupt)(struct nap_vm*);

/**
 * The TNapVM struct represents an instance of a virtual machine.
 * This structure contains all the necessary components that are used to
 * create a fully functional virtual machine. This actually is a linked list
 * of NAP Vm structures, each representing a virtual machine for a piece of code
 * that was started with the nap command "execute()"
 */
struct nap_vm
{
    /* Registers section */

    nap_int_t       regi  [REGISTER_COUNT]; /* the integer registers         */
    char*           regs  [REGISTER_COUNT]; /* the string registers          */
    nap_int_t       regidx[REGISTER_COUNT]; /* the register indexes          */
    enum flag_staus lbf;                    /* the last boolean flag         */
    uint8_t         mrc;                    /* number of registers of the VM */

    /* return values */
    nap_int_t rvi;                          /* the integer return value      */

    /* variables regarding the execution flow */

    uint8_t* content;                       /* the content of the file (ie the bytecodes)*/
    uint64_t cc;                            /* the instruction pointer */
    uint64_t call_frames[STACK_INIT];       /* the jump back points, the first address after the calls' index */
    uint32_t cfsize;                        /* the size of the call frames vector */
    uint8_t  current_opcode;                /* the current opcode */

    /* variables about the structure of the file */

    uint32_t  meta_location;                /* the location of the metatable in the file */
    uint32_t  stringtable_location;         /* the location of the stringtable in the file */
    uint32_t  jumptable_location;           /* the location of the jumptable in the file */
    uint8_t   file_bitsize;                 /* the bit size: 0x32, 0x64*/

    /* variables for the meta table */

    struct variable_entry** metatable;      /* the variables */
    size_t meta_size;                       /* the size of  the variables vector */

    /* variables for the stack */

    struct stack_entry** stack;             /* in this stack */
    uint64_t stack_size;                    /* initial stack size */
    int64_t stack_pointer;                  /* the stack pointer */

    /* variables for the jumptable */

    struct jumptable_entry** jumptable;     /* the jumptable itself */
    size_t jumptable_size;                  /* the size of the jumptable */
    uint32_t jmpc;                          /* counts the jumptable entries on loading*/

    /* variables for the stringtable */

    struct strtable_entry** stringtable;    /* the stringtable itself */
    uint64_t strt_size;                     /* the size of the stringtable */

    enum environments environment;          /* whether this is run as embedded in an app or a standalone application */
    uint8_t flags;                          /* Some flags used by the VM */

    /* runtime code execution support */
    struct nap_bytecode_chunk** btyecode_chunks; /* holds all the compiled bytechunks */
    struct nap_vm* parent;                  /* the parent VM of this. NULL if main VM */
    size_t chunk_counter;                   /* counts the number of chunks */
    size_t allocated_chunks ;               /* how many chunks have we allocated */
    /* interrupt handling */

    /* the interrupt vectors of the implementation */
    interrupt interrupts[255];
};

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
struct nap_vm* nap_vm_inject(uint8_t* bytecode, int bytecode_len);

/**
 * @brief nap_vm_get_int return the value of the variable called "name"
 * @param vm - the VM in which this runs
 * @param name - the name of the variable
 * @param [out] found - if we found it or not. Set to 1 or 0.
 * @return the value, or 0x0BADF00D if not found
 */
nap_int_t nap_vm_get_int(struct nap_vm* vm, char* name, int* found);

#ifdef __cplusplus
}
#endif

#endif
