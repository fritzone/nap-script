#include "push.h"
#include "stack.h"

void nap_push(struct nap_vm *vm)
{
    struct stack_entry* se = (struct stack_entry*)(
                                calloc(sizeof(struct stack_entry), 1));
    se->type = (StackEntryType)vm->content[cc ++];

    if(se->type == OPCODE_INT || se->type == OPCODE_STRING) /* or float*/
    {
        uint8_t push_what = content[cc ++];

        if(push_what == OPCODE_VAR)
        {
            uint32_t* p_var_index = (uint32_t*)(content + cc);
            struct variable_entry* ve = metatable[*p_var_index];

            cc += sizeof(uint32_t);

            ve->instantiation = (struct stack_entry*)(calloc(sizeof(struct stack_entry), 1));
            ve->instantiation->type = se->type; /* must match the stack entry */

            if(se->type == OPCODE_INT) /* pushing an integer */
            {
                int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                *temp = 0;
                ve->instantiation->value = temp;
            }
            else
            if(se->type == OPCODE_STRING) /* pushing a string */
            {
                char* temp = (char*)calloc(1, sizeof(char));
                *temp = 0;
                ve->instantiation->value = temp;
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
        uint8_t reg_type = content[cc ++];
        uint8_t reg_idx = content[cc ++];

        if(reg_type == OPCODE_INT) /* pushing an int register */
        {
            int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
            *temp = regi[reg_idx];

            /* setting the value of the stack entry */
            se->value = temp;
        }
        else
        {
            _NOT_IMPLEMENTED
        }
    }
    else
    {
        fprintf(stderr, "not implemented push destination: [0x%x]\n",
                se->type);
        exit(10);
    }

    stack[++ stack_pointer] = se;
}
