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
    nap_int_t cur_stack_peeker = 0;
    nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
    char* last_format_specifier = NULL;
    StackEntryType last_type = STACK_ENTRY_INVALID;

    FILE* dest = stdout;

    NAP_NN_ASSERT(vm, temp);
    *temp = *(nap_int_t*)vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value;

    /* start and fetch the elements */
    cur_stack_peeker = *temp;
    while(cur_stack_peeker > 0)
    {
        last_type = vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->type;
        switch(last_type)
        {
            case STACK_ENTRY_INT: /* This uses the last format specifier.*/
                if(last_format_specifier == NULL)
                {
                    fprintf(dest, "%"PRINT_d"",  *(nap_int_t*)vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value);
                }
                else
                {
                    fprintf(dest, last_format_specifier, *(nap_int_t*)vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value);
                    free(last_format_specifier);
                    last_format_specifier = NULL;
                }
                break;
            case STACK_ENTRY_BYTE: /* This uses the last format specifier.*/
                if(last_format_specifier == NULL)
                {
                    fprintf(dest, "%i",  *(nap_byte_t*)vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value);
                }
                else
                {
                    fprintf(dest, last_format_specifier, *(nap_int_t*)vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value);
                    free(last_format_specifier);
                    last_format_specifier = NULL;
                }
                break;
            case STACK_ENTRY_REAL: /* This uses the last format specifier */
                if(last_format_specifier == NULL)
                {
                    fprintf(dest, "%Lf",  *(nap_real_t*)vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value);
                }
                else
                {
                    fprintf(dest, last_format_specifier, *(nap_real_t*)vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value);
                    free(last_format_specifier);
                    last_format_specifier = NULL;
                }
                break;
                break;
            case STACK_ENTRY_STRING:
                {
                /* the string is a Unicode string, convert it to system representation*/
                size_t dest_len = vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->len, real_len = 0;
                char* t = convert_string_from_bytecode_file(vm,
                                                            vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->value,
                                                            vm->cec->stack[nap_sp(vm) - cur_stack_peeker]->len * CC_MUL, 
                                                            dest_len, &real_len);
                /* Now see if this is a format specifier or not */
                if( t && t[0] == '%' )
                {
                    if(last_format_specifier != NULL)
                    {
                        fprintf(dest, "%s", last_format_specifier);
                        free(last_format_specifier);
                    }
                    last_format_specifier = strdup(t);
                }
                else
                {
                    if(last_format_specifier != NULL)
                    {
                        fprintf(dest, "%s", last_format_specifier);
                        free(last_format_specifier);
                        last_format_specifier = NULL;
                    }

                    if(t)
                    {
                        size_t sctr = 0;
                        size_t slen = strlen(t);
                        for(; sctr<slen; sctr++ )
                        {
                            if(t[sctr] == '\\')
                            {
                                if(sctr+1 < slen)
                                {
                                    switch(t[sctr + 1])
                                    {
                                    case 'n':
                                        fprintf(dest, "\n");
                                        break;
                                    case 't':
                                        fprintf(dest, "\t");
                                        break;
                                    case 'r':
                                        fprintf(dest, "\r");
                                        break;
                                    case 'f':
                                        fprintf(dest, "\f");
                                        break;
                                    case 'b':
                                        fprintf(dest, "\b");
                                        break;
                                    case 'v':
                                        fprintf(dest, "\v");
                                        break;
                                    case '\'':
                                        fprintf(dest, "'");
                                        break;
                                    case '"':
                                        fprintf(dest, "\"");
                                        break;
                                    case '\\':
                                        fprintf(dest, "\\");
                                        break;
                                    default:
                                        fprintf(dest, "\\%c", t[sctr+1]);
                                    }
                                    sctr ++;
                                }
                            }
                            else
                            {
                                fprintf(dest,"%c",t[sctr]);
                            }
                        }

                    }
                    else
                    {
                        fprintf(dest, "(null)");
                    }
                }

                if(t)
                {
                    free(t);
                }
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
