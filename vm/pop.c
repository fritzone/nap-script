#include "pop.h"
#include "nbci.h"
#include "opcodes.h"
#include "stack.h"
#include "metatbl.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_pop(struct nap_vm* vm)
{
    uint8_t pop_what = vm->content[vm->cc ++];
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
                /* Do not touch the vm->stack for now.*/
            }
            else
            {
                /* check if they have the same type */
                if(vm->stack[vm->stack_pointer]->type == STACK_ENTRY_INT)
                {
                    vm->regi[register_index] = *(int64_t*)vm->stack[vm->stack_pointer]->value;
                }
                else
                if(vm->stack[vm->stack_pointer]->type == OPCODE_REG)
                {
                    vm->regi[register_index] = *(int64_t*)vm->stack[vm->stack_pointer]->value;
                }
                else /* default value */
                {
                    vm->regi[register_index] = 0;
                }
                MEM_FREE(vm->stack[vm->stack_pointer]->value);
                MEM_FREE(vm->stack[vm->stack_pointer]);

                vm->stack_pointer --;
            }
        }
        else
        {
            _NOT_IMPLEMENTED
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
                    nap_int_t* temp = (nap_int_t*)calloc(1, sizeof(nap_int_t));
                    *temp = 0;
                    ve->instantiation->value = temp;
                }
            }
            else
            {
                _NOT_IMPLEMENTED
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
                    if(vm->stack[vm->stack_pointer]->type == OPCODE_IMMEDIATE)
                    {
                        *(nap_int_t*)ve->instantiation->value = *(nap_int_t*)vm->stack[vm->stack_pointer]->value;
                    }
                    else
                    if(vm->stack[vm->stack_pointer]->type == OPCODE_INT)
                    {
                        struct variable_entry* ve_src = vm->stack[vm->stack_pointer]->value;
                        *(nap_int_t*)ve->instantiation->value = *(nap_int_t*)ve_src->instantiation->value;
                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }
                }
                else /* allocate the memory for the value */
                {
                    nap_int_t* temp = (nap_int_t*)calloc(1, sizeof(nap_int_t));
                    if(vm->stack[vm->stack_pointer]->type == OPCODE_IMMEDIATE)
                    {
                        *temp  = *(int64_t*)vm->stack[vm->stack_pointer]->value;
                    }
                    else
                    if(vm->stack[vm->stack_pointer]->type == OPCODE_INT)
                    {
                        struct variable_entry* ve_src = vm->stack[vm->stack_pointer]->value;
                        *temp  = *(int64_t*)ve_src->instantiation->value;
                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }
                    ve->instantiation->value = temp;
                }
            }
            if(ve->instantiation->type == STACK_ENTRY_STRING)
            {
                _NOT_IMPLEMENTED
            }
            else
            {
                _NOT_IMPLEMENTED
            }

            if(vm->stack[vm->stack_pointer]->type == OPCODE_IMMEDIATE
                    || vm->stack[vm->stack_pointer]->len != 0)
            { /* immediate values are being allocated by the push, also string values */
                MEM_FREE(vm->stack[vm->stack_pointer]->value);
            }

            MEM_FREE(vm->stack[vm->stack_pointer]);

            /* and the stack will decrease */
            vm->stack_pointer --;
        }
    }
    else
    {
        _NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}
