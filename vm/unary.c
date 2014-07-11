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
    uint8_t op_what = vm->content[nap_step_ip(vm)]; /* variable, register*/
    if(op_what == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* var = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(var);
        CHECK_VARIABLE_INSTANTIATON(var);

        if(var->instantiation->type == OPCODE_INT)
        {
            if(vm->cec->current_opcode == OPCODE_BCOM)
            {
                (*(nap_int_t*)var->instantiation->value) = ~(*(nap_int_t*)var->instantiation->value);
            }
            else
            if(vm->cec->current_opcode == OPCODE_NOT)
            {
                (*(nap_int_t*)var->instantiation->value) = !(*(nap_int_t*)var->instantiation->value);
            }
            else
            if(vm->cec->current_opcode == OPCODE_NEG)
            {
                (*(nap_int_t*)var->instantiation->value) = (*(nap_int_t*)var->instantiation->value);
            }
            else
            if(vm->cec->current_opcode == OPCODE_POS)
            {
                (*(nap_int_t*)var->instantiation->value) = llabs((*(nap_int_t*)var->instantiation->value));
            }
            else
            {
                NAP_NOT_IMPLEMENTED  /* No other type of unary operation */
            }
        }
        else
        if(var->instantiation->type == OPCODE_BYTE)
        {
            if(vm->cec->current_opcode == OPCODE_BCOM)
            {
                (*(nap_byte_t*)var->instantiation->value) = ~(*(nap_byte_t*)var->instantiation->value);
            }
            else
            if(vm->cec->current_opcode == OPCODE_NOT)
            {
                (*(nap_byte_t*)var->instantiation->value) = !(*(nap_byte_t*)var->instantiation->value);
            }
            else
            if(vm->cec->current_opcode == OPCODE_NEG)
            {
                (*(nap_byte_t*)var->instantiation->value) = (*(nap_byte_t*)var->instantiation->value);
            }
            else
            if(vm->cec->current_opcode == OPCODE_POS) /* TODO: does this make sense? A byte is always positive. */
            {
                (*(nap_byte_t*)var->instantiation->value) = (nap_byte_t)abs((int)(*(nap_byte_t*)var->instantiation->value));
            }
            else /* No other type of unary operation */
            {
                NAP_NOT_IMPLEMENTED
            }
        }
        else /* Unary operations on other data types are not allowed */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(op_what == OPCODE_REG)
    {
        uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(register_type == OPCODE_INT) /* unary operation on an int register */
        {
            uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            if(vm->cec->current_opcode == OPCODE_BCOM)
            {
                nap_set_regi(vm, register_index, ~ nap_regi(vm, register_index));
            }
            else
            if(vm->cec->current_opcode == OPCODE_NOT)
            {
                nap_set_regi(vm, register_index, ! nap_regi(vm, register_index));
            }
            else
            if(vm->cec->current_opcode == OPCODE_NEG)
            {
                nap_set_regi(vm, register_index, - nap_regi(vm, register_index));
            }
            else
            if(vm->cec->current_opcode == OPCODE_POS)
            {
                nap_set_regi(vm, register_index, llabs(nap_regi(vm, register_index)));
            }
            else
            {
                NAP_NOT_IMPLEMENTED
            }
        }
        else
        if(register_type == OPCODE_BYTE) /* unary operation on a byte register */
        {
            uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            if(vm->cec->current_opcode == OPCODE_BCOM)
            {
                nap_set_regb(vm, register_index, ~ nap_regb(vm, register_index));
            }
            else
            if(vm->cec->current_opcode == OPCODE_NOT)
            {
                nap_set_regb(vm, register_index, ! nap_regb(vm, register_index));
            }
            else
            if(vm->cec->current_opcode == OPCODE_NEG)
            {
                nap_set_regb(vm, register_index, - nap_regb(vm, register_index));
            }
            else
            if(vm->cec->current_opcode == OPCODE_POS) /* TODO: does this make sense? */
            {
                nap_set_regb(vm, register_index, (nap_byte_t)abs(nap_regb(vm, register_index)));
            }
            else
            {
                NAP_NOT_IMPLEMENTED
            }
        }
        else /* Unary operations on other types are not allowed */
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

