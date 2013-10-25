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
        uint32_t peek_index = 0;
        uint8_t peek_target = 0;

        if(peek_index_type == OPCODE_IMMEDIATE) /* immediate value (1,..) */
        {
            uint8_t imm_size = vm->content[vm->cc ++];
            /* and now write the number avm->ccording to the size */
            if(imm_size == OPCODE_BYTE)
            {
                int8_t* immediate = (int8_t*)(vm->content + vm->cc);
                peek_index = *immediate;
                vm->cc ++;
            }
            else
            if(imm_size == OPCODE_SHORT)
            {
                int16_t* immediate = (int16_t*)(vm->content + vm->cc);
                peek_index = *immediate;
                vm->cc += 2;
            }
            else
            if(imm_size == OPCODE_LONG)
            {
                int32_t* immediate = (int32_t*)(vm->content + vm->cc);
                peek_index = *immediate;
                vm->cc += 4;
            }
            else
            {
                printf("invalid immediate size: 0x%x", imm_size);
                exit(14);
            }
        }
        else
        {
            _NOT_IMPLEMENTED
        }

        /* now we know the peek index, see into what are we peeking */
        peek_target = vm->content[vm->cc ++];
        if(peek_target == OPCODE_VAR)
        {
            uint32_t* p_var_index = (uint32_t*)(vm->content + vm->cc);
            struct variable_entry* ve = vm->metatable[*p_var_index];
            vm->cc += sizeof(uint32_t);

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
