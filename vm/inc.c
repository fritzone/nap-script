#include "inc.h"
#include "nbci.h"
#include "metatbl.h"
#include "opcodes.h"
#include "stack.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_inc(struct nap_vm* vm)
{
    uint8_t inc_what = vm->content[nap_step_ip(vm)]; /* variable, register, indexed*/
    if(inc_what == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);

        ASSERT_NOT_NULL_VAR(ve)
        CHECK_VARIABLE_INSTANTIATON(ve)

        if(ve->instantiation->type == OPCODE_INT)
        {
            (*(nap_int_t*)ve->instantiation->value) ++;
        }
        else
        if(ve->instantiation->type == OPCODE_BYTE)
        {
            (*(nap_byte_t*)ve->instantiation->value) ++;
        }
        else
        if(ve->instantiation->type == OPCODE_REAL)
        {
            (*(nap_real_t*)ve->instantiation->value) ++;
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(inc_what == OPCODE_REG)
    {
        uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)  /* to handle inc reg int x */
        {
            uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            vm->cec->regi[register_index] ++;
        }
        else
        if(register_type == OPCODE_IDX)
        {
            uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            vm->cec->regidx[register_index] ++;
        }
        else
        if(register_type == OPCODE_BYTE)
        {
            uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            vm->cec->regb[register_index] ++;
        }
        else
        if(register_type == OPCODE_REAL)
        {
            uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            vm->cec->regr[register_index] ++;
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(inc_what == OPCODE_CCIDX) /* increasing an indexed*/
    {
        uint8_t ccidx_target = vm->content[nap_step_ip(vm)];  /* should be a variable */
        if(ccidx_target == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            struct variable_entry* var = nap_fetch_variable(vm, var_index);
            uint8_t ctr_used_index_regs = 0;
            char* error = NULL;
            int64_t idx = -1;

            ASSERT_NOT_NULL_VAR(var)
            CHECK_VARIABLE_INSTANTIATON(var)

            /* now should come the index reg counter of vm->ccidx,
               a simple byte since there are max 256 indexes */
            ctr_used_index_regs = vm->content[nap_step_ip(vm)];
            idx = deliver_flat_index(vm, var, ctr_used_index_regs, &error);

            if(idx < 0) /* error? */
            {
                /* do not use the set_error here, the string was allocated
                   copying it would be a memory leak*/
                vm->error_description = error;
                return NAP_FAILURE;
            }

            /* increasing an indexed integer */
            if(var->instantiation->type == OPCODE_INT)
            {
                /* casting it to nap_int_t is ok, since
                 * var->instantiation->type == OPCODE_INT so we have
                 * allocated nap_int_t variable in the grow */
                ((nap_int_t*)var->instantiation->value)[idx] ++;
            }
            else
            /* increasing an indexed byte */
            if(var->instantiation->type == OPCODE_BYTE)
            {
                ((nap_byte_t*)var->instantiation->value)[idx] ++;
            }
            else
            /* increasing an indexed real */
            if(var->instantiation->type == OPCODE_REAL)
            {
                ((nap_real_t*)var->instantiation->value)[idx] ++;
            }
            else
            {
                NAP_NOT_IMPLEMENTED /* No other type of indexables */
            }
        }
        else /* what else can be indexed? */
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
