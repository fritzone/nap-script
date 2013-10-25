#include "pop.h"
#include "nbci.h"
#include "opcodes.h"
#include "stack.h"

#include <stdlib.h>

void nap_pop(struct nap_vm* vm)
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
                free(vm->stack[vm->stack_pointer]->value);
                free(vm->stack[vm->stack_pointer]);

                vm->stack_pointer --;
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
