#include "comparison.h"
#include "opcodes.h"
#include "nbci.h"
#include "metatbl.h"
#include "stack.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>
#include <stdint.h>

/**
 * Sets the last boolean flag according to the operation found int current_opcode
 * @param vm - the virtual machine
 * @param reg - the registers value to check
 * @param immediate - against this value
 * @param current_opcode - the operation which is supposed to be executed
 */
static int nap_vm_set_lbf_to_op_result(struct nap_vm* vm, nap_int_t reg,
                                        nap_int_t immediate, uint8_t opcode)
{
    register uint8_t temp_lbf;

    if(opcode == OPCODE_EQ)
    {
        temp_lbf = (reg == immediate);
    }
    else
    if(opcode == OPCODE_NEQ)
    {
        temp_lbf = (reg != immediate);
    }
    else
    if(opcode == OPCODE_LT)
    {
        temp_lbf = (reg <  immediate);
    }
    else
    if(opcode == OPCODE_GT)
    {
        temp_lbf = (reg >  immediate);
    }
    else
    if(opcode == OPCODE_LTE)
    {
        temp_lbf = (reg <= immediate);
    }
    else
    if(opcode == OPCODE_GTE)
    {
        temp_lbf = (reg >= immediate);
    }
    else
    {
        _NOT_IMPLEMENTED
    }

    if(vm->lbf == UNDECIDED)
    {
        vm->lbf = temp_lbf;
    }
    else
    {
        vm->lbf &= temp_lbf;
    }

    return NAP_SUCCESS;
}

int nap_comparison(struct nap_vm* vm)
{
    uint8_t cmp_first = vm->content[vm->cc ++];   /* what to check (reg only)*/

    if(cmp_first == OPCODE_REG) /* do we check a register? */
    {
        uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
            uint8_t cmp_second = vm->content[vm->cc ++]; /* what are we checking against*/

            if(cmp_second == OPCODE_IMMEDIATE) /* comparing int register with immediate value (1,..) */
            {
                nap_int_t immediate = nap_read_immediate(vm);
                return nap_vm_set_lbf_to_op_result(vm, vm->regi[register_index], immediate, vm->current_opcode);
            }
            else
            if(cmp_second == OPCODE_VAR) /* int register compared to a variable */
            {
                nap_index_t var_index = nap_fetch_index(vm);
                struct variable_entry* ve = nap_fetch_variable(vm, var_index);
                ASSERT_NOT_NULL_VAR(ve);

                if(ve->instantiation->type == OPCODE_INT) /* comparing int register with an int variable */
                {
                    return nap_vm_set_lbf_to_op_result(vm, vm->regi[register_index],
                                                *(nap_int_t*)ve->instantiation->value, vm->current_opcode);
                }
                else /* comparing int register with another kind of variable */
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            if(cmp_second == OPCODE_REG)
            {
                uint8_t second_register_type = vm->content[vm->cc ++]; /* int/string/float...*/

                if(second_register_type == OPCODE_INT) /* comparing int reg with another INT type register */
                {
                    uint8_t second_register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
                    return nap_vm_set_lbf_to_op_result(vm, vm->regi[register_index],
                                                vm->regi[second_register_index], vm->current_opcode);
                }
                else /* comparing int reg with another type register */
                {
                    _NOT_IMPLEMENTED
                }
            }
            else /* comparing an int register to something else. What might that be? */
            {
                _NOT_IMPLEMENTED
            }
        } /* some other type of register is compared against something */
        else
        {
            _NOT_IMPLEMENTED
        }
    }
    else
    if(cmp_first == OPCODE_VAR) /* do we compare a variable to something? */
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(ve)

        if(ve->instantiation->type == OPCODE_INT) /* comparing int variable to something */
        {
            uint8_t cmp_second = vm->content[vm->cc ++]; /* what are we checking against*/
            if(cmp_second == OPCODE_VAR) /* int variable compared to some variable */
            {
                nap_index_t second_var_index = nap_fetch_index(vm);
                struct variable_entry* second_ve = nap_fetch_variable(vm, second_var_index);
                ASSERT_NOT_NULL_VAR(second_ve)

                if(second_ve->instantiation->type == OPCODE_INT) /* comparing int register with an int variable */
                {
                    return nap_vm_set_lbf_to_op_result(vm, *(nap_int_t*)ve->instantiation->value,
                                                *(nap_int_t*)second_ve->instantiation->value,
                                                vm->current_opcode);
                }
                else /* comparing int variable with another kind of variable */
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            if(cmp_second == OPCODE_REG) /* comparing int variable against a register */
            {
                uint8_t second_register_type = vm->content[vm->cc ++]; /* int/string/float...*/

                if(second_register_type == OPCODE_INT) /* comparing int reg with another INT type register */
                {
                    uint8_t second_register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
                    return nap_vm_set_lbf_to_op_result(vm, *(nap_int_t*)ve->instantiation->value,
                                                vm->regi[second_register_index], vm->current_opcode);
                }
                else /* comparing int reg with another type register */
                {
                    _NOT_IMPLEMENTED
                }
            }
        }
        else /* comparing an another type of variable */
        {
            _NOT_IMPLEMENTED
        }
    }
    else
    {
        return NAP_FAILURE;
    }
    return NAP_SUCCESS;
}
