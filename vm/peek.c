#include "peek.h"
#include "nbci.h"
#include "opcodes.h"
#include "stack.h"
#include "metatbl.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>
#include <string.h>

int nap_peek(struct nap_vm *vm)
{
    uint8_t peek_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

    uint8_t peek_index_type = vm->content[nap_step_ip(vm)]; /* what are we moving in*/
    nap_index_t peek_index = 0; /* the index that's peeked */
    uint8_t peek_target = 0;    /* this normally is OPCODE_VAR for compiled code*/

    if(peek_index_type == OPCODE_IMMEDIATE) /* immediate value (1,..) */
    {
        int success = 0;
        peek_index = (nap_index_t)nap_read_immediate(vm, &success);
        if(success == NAP_FAILURE)
        {
            return NAP_FAILURE;
        }
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }

    /* now we know the peek index, see into what are we peeking */
    peek_target = vm->content[nap_step_ip(vm)];
    if(peek_target == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(ve);

        /* there supposed to be no instantiation at this point for the var */
        if(ve->instantiation)
        {
            if(ve->instantiation->value)
            {
                NAP_MEM_FREE(ve->instantiation->value);
            }
            NAP_MEM_FREE(ve->instantiation);
        }

        /* create a new instantiation */
        ve->instantiation = NAP_MEM_ALLOC(1, struct stack_entry);
        NAP_NN_ASSERT(vm, ve->instantiation);

        ve->instantiation->type = (StackEntryType)peek_type;

        if(peek_type == OPCODE_INT) /* we are dealing with an INT type peek */
        {   /* peek int: assumes that on the stack there is a nap_int_t in the value of the stack_entry at the given index*/
            nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
            NAP_NN_ASSERT(vm, temp);
            *temp = *(nap_int_t*)vm->cec->stack[nap_sp(vm) - peek_index]->value; /* STACK VALUE FROM peek_index */
            ve->instantiation->value = temp;
        }
        else
        if(peek_type == OPCODE_STRING)
        {   /* assumes there is a string on the stack, at the given index'd stack_entry */
            char* temp = NULL;
            struct stack_entry* se = vm->cec->stack[nap_sp(vm) - peek_index];
            size_t len = se->len * CC_MUL;
            temp = NAP_MEM_ALLOC(len, char);
            NAP_NN_ASSERT(vm, temp);
            memcpy(temp, se->value, len);
            ve->instantiation->value = temp;
            ve->instantiation->len = se->len; /* real length, not UTF32 length*/
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
