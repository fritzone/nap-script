#include "push.h"
#include "stack.h"
#include "opcodes.h"
#include "metatbl.h"

#include <stdlib.h>
#include <stdint.h>

void nap_push(struct nap_vm *vm)
{
    struct stack_entry* se = (struct stack_entry*)(
                                calloc(sizeof(struct stack_entry), 1));
    se->type = (StackEntryType)vm->content[vm->cc ++];

    if(se->type == OPCODE_INT || se->type == OPCODE_STRING) /* or float*/
    {
        uint8_t push_what = vm->content[vm->cc ++];

        if(push_what == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            struct variable_entry* ve = vm->metatable[var_index];

            /* TODO: is ve NULL? */

            if(ve->instantiation == NULL)
            {
                ve->instantiation = (struct stack_entry*)(calloc(sizeof(struct stack_entry), 1));
            }
            ve->instantiation->type = se->type; /* must match the stack entry */

            if(se->type == OPCODE_INT) /* pushing an integer */
            {
                if(ve->instantiation->value) /* to not to create another value if we have it*/
                {                            /* such as variable declaration in a loop */
                    *(int64_t*)ve->instantiation->value = 0;
                }
                else
                {
                    ve->instantiation->value = calloc(1, sizeof(int64_t));
                    *(int64_t*)ve->instantiation->value = 0;
                }
            }
            else
            if(se->type == OPCODE_STRING) /* pushing a string */
            {
                if(ve->instantiation->value)
                {
                    *(char*)ve->instantiation->value = 0;
                }
                else
                {
                    ve->instantiation->value = (char*)calloc(1, sizeof(char));
                    *(char*)ve->instantiation->value = 0;
                }
            }
            else
            {
                _NOT_IMPLEMENTED
            }

            /* setting the value of the stack entry */
            se->value = ve;
        }
        else
        {
            fprintf(stderr, "unknown push [push int 0x%x]", push_what);
            exit(66);
        }
    }
    else
    if(se->type == OPCODE_REG) /* pushing a register */
    {
        uint8_t reg_type = vm->content[vm->cc ++];
        uint8_t reg_idx = vm->content[vm->cc ++];

        if(reg_type == OPCODE_INT) /* pushing an int register */
        {
            nap_int_t* temp = (nap_int_t*)calloc(1, sizeof(nap_int_t));
            *temp = vm->regi[reg_idx];

            /* setting the value of the stack entry */
            se->value = temp;
        }
        else
        {
            _NOT_IMPLEMENTED
        }
    }
    else
    if(se->type == OPCODE_IMMEDIATE)
    {
        nap_int_t nr = nap_read_immediate(vm);
        nap_int_t* temp = (nap_int_t*)calloc(1, sizeof(nap_int_t));
        *temp = nr;

        /* setting the value of the stack entry */
        se->value = temp;
    }
    else
    if(se->type == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = vm->metatable[var_index];
        if(ve->instantiation == NULL)
        {
            fprintf(stderr, "invalid push of an undeclared variable [%s]\n", ve->name);
            exit(9);
        }
        se->type = ve->instantiation->type; /* must match the stack entry */

        /* setting the value of the stack entry, nothing else is required here */
        se->value = ve;

    }
    else
    {
        _NOT_IMPLEMENTED
    }

    vm->stack[++ vm->stack_pointer] = se;
}
