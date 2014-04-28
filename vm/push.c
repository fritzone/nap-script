#include "push.h"
#include "stack.h"
#include "opcodes.h"
#include "metatbl.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int nap_push(struct nap_vm *vm)
{
    struct stack_entry* se = NAP_MEM_ALLOC(1, struct stack_entry);
    NAP_NN_ASSERT(vm, se);

    se->type = (StackEntryType)vm->content[nap_step_ip(vm)]; /* whether we push
                  int/string/real/byte for creating a variable or just pushing
                  a variable without the int/string/byte/etc ... or a string
                  or a number, ... This just saves a variable*/

    /* see for push int global.XX, push string global.YY ... ie: declaring a variable */
    if(   se->type == STACK_ENTRY_INT
       || se->type == STACK_ENTRY_BYTE
       || se->type == STACK_ENTRY_STRING) /* STACK_ENTRY_STRING is the same as OPCODE_STRING */
        /* or real */
    {
        uint8_t push_what = vm->content[nap_step_ip(vm)];

        if(push_what == OPCODE_VAR) /* This is basically defining a variable on the stack */
        {                           /* such as push int globa.variable_name */
            nap_index_t var_index = nap_fetch_index(vm);
            struct variable_entry* ve = nap_fetch_variable(vm, var_index);
            ASSERT_NOT_NULL_VAR(ve);

            ve->instantiation = NAP_MEM_ALLOC(1, struct stack_entry);
            NAP_NN_ASSERT(vm, ve->instantiation);

            ve->instantiation->type = se->type; /* must match the stack entry */

            if(se->type == STACK_ENTRY_INT) /* pushing an integer */
            {
                ve->instantiation->value = NAP_MEM_ALLOC(1, nap_int_t);
                NAP_NN_ASSERT(vm, ve->instantiation->value);
                *(nap_int_t*)ve->instantiation->value = 0;
            }
            else
            if(se->type == STACK_ENTRY_BYTE) /* pushing a byte */
            {
                ve->instantiation->value = NAP_MEM_ALLOC(1, nap_byte_t);
                NAP_NN_ASSERT(vm, ve->instantiation->value);
                *(nap_byte_t*)ve->instantiation->value = 0;
            }
            else
            if(se->type == STACK_ENTRY_STRING) /* pushing a string */
            {
                ve->instantiation->value = NAP_MEM_ALLOC(1, char);
                NAP_NN_ASSERT(vm, ve->instantiation->value);
                *(char*)ve->instantiation->value = 0;
            }
            else
            {
                NAP_NOT_IMPLEMENTED
            }

            /* setting the value of the stack entry to be the 0 coming from the
               creation of the variable, so there is a 0 on the stack */
            se->value = ve->instantiation->value;

            /* this will be a variable definition, set the correct field */
            se->var_def = ve;

        }
        else
        {
            /* is this push "abcde"? <- the compiler generates: push string index */
            if(se->type == OPCODE_STRING) /* pushing an immediate string on the stack! */
            {
                nap_index_t str_index = 0; /* the index of the string */
                size_t len = 0;            /* the length of the string */
                char* temp = 0;            /* a temporary variable */

                nap_move_ip(vm, 1, BACKWARD); /* stepping one back, right now we point to first byte in index */

                str_index = nap_fetch_index(vm);
                if(str_index >= vm->strt_size) /* this index does not exist */
                {
                    return NAP_FAILURE;
                }
                
                len = vm->stringtable[str_index]->len * CC_MUL; /* UTF32 */
                temp = NAP_MEM_ALLOC(len,  char);
                NAP_NN_ASSERT(vm, temp) ;

                memcpy(temp, vm->stringtable[str_index] , len);
                se->value = temp; /* the stack_entry->value will be the string itself */
                se->len = vm->stringtable[str_index]->len; /* the stack_entry->len will be the
                                                    real length of the string, not
                                                    the length of the UTF32 thing */
            }
            else
            {
                char s[64];
                SNPRINTF(s, 64, "unknown push [0x%x] at %"PRINT_u" (%"PRINT_x")", 
                         push_what, nap_ip(vm), nap_ip(vm));
                return nap_vm_set_error_description(vm, s);
            }
        }
    }
    else
    if(se->type == OPCODE_REG) /* pushing a register. */
    {
        uint8_t register_index = 0; /* the index of the register */
        se->type = vm->content[nap_step_ip(vm)]; /* update the stack entry type */
        register_index = vm->content[nap_step_ip(vm)];

        if(se->type == OPCODE_INT) /* pushing an int register */
        {
            nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
            NAP_NN_ASSERT(vm, temp);

            *temp = vm->regi[register_index];

            /* setting the value of the stack entry */
            se->value = temp;
        }
        else
        if(se->type == OPCODE_BYTE) /* pushing a byte register */
        {
            nap_byte_t* temp = NAP_MEM_ALLOC(1, nap_byte_t);
            NAP_NN_ASSERT(vm, temp);

            *temp = nap_regb(vm, register_index);

            /* setting the value of the stack entry */
            se->value = temp;
        }
        else
        if(se->type == OPCODE_STRING) /* pushing a string register */
        {
            size_t len = vm->regslens[register_index] * CC_MUL; /* UTF32*/
            char* temp = NAP_MEM_ALLOC(len, char);
            NAP_NN_ASSERT(vm, temp);

            memcpy(temp, vm->regs[register_index], len);
            se->value = temp; /* the stack_entry->value will be the string itself */
            se->len = vm->regslens[register_index]; /* the stack_entry->len will be the
                                                real length of the string, not
                                                the length of the UTF32 thing */
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(se->type == OPCODE_IMMEDIATE)
    {
        /* immediate values (23, 42) are pushed as ints */
        int success = 0;
        nap_int_t* temp = NULL;
        nap_int_t nr = nap_read_immediate(vm, &success);
        
        if(!success)
        {
            return NAP_FAILURE;
        }
        
        temp = NAP_MEM_ALLOC(1, nap_int_t);
        NAP_NN_ASSERT(vm, temp);
        *temp = nr;

        /* setting the value of the stack entry and indicating it is immediate */
        se->value = temp;
        se->type = STACK_ENTRY_INT;
    }
    else
    if(se->type == OPCODE_VAR) /* such as: push global.varname*/
    {
        /* this push does not declare the variable merely creates a stack entry
           with its value*/
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(ve)
        CHECK_VARIABLE_INSTANTIATON(ve)

        se->type = ve->instantiation->type; /* force to match the stack entry */

        if(se->type == STACK_ENTRY_INT) /* pushing an int variable */
        {                               /* STACK_ENTRY_INT = OPCODE_INT */
            nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
            NAP_NN_ASSERT(vm, temp);

            *temp = *(nap_int_t*)ve->instantiation->value;

            /* setting the value of the stack entry */
            se->value = temp;
        }
        else
        if(se->type == STACK_ENTRY_BYTE) /* pushing a byte variable */
        {
            nap_byte_t* temp = NAP_MEM_ALLOC(1, nap_byte_t);
            NAP_NN_ASSERT(vm, temp);
            
            *temp = *(nap_byte_t*)ve->instantiation->value;
            
            /* setting the value of the stack entry */
            se->value = temp;
        }
        else
        if(se->type == STACK_ENTRY_STRING)
        {
            size_t len = ve->instantiation->len * CC_MUL; /* UTF32*/
            char* temp = NAP_MEM_ALLOC(len, char);
            NAP_NN_ASSERT(vm, temp);
            
            memcpy(temp, ve->instantiation->value, len);
            se->value = temp; /* the stack_entry->value will be the string itself */
            se->len = ve->instantiation->len; /* the stack_entry->len will be the
                                                  real length of the string, not
                                                  the length of the UTF32 thing */        
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

    vm->stack[++ vm->stack_pointer] = se;
    return NAP_SUCCESS;
}
