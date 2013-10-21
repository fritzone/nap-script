/******************************************************************************/
/*                             Other functions section                        */
/******************************************************************************/

#include "nbci.h"

void nap_vm_set_lbf_to_op_result(struct nap_vm* vm, int64_t reg, int64_t immediate, uint8_t opcode)
{
    if(opcode == OPCODE_EQ)
    {
        vm->lbf = (reg == immediate);
    }
    else
    if(opcode == OPCODE_NEQ)
    {
        vm->lbf = (reg != immediate);
    }
    else
    if(opcode == OPCODE_LT)
    {
        vm->lbf = (reg <  immediate);
    }
    else
    if(opcode == OPCODE_GT)
    {
        vm->lbf = (reg >  immediate);
    }
    else
    if(opcode == OPCODE_LTE)
    {
        vm->lbf = (reg <= immediate);
    }
    else
    if(opcode == OPCODE_GTE)
    {
        vm->lbf = (reg >= immediate);
    }
    else
    {
        _NOT_IMPLEMENTED
    }
}

void do_operation(struct nap_vm* vm, int64_t* target, int64_t operand, uint8_t opcode)
{
    if(opcode == OPCODE_ADD)
    {
        *target += operand;
    }
    else
    if(opcode == OPCODE_SUB)
    {
        *target -= operand;
    }
    else
    if(opcode == OPCODE_DIV)
    {
        *target /= operand;
    }
    else
    if(opcode == OPCODE_MUL)
    {
        *target *= operand;
    }
    else
    if(opcode == OPCODE_MOD)
    {
        *target %= operand;
    }
    else
    {
        _NOT_IMPLEMENTED
    }
}

/**
 * Cleans the allocated memory
 */
void cleanup(struct nap_vm* vm)
{
    uint64_t i;
    int64_t tempst;
    int64_t tempjmi;
    dump();
    for(i=0; i<meta_size; i++)
    {
        if(metatable[i]->instantiation)
        {
            if(metatable[i]->instantiation->value)
            {
                free(metatable[i]->instantiation->value);
            }
            free(metatable[i]->instantiation);
        }
    }
    for(i=0; i<meta_size; i++)
    {
        free(metatable[i]->name);
        free(metatable[i]);
    }
    free(metatable);

    /* free the allocated stack */
    for(tempst = stack_pointer; tempst > -1; tempst --)
    {
        if(stack[tempst]->type == OPCODE_INT) /* or float/string */
        {
            /* this wa already freed in the metatable */
        }
        else /* register type */
        if(stack[tempst]->type == OPCODE_REG || stack[tempst]->type == STACK_ENTRY_MARKER_NAME)
        {
            free(stack[tempst]->value);
        }

        free(stack[tempst]);
    }
    free(stack);

    /* freeing the jumptable */
    for(tempjmi = jumptable_size - 1; tempjmi >= 0; tempjmi --)
    {
        free(jumptable[tempjmi]);
    }

    free(jumptable);

    /* freeing the content */
    free(vm->content);
}
