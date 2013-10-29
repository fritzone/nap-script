#ifndef _NBCI_H_
#define _NBCI_H_

#include <stdint.h>
#include <stdio.h>
#ifdef _WINDOWS
#define PRId64 "lld"
#define PRIu64 "lld"
#define PRIx64 "x"
#else
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#define REGISTER_COUNT 256                    /* number of registers in the VM*/
#define STACK_INIT   4096                     /* initially 4096 entries  */

#define _NOT_IMPLEMENTED \
    do {\
    fprintf(stderr, "NI: line [%d] instr [%d] opcode [%x] at %"PRIu64" (%" PRIx64 ")\n\n", \
            __LINE__, vm->content[vm->cc - 1], vm->current_opcode, vm->cc - 1, vm->cc - 1); \
    exit(99);\
    } while(0);

/* types for manipulating the addresses, indexes, etc */
typedef uint32_t nap_addr_t;
typedef uint32_t nap_mark_t;
typedef uint32_t nap_index_t;
typedef int64_t nap_number_t;

/**
 * @brief The TNapVM struct represents an instance of a virtual machine.
 * This structure contains all the necessary components that are used to
 * create a fully functional virtual machine.
 */
struct nap_vm
{
    /* Registers section */

    int64_t regi[REGISTER_COUNT];           /* the integer registers */
    char* regs[REGISTER_COUNT];             /* the string registers */
    int64_t regidx[REGISTER_COUNT];         /* the register indexes */
    uint8_t lbf;                            /* the last boolean flag of the machine */

    /* variables regarding the execution flow */

    uint8_t* content;                       /* the content of the file (ie the bytecodes)*/
    uint64_t cc;                            /* the instruction pointer */
    uint64_t call_frames[STACK_INIT];       /* the jump back points, the first address after the calls' index */
    uint32_t cfsize;                        /* the size of the call frames vector */
    uint8_t current_opcode;                 /* the current opcode */

    /* variables about the structure of the file*/

    uint32_t meta_location;
    uint32_t stringtable_location;
    uint32_t jumptable_location;
    uint8_t file_bitsize;                   /* the bit size: 0x32, 0x64*/


    /* variables for the meta table*/
    struct variable_entry** metatable;      /* the variables */
    size_t meta_size;                       /* the size of  the variables vector */

    /* variables for the stack */
    struct stack_entry** stack;             /* in this stack */
    uint64_t stack_size;                    /* initial stack size */
    int64_t stack_pointer;                  /* the stack pointer */

    /* variables for the jumptable */
    struct jumptable_entry** jumptable;   /* the jumptable itself */
    size_t jumptable_size;              /* the size of the jumptable */
    uint32_t jmpc;               /* counts the jumptable entries on loading*/

    /* variables for the stringtable */
    struct strtable_entry** stringtable;  /* the stringtable itself */
    uint64_t strt_size;                 /* the size of the stringtable */
};

/**
 * Sets the last boolean flag according to the operation found int current_opcode
 * @param vm - the virtual machine
 * @param reg - the registers value to check
 * @param immediate - against this value
 * @param current_opcode - the operation which is supposed to be executed
 */
void nap_vm_set_lbf_to_op_result(struct nap_vm* vm, nap_number_t reg, nap_number_t immediate, uint8_t opcode);

/**
 * Perform the given operation to be found in the opcode, stores the result in target
 * @param vm - the virtual machine
 * @param target - the target of the operation, and the first operand
 * @param operand - the operand on whcih we perform
 * @param opcode - the operation we perform
 * @throws a system error if the operation is division and the operand is zero
 */
void do_operation(struct nap_vm* vm, nap_number_t *target, nap_number_t operand, uint8_t opcode);

/**
 * Does a system cleanup of the virtual machine (free memory, etc...)
 * @param vm - the  virtual machine
 */
void cleanup(struct nap_vm* vm);

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

/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
void read_jumptable(struct nap_vm *vm, FILE* fp);

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

#endif
