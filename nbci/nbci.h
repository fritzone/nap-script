#ifndef _NBCI_H_
#define _NBCI_H_

#include <stdint.h>

#define REGISTER_COUNT 256                    /* number of registers in the VM*/
#define STACK_INIT   4096                     /* initially 4096 entries  */

/* generic variables regarding the file */
extern uint8_t file_bitsize;                  /* the bit size: 0x32, 0x64*/

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
    uint64_t call_frames[STACK_INIT];       /* the jump back points */
    uint32_t cfsize;                        /* the size of the call frames vector */
    uint8_t current_opcode;                 /* the current opcode */

    /* variables for the meta table*/

    struct variable_entry** metatable;      /* the variables */
    uint64_t meta_size;                     /* the size of  the variables vector */
};

/**
 * Sets the last boolean flag according to the operation found int current_opcode
 * @param vm - the virtual machine
 * @param reg - the registers value to check
 * @param immediate - against this value
 * @param current_opcode - the operation which is supposed to be executed
 */
void nap_vm_set_lbf_to_op_result(struct nap_vm* vm, int64_t reg, int64_t immediate, uint8_t opcode);

/**
 * Perform the given operation to be found in the opcode, stores the result in target
 * @param vm - the virtual machine
 * @param target - the target of the operation, and the first operand
 * @param operand - the operand on whcih we perform
 * @param opcode - the operation we perform
 * @throws a system error if the operation is division and the operand is zero
 */
void do_operation(struct nap_vm* vm, int64_t* target, int64_t operand, uint8_t opcode);

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

#endif
