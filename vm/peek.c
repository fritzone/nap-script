#include "peek.h"
#include "nbci.h"
#include "opcodes.h"
#include "stack.h"
#include "metatbl.h"

#include <stdlib.h>

void nap_peek(struct nap_vm *vm)
{
    uint8_t peek_type = vm->content[vm->cc ++]; /* int/string/float...*/

    /* we are dealing with an INT type peek */
    if(peek_type == OPCODE_INT)
    {
        uint8_t peek_index_type = vm->content[vm->cc ++]; /* what are we moving in*/
        nap_index_t peek_index = 0;
        uint8_t peek_target = 0;

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
            struct variable_entry* ve = vm->metatable[var_index];

            /* there is no instantioation at this point for the var */
            ve->instantiation = (struct stack_entry*)(
                        calloc(sizeof(struct stack_entry), 1));
            ve->instantiation->type = (StackEntryType)peek_type;
            if(peek_type == OPCODE_INT)
            {
                int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                *temp = *(int64_t*)vm->stack[vm->stack_pointer - peek_index]->value; /* STACK VALUE FROM peek_index */
                ve->instantiation->value = temp;
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
    else
    {
        _NOT_IMPLEMENTED
    }

}
