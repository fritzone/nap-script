#include "call_intern.h"
#include "nbci.h"
#include "jmptable.h"
#include "nbci_impl.h"
#include "nap_consts.h"
#include "opcodes.h"
#include "metatbl.h"

#include <stdlib.h>

/* This method implements internal calls such as grow, create, etc ...
 * Noone is supposed to call this method
 */
int nap_call_intern(struct nap_vm* vm)
{
    uint8_t call_what = vm->content[vm->cc ++];

    if(call_what == OPCODE_GROW)
    {
        uint8_t grow_what = vm->content[vm->cc ++];
        if(grow_what == OPCODE_VAR) /* we are adding a dimension to a variable */
        {
            nap_index_t var_index = nap_fetch_index(vm);
            struct variable_entry* ve = nap_fetch_variable(vm, var_index);
			uint8_t grow_target_type = 0;
            ASSERT_NOT_NULL_VAR(ve)

            grow_target_type = vm->content[vm->cc ++];
            if(grow_target_type == OPCODE_REG) /* grow with the value of a register */
            {
                uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/
                uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/

                /* we are dealing with an INT type register */
                if(register_type == OPCODE_INT)
                {
                    nap_int_t grow_value = vm->regi[register_index];
                    nap_int_t new_size = 1;
                    uint8_t dim_counter = 0;

                    if(grow_value == 0)
                    {
                        char* s = (char*)calloc(256, sizeof(char));
                        SNPRINTF(s, 256,
                                "[ERR-INT] Cannot grow with 0 index [%s].",
                                 ve->name);
                        vm->error_description = s;
                        return NAP_FAILURE;
                    }
                    /* recalculate how much memory we need*/
                    while(dim_counter < 254 && ve->dimensions[dim_counter])
                    {
                        new_size *= ve->dimensions[dim_counter ++];
                    }
                    new_size *= grow_value;

                    /* now allocate the memory for this*/
                    if(ve->instantiation)
                    {
                        uint8_t data_size = sizeof(char);
						void* t = NULL;
                        /* calculate the size of the var depending on its needs*/

                        /* an array of ints */
                        if(ve->instantiation->type == STACK_ENTRY_INT)
                        {
                            data_size = sizeof(nap_int_t);
                        }
                        else
                        /* an array of doubles */
                        if(ve->instantiation->type == STACK_ENTRY_REAL)
                        {
                            data_size = sizeof(nap_real_t);
                        }
                        /* an array of strings */
                        else
                        {
                            _NOT_IMPLEMENTED
                        }

                        ve->data_size = data_size;
                        t = realloc(ve->instantiation->value,
                                          new_size * data_size);
                        if(!t)
                        {
                            char* s = (char*)calloc(256, sizeof(char));
                            SNPRINTF(s, 256,
                                    "[ERR-INT] Cannot grow [%s]. Not enough memory.",
                                     ve->name);
                            vm->error_description = s;
                            return NAP_FAILURE;
                        }

                        /* copying it over */
                        ve->instantiation->value = t;
                        ve->instantiation->len = new_size;
                    }
                    else
                    {
                        char* s = (char*)calloc(256, sizeof(char));
                        SNPRINTF(s, 256,
                                "[ERR-INT-2] Cannot grow [%s]. Not pushed yet.",
                                 ve->name);
                        vm->error_description = s;
                        return NAP_FAILURE;
                    }

                    /* and update the variable itself */
                    ve->dimensions[dim_counter] = grow_value;
                    ve->dimension_count ++;
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
    else
    {
        return NAP_FAILURE;
    }

    return NAP_SUCCESS;
}

