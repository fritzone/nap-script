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

    if(peek_index_type == OPCODE_IMMEDIATE_INT) /* immediate value (1,..) */
    {
        int success = 0;
        peek_index = (nap_index_t)nap_read_immediate_int(vm, &success);
        if(success == NAP_FAILURE)
        {
            return NAP_FAILURE;
        }
    }
    else /* nothing else can be peeked from the stack */
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
        }

        /* create a new instantiation */
        ve->instantiation = NAP_MEM_ALLOC(1, struct stack_entry);
        NAP_NN_ASSERT(vm, ve->instantiation);

        ve->instantiation->type = (StackEntryType)peek_type;

        struct stack_entry* se = vm->cec->stack[nap_sp(vm) - peek_index];
        if(se->holds_array)
        {
            size_t size_to_copy = ((struct variable_entry*)se->value)->data_size * ((struct variable_entry*)se->value)->instantiation->len;
            char* tmp = NAP_MEM_ALLOC(size_to_copy, char);
            memcpy(tmp, ((struct variable_entry*)se->value)->instantiation->value, size_to_copy);
            ve->instantiation->value = tmp;
            ve->instantiation->len = ((struct variable_entry*)se->value)->instantiation->len;
            ve->data_size = ((struct variable_entry*)se->value)->data_size;
            ve->dimension_count = ((struct variable_entry*)se->value)->dimension_count;
            memcpy(ve->dimensions, ((struct variable_entry*)se->value)->dimensions, sizeof(ve->dimensions));
        }
        else
        {
            if(peek_type == OPCODE_INT) /* we are dealing with an INT type peek */
            {   /* peek int: assumes that on the stack there is a nap_int_t in the value of the stack_entry at the given index*/
                nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
                NAP_NN_ASSERT(vm, temp);
                *temp = *(nap_int_t*)se->value; /* STACK VALUE FROM peek_index */
                ve->instantiation->value = temp;
            }
            else
            if(peek_type == OPCODE_BYTE) /* we are dealing with a BYTE type peek */
            {   /* peek byte: assumes that on the stack there is a nap_byte_t in the value of the stack_entry at the given index*/
                nap_byte_t* temp = NAP_MEM_ALLOC(1, nap_byte_t);
                NAP_NN_ASSERT(vm, temp);
                *temp = *(nap_byte_t*)se->value; /* STACK VALUE FROM peek_index */
                ve->instantiation->value = temp;
            }
            else
            if(peek_type == OPCODE_REAL) /* we are dealing with a real type peek */
            {   /* peek real: assumes that on the stack there is a nap_real_t in the value of the stack_entry at the given index*/
                nap_real_t* temp = NAP_MEM_ALLOC(1, nap_real_t);
                NAP_NN_ASSERT(vm, temp);
                *temp = *(nap_real_t*)se->value; /* STACK VALUE FROM peek_index */
                ve->instantiation->value = temp;
            }
            else
            if(peek_type == OPCODE_STRING)
            {   /* assumes there is a string on the stack, at the given index'd stack_entry */
                char* temp = NULL;
                size_t len = se->len * CC_MUL;
                temp = NAP_MEM_ALLOC(len, char);
                NAP_NN_ASSERT(vm, temp);
                memcpy(temp, se->value, len);
                ve->instantiation->value = temp;
                ve->instantiation->len = se->len; /* real length, not UTF32 length*/
            }
            else
            {
                NAP_NOT_IMPLEMENTED /* no more types */
            }
        }
    }
    else
    {
        NAP_NOT_IMPLEMENTED /* can only peek in a variable */
    }

    return NAP_SUCCESS;
}
