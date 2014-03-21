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

    NAP_NN_ASSERT(vm, vm->stack);

    if(vm->stack_pointer == 0) /* no more entries on the stack. Return error */
    {
        return NAP_FAILURE;
    }

    pop_what = vm->content[vm->cc ++];
    if(pop_what == OPCODE_REG)
    {
        uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/

            if(vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
            {
                /*obviously popping something when nothing was returned
                 from a misbehvaing function. Set the register to 0 */
                vm->regi[register_index] = 0;
                /* Do not touch the vm->stack for now, a clrsn might come later*/
            }
            else
            {
                /* check if they have the same type */
                if(vm->stack[vm->stack_pointer]->type == STACK_ENTRY_INT)
                {   /* int on the stack */
                    vm->regi[register_index] = *(nap_int_t*)vm->stack[vm->stack_pointer]->value;
                }
                else
                if(vm->stack[vm->stack_pointer]->type == STACK_ENTRY_BYTE)
                {   /* byte on the stack, convert it to nap_int */
                    vm->regi[register_index] = (nap_int_t)(*(nap_byte_t*)vm->stack[vm->stack_pointer]->value);
                }
                else /* default value */
                {
                    vm->regi[register_index] = 0;
                }
                NAP_MEM_FREE(vm->stack[vm->stack_pointer]->value);
                NAP_MEM_FREE(vm->stack[vm->stack_pointer]);

                vm->stack_pointer --;
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

        if(vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
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
                    if(vm->stack[vm->stack_pointer]->type == OPCODE_INT)
                    {
                        *(nap_int_t*)ve->instantiation->value = *(nap_int_t*)vm->stack[vm->stack_pointer]->value;
                    }
                    else
                    {
                        NAP_NOT_IMPLEMENTED
                    }
                }
                else /* allocate the memory for the value */
                {
                    nap_int_t* temp = NULL;
                    if(vm->stack[vm->stack_pointer]->type == OPCODE_INT)
                    {
                        temp = NAP_MEM_ALLOC(1, nap_int_t);
                        *temp  = *(int64_t*)vm->stack[vm->stack_pointer]->value;
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
                if(vm->stack[vm->stack_pointer]->type == OPCODE_STRING)
                { /* and there is a string on the stack */
                    char* temp = NULL;
                    if(ve->instantiation->value)
                    {
                        NAP_MEM_FREE(ve->instantiation->value);
                    }
                    NAP_STRING_ALLOC(vm, temp, vm->stack[vm->stack_pointer]->len);
                    NAP_STRING_COPY(temp, vm->stack[vm->stack_pointer]->value, vm->stack[vm->stack_pointer]->len );

                    ve->instantiation->value = temp;
                    ve->instantiation->len = vm->stack[vm->stack_pointer]->len;
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

            NAP_MEM_FREE(vm->stack[vm->stack_pointer]->value);
            NAP_MEM_FREE(vm->stack[vm->stack_pointer]);

            /* and the stack will decrease */
            vm->stack_pointer --;
        }
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}
