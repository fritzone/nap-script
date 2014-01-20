#include "operation.h"
#include "nbci.h"
#include "opcodes.h"
#include "metatbl.h"
#include "stack.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_operation(struct nap_vm* vm)
{
    uint8_t add_target = vm->content[vm->cc ++];   /* where we add (reg, var)*/

    if(add_target == OPCODE_REG) /* we add into a register? */
    {
        uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
            uint8_t add_source = vm->content[vm->cc ++]; /* what are we adding to it*/

            if(add_source == OPCODE_IMMEDIATE) /* immediate value (1,..) added to register */
            {
                nap_int_t imm_operand = nap_read_immediate(vm);
                do_int_operation(vm, &vm->regi[register_index], imm_operand, vm->current_opcode);
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
                do_int_operation(vm, &vm->regi[register_index], *(int64_t*)var->instantiation->value, vm->current_opcode);
            }
            else
            if(add_source == OPCODE_REG) /* adding a register to a register */
            {
                uint8_t second_register_type = vm->content[vm->cc ++]; /* int/string/float...*/
                /* we are dealing with an INT type second register */
                if(second_register_type == OPCODE_INT)
                {
                    uint8_t second_register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
                    do_int_operation(vm, &vm->regi[register_index], vm->regi[second_register_index], vm->current_opcode);
                }
                else
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            {
                _NOT_IMPLEMENTED
            }
        }
        else
        if(register_type == OPCODE_STRING)
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
            uint8_t add_source = vm->content[vm->cc ++]; /* what are we adding to it*/
            if(add_source == OPCODE_REG)
            {
                _NOT_IMPLEMENTED
            }
            else
            if(add_source == OPCODE_VAR)
            {
                nap_index_t var_index = nap_fetch_index(vm);
                struct variable_entry* var = nap_fetch_variable(vm, var_index);
                ASSERT_NOT_NULL_VAR(var)
                CHECK_VARIABLE_INSTANTIATON(var)
                CHECK_VARIABLE_TYPE(var,STACK_ENTRY_STRING)

                /* and moving the value in the regsiter itself */
                do_string_operation(vm, &vm->regs[register_index],
                                    &vm->regslens[register_index],
                                    var->instantiation->value,
                                    var->instantiation->len,
                                    vm->current_opcode);
            }
            else
            {
                _NOT_IMPLEMENTED
            }
        }
        else
        {
            _NOT_IMPLEMENTED
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
        add_source = vm->content[vm->cc ++];

        /* moving a register in a variable? */
        if(add_source == OPCODE_REG)
        {
            uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/

            /* we are dealing with an INT type register */
            if(register_type == OPCODE_INT)
            {
                uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/

                /* perform the operation only if the values are not the same already*/
                if(var->instantiation->value)
                {
                    int64_t* temp = (int64_t*)var->instantiation->value;
                    do_int_operation(vm, temp, vm->regi[register_index], vm->current_opcode);
                }
                else /* allocate the memory for the value */
                { /* this should generate some error, there should be a value before add */
                    int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                    *temp = vm->regi[register_index];
                    var->instantiation->value = temp;
                }
            }
            else
            {
                _NOT_IMPLEMENTED
            }
        }
        else
        {
            _NOT_IMPLEMENTED
        }
    }
    else /* maybe adding (substracting) from an indexed value? */
    {
        _NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}
