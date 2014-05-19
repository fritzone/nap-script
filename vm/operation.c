#include "operation.h"
#include "nbci.h"
#include "opcodes.h"
#include "metatbl.h"
#include "stack.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief Perform the given operation to be found in the opcode, stores the
 *        result in target
 *
 * @param vm - the virtual machine
 * @param target - the target of the operation, and the first operand
 * @param operand - the operand with which we perform
 * @param opcode - the operation we perform
 *
 * @throws a system error if the operation is division and the operand is zero
 */
int do_int_operation(struct nap_vm* vm, nap_int_t* target, nap_int_t operand,
                      uint8_t opcode)
{
    if(opcode == OPCODE_ADD)
    {
        *target += operand;
    }
    else
    if(opcode == OPCODE_SUB)
    {
        *target -= operand;
    }
    else
    if(opcode == OPCODE_DIV)
    {
        if(operand == 0)
        {
            nap_set_error(vm, ERR_VM_0023);
            return NAP_FAILURE;
        }
        else
        {
            *target /= operand;
        }
    }
    else
    if(opcode == OPCODE_MUL)
    {
        *target *= operand;
    }
    else
    if(opcode == OPCODE_MOD)
    {
        if(operand == 0)
        {
            nap_set_error(vm, ERR_VM_0023);
            return NAP_FAILURE;
        }
        else
        {
            *target %= operand;
        }
    }
    else
    if(opcode == OPCODE_SHL)
    {
        *target <<= operand;
    }
    else
    if(opcode == OPCODE_SHR)
    {
        *target >>= operand;
    }
    else
    if(opcode == OPCODE_AND)
    {
        *target &= operand;
    }
    else
    if(opcode == OPCODE_OR)
    {
        *target |= operand;
    }
    else
    if(opcode == OPCODE_XOR)
    {
        *target ^= operand;
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}

/**
 * @brief do_string_operation performs a string operation.
 *
 * @param vm the VM in which we perform the operation
 * @param target the target which will be populated with the result of the
 *               operation. Will be freed
 * @param len The length of the target will be updated. Initially it contains
 *            the original length
 * @param operand The second operand
 * @param operand_len The length of the second operand
 * @param opcode The actual operation
 * @return NAP_SUCCESS if the operation succeeded, NAP_FAILURE otherwise.
 *         Internal error of VM is updated in second case.
 */
static int do_string_operation(struct nap_vm *vm, nap_string_t *target,
                               size_t *len, nap_string_t operand,
                               size_t operand_len, uint8_t opcode)
{
    if(opcode == OPCODE_ADD) /* add two strings, result will be in target */
    {
        size_t final_len = *len * CC_MUL;
        nap_string_t temp = NULL;

        NAP_STRING_ALLOC(vm, temp, (*len + operand_len));
        NAP_STRING_COPY(temp, *target, *len);
        NAP_STRING_COPY(temp + final_len, operand, operand_len);

        *len += operand_len;
        NAP_MEM_FREE(*target);
        *target = temp;

        return NAP_SUCCESS;
    }

    return nap_vm_set_error_description(vm, "Invalid string operation");
}

int nap_operation(struct nap_vm* vm)
{
    uint8_t add_target = vm->content[nap_step_ip(vm)];   /* where we add (reg, var)*/

    if(add_target == OPCODE_REG) /* we add into a register? */
    {
        uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)
        {
            uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            uint8_t add_source = vm->content[nap_step_ip(vm)]; /* what are we adding to it*/

            if(add_source == OPCODE_IMMEDIATE_INT) /* immediate value (1,..) added to register */
            {
                int success = 0;
                nap_int_t imm_operand = nap_read_immediate_int(vm, &success);
                if(success == NAP_FAILURE)
                {
                    return NAP_FAILURE;
                }
                
                return do_int_operation(vm, &vm->cec->regi[register_index],
                                        imm_operand, vm->cec->current_opcode);
            }
            else
            if(add_source == OPCODE_VAR) /* adding a variable to the reg*/
            {
                nap_index_t var_index = nap_fetch_index(vm);
                struct variable_entry* var = nap_fetch_variable(vm, var_index);
                ASSERT_NOT_NULL_VAR(var)
                CHECK_VARIABLE_INSTANTIATON(var)
                CHECK_VARIABLE_TYPE(var,STACK_ENTRY_INT)

                /* and moving the value in the regsiter itself */
                return do_int_operation(vm, &vm->cec->regi[register_index],
                                        *(int64_t*)var->instantiation->value,
                                        vm->cec->current_opcode);
            }
            else
            if(add_source == OPCODE_REG) /* adding a register to a register */
            {
                uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
                /* we are dealing with an INT type second register */
                if(second_register_type == OPCODE_INT)
                {
                    uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
                    return do_int_operation(vm, &vm->cec->regi[register_index],
                                            nap_regi(vm, second_register_index),
                                            vm->cec->current_opcode);
                }
                else
                {
                    NAP_NOT_IMPLEMENTED
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED
            }
        }
        else
        if(register_type == OPCODE_STRING)
        {
            uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            uint8_t add_source = vm->content[nap_step_ip(vm)]; /* what are we adding to it*/
            if(add_source == OPCODE_VAR) /* add reg string (0), xyz */
            {
                nap_index_t var_index = nap_fetch_index(vm);
                struct variable_entry* var = nap_fetch_variable(vm, var_index);
                ASSERT_NOT_NULL_VAR(var)
                CHECK_VARIABLE_INSTANTIATON(var)
                CHECK_VARIABLE_TYPE(var,STACK_ENTRY_STRING)

                /* and moving the value in the regsiter itself */
                do_string_operation(vm,
                                    &(nap_regs(vm, register_index)->s),
                                    &(nap_regs(vm, register_index)->l),
                                    var->instantiation->value,
                                    var->instantiation->len,
                                    vm->cec->current_opcode);
            }
            else
            if(add_source == OPCODE_STRING) /* add reg string (0), "B" */
            {
                /* fetch the index of the string */
                nap_index_t string_index = nap_fetch_index(vm);

                do_string_operation(vm,
                                    &(nap_regs(vm, register_index)->s),
                                    &(nap_regs(vm, register_index)->l),
                                    vm->stringtable[string_index]->string,
                                    vm->stringtable[string_index]->len,
                                    vm->cec->current_opcode);

            }
            else
            if(add_source == OPCODE_REG) /* add reg string (0), reg string (1) */
            {
                /* fetch the index of the string */
                uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* string, int, etc... */
                if(second_register_type == OPCODE_STRING)
                {
                    uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
                    do_string_operation(vm,
                                        &(nap_regs(vm, register_index)->s),
                                        &(nap_regs(vm, register_index)->l),
                                        nap_regs(vm, second_register_index)->s,
                                        nap_regs(vm, second_register_index)->l,
                                        vm->cec->current_opcode);
                }
                else
                {
                    NAP_NOT_IMPLEMENTED
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED
            }
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(add_target == OPCODE_VAR) /* we move into a variable */
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* var = nap_fetch_variable(vm, var_index);
        uint8_t add_source = 0;

        ASSERT_NOT_NULL_VAR(var)
        CHECK_VARIABLE_INSTANTIATON(var)

        /* and now let's see what we move in the variable */
        add_source = vm->content[nap_step_ip(vm)];

        /* moving a register in a variable? */
        if(add_source == OPCODE_REG)
        {
            uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

            /* we are dealing with an INT type register */
            if(register_type == OPCODE_INT)
            {
                uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/

                /* perform the operation only if the values are not the same already*/
                if(var->instantiation->value)
                {
                    nap_int_t* temp = (nap_int_t*)var->instantiation->value;
                    return do_int_operation(vm, temp,
                                            nap_regi(vm, register_index),
                                            vm->cec->current_opcode);
                }
                else /* allocate the memory for the value */
                { /* this should generate some error, there should be a value before add */
                    nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
                    *temp = nap_regi(vm, register_index);
                    var->instantiation->value = temp;
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED
            }
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else /* maybe adding (substracting) from an indexed value? */
    {
        NAP_NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}
