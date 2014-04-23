#include "unary.h"
#include "nbci.h"
#include "metatbl.h"
#include "opcodes.h"
#include "stack.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_unary(struct nap_vm* vm)
{
    /* vm-> current_opcode has the opcode */
    uint8_t op_what = vm->content[vm->cc ++]; /* variable, register*/
    if(op_what == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* var = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(var);
        CHECK_VARIABLE_INSTANTIATON(var);

        if(var->instantiation->type == OPCODE_INT)
        {
            if(vm->current_opcode == OPCODE_BCOM)
            {
                (*(nap_int_t*)var->instantiation->value) = ~(*(nap_int_t*)var->instantiation->value);
            }
            else
            if(vm->current_opcode == OPCODE_NOT)
            {
                (*(nap_int_t*)var->instantiation->value) = !(*(nap_int_t*)var->instantiation->value);
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
    if(op_what == OPCODE_REG)
    {
        uint8_t regt = vm->content[vm->cc ++]; /* int/string/float...*/
        if(regt == OPCODE_INT) /* unary operation on an int register */
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
            if(vm->current_opcode == OPCODE_BCOM)
            {
                vm->regi[register_index] = ~vm->regi[register_index];
            }
            else
            if(vm->current_opcode == OPCODE_NOT)
            {
                vm->regi[register_index] = !vm->regi[register_index];
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
    return NAP_SUCCESS;
}

