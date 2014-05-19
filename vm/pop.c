#include "pop.h"
#include "nbci.h"
#include "opcodes.h"
#include "stack.h"
#include "metatbl.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>
#include <string.h>

int nap_pop(struct nap_vm* vm)
{
    uint8_t pop_what = 0;

    if(nap_sp(vm) == 0) /* no more entries on the stack. Return error */
    {
        return NAP_FAILURE;
    }

    pop_what = vm->content[nap_step_ip(vm)];
    if(pop_what == OPCODE_REG)
    {
        uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        /* we are popping an INT type register */
        if(register_type == OPCODE_INT)
        {
            uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/

            if(vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_MARKER_NAME)
            {
                /*obviously popping something when nothing was returned
                 from a misbehvaing function. Set the register to 0 */
                nap_set_regi(vm, register_index, 0);
                /* Do not touch the vm->cec->stack for now, a clrsn might come later*/
            }
            else
            {
                /* check if they have the same type */
                if(vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_INT)
                {   /* int on the stack */
                    nap_set_regi(vm, register_index, *(nap_int_t*)vm->cec->stack[nap_sp(vm)]->value);
                }
                else
                if(vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_BYTE)
                {   /* byte on the stack, convert it to nap_int */
                    nap_set_regi(vm, register_index, (nap_int_t)(*(nap_byte_t*)vm->cec->stack[nap_sp(vm)]->value));
                }
                else /* default value */
                {
                    nap_set_regi(vm, register_index, 0);
                }
                NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]->value);
                NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]);

                vm->cec->stack_pointer --;
            }
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(pop_what == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(ve)
        CHECK_VARIABLE_INSTANTIATON(ve)

        if(vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_MARKER_NAME)
        {
            /*obviously popping something when nothing was pushed properly.
             * Set the variable to 0 */
            if(ve->instantiation->type == STACK_ENTRY_INT)
            {
                if(ve->instantiation->value)
                {
                    *(nap_int_t*)ve->instantiation->value = 0;
                }
                else /* allocate the memory for the value */
                {
                    nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
                    NAP_NN_ASSERT(vm, temp);

                    *temp = 0;
                    ve->instantiation->value = temp;
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED
            }
            /* Do not touch the stack for now.*/
        }
        else
        {
            /* popping into an integer variable */
            if(ve->instantiation->type == STACK_ENTRY_INT)
            {
                if(ve->instantiation->value)
                {
                    if(vm->cec->stack[nap_sp(vm)]->type == OPCODE_INT)
                    {
                        *(nap_int_t*)ve->instantiation->value = *(nap_int_t*)vm->cec->stack[nap_sp(vm)]->value;
                    }
                    else
                    {
                        NAP_NOT_IMPLEMENTED
                    }
                }
                else /* allocate the memory for the value */
                {
                    nap_int_t* temp = NULL;
                    if(vm->cec->stack[nap_sp(vm)]->type == OPCODE_INT)
                    {
                        temp = NAP_MEM_ALLOC(1, nap_int_t);
						NAP_NN_ASSERT(vm, temp);
                        *temp  = *(int64_t*)vm->cec->stack[nap_sp(vm)]->value;
                    }
                    else
                    {
                        NAP_NOT_IMPLEMENTED
                    }
                    ve->instantiation->value = temp;
                }
            }
            else
            if(ve->instantiation->type == STACK_ENTRY_STRING) /* popping a string variable */
            {
                if(vm->cec->stack[nap_sp(vm)]->type == OPCODE_STRING)
                { /* and there is a string on the stack */
                    char* temp = NULL;
                    if(ve->instantiation->value)
                    {
                        NAP_MEM_FREE(ve->instantiation->value);
                    }
                    NAP_STRING_ALLOC(vm, temp, vm->cec->stack[nap_sp(vm)]->len);
                    NAP_STRING_COPY(temp, vm->cec->stack[nap_sp(vm)]->value, vm->cec->stack[nap_sp(vm)]->len );

                    ve->instantiation->value = temp;
                    ve->instantiation->len = vm->cec->stack[nap_sp(vm)]->len;
                }
                else /* popping a string when the push was not a string */
                {
                    NAP_NOT_IMPLEMENTED
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED
            }

            NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]->value);
            NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]);

            /* and the stack will decrease */
            vm->cec->stack_pointer --;
        }
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}
