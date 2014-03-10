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
    uint8_t peek_type = vm->content[vm->cc ++]; /* int/string/float...*/

    uint8_t peek_index_type = vm->content[vm->cc ++]; /* what are we moving in*/
    nap_index_t peek_index = 0; /* the index that's peeked */
    uint8_t peek_target = 0;    /* this normally is OPCODE_VAR for compiled code*/

    if(peek_index_type == OPCODE_IMMEDIATE) /* immediate value (1,..) */
    {
        peek_index = (nap_index_t)nap_read_immediate(vm);
    }
    else
    {
        _NOT_IMPLEMENTED
    }

    /* now we know the peek index, see into what are we peeking */
    peek_target = vm->content[vm->cc ++];
    if(peek_target == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(ve)

        /* there supposed to be no instantiation at this point for the var */
        if(ve->instantiation)
        {
            if(ve->instantiation->value)
            {
                MEM_FREE(ve->instantiation->value);
            }
            MEM_FREE(ve->instantiation);
        }

        /* create a new instantiation */
        ve->instantiation = (struct stack_entry*)(
                    calloc(sizeof(struct stack_entry), 1));
        ve->instantiation->type = (StackEntryType)peek_type;

        if(peek_type == OPCODE_INT) /* we are dealing with an INT type peek */
        {   /* peek int: assumes that on the stack there is a nap_int_t in the value of the stack_entry at the given index*/
            nap_int_t* temp = (nap_int_t*)calloc(1, sizeof(nap_int_t));
            *temp = *(nap_int_t*)vm->stack[vm->stack_pointer - peek_index]->value; /* STACK VALUE FROM peek_index */
            ve->instantiation->value = temp;
        }
        else
        if(peek_type == OPCODE_STRING)
        {   /* assumes there is a string on the stack, at the given index'd stack_entry */
            char* temp = NULL;
            struct stack_entry* se = vm->stack[vm->stack_pointer - peek_index];
            size_t len = se->len * CC_MUL;
            temp = (char*)(calloc(len,  sizeof(char)));
            memcpy(temp, se->value, len);
            ve->instantiation->value = temp;
            ve->instantiation->len = se->len; /* real length, not UTF32 length*/
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

    return NAP_SUCCESS;
}
