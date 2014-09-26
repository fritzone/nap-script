#include "intr_1.h"
#include "nbci.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>
#include <stdio.h>

uint16_t intr_1(struct nap_vm *vm)
{
    /* peek the last value from stack. It contains the number of parameters that
     * were passed in. Should be an int */
    int cur_stack_peeker = 0;
    nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
    NAP_NN_ASSERT(vm, temp);
    *temp = *(nap_int_t*)vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value;

    /* start and fetch the elements */
    cur_stack_peeker = *temp;
    while(cur_stack_peeker > 0)
    {

        switch(vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->type)
        {
            case STACK_ENTRY_INT:
                fprintf(stdout, "%"PRINT_d"",  *(nap_int_t*)vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value);
                break;
            case STACK_ENTRY_REAL:
                break;
            case STACK_ENTRY_STRING:
                {
                /* the string is a Unicode string, convert it to system representation*/
                size_t dest_len = vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->len, real_len = 0;
                //printf("LEN:%d\n", vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->len);
                char* t = convert_string_from_bytecode_file(vm, vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value,
                        vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->len * CC_MUL, dest_len, &real_len);
                /* Now see if this is a format specifier or not */
                if( t && t[0] =='%')
                {
fprintf(stdout,"%s",t);
                }
                else
                {
                    if(t)
                    fprintf(stdout,"%s",t);
                }
                if(t) free(t);

                else
                puts("NULL t");
                break;
                }
            default:
                
                return NAP_FAILURE;
        };
        cur_stack_peeker --;
    }
    free(temp);
    return NAP_SUCCESS;
}
