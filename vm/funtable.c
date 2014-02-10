#include "funtable.h"
#include "nap_consts.h"
#include "nbci.h"
#include "byte_order.h"

#include <stdlib.h>
#include <string.h>

int interpret_funtable(struct nap_vm *vm, uint8_t *start_location, uint32_t len)
{
    uint8_t* cloc = start_location + 4; /* skip the .fun TODO :check if it is .fun */
    size_t func = 0; /* counts the inserted functions */

    /* number of functions */
    size_t count = htovm_32(*(uint32_t*)(cloc));
    if(count == 0)
    {
        return NAP_SUCCESS;
    }
    vm->funtable_entries = count;
    vm->funtable = (struct funtable_entry**) calloc(vm->funtable_entries + 1,
                                              sizeof(struct funtable_entry*));
    if(vm->funtable == NULL)
    {
        vm->funtable_entries = 0;
        return NAP_FAILURE;
    }


    cloc += 4;
    for(;;)
    {
        uint16_t fun_name_len = 0;
        char* fun_name = NULL;
        uint8_t* pars;
        struct funtable_entry* entry = (struct funtable_entry*)calloc(1,
                                                 sizeof(struct funtable_entry));
        if(entry == NULL)
        {
            return NAP_FAILURE;
        }


        if( (cloc + 4) > (vm->content + len) || func == count + 1)
        {
            return NAP_SUCCESS;
        }

        /* the index of this fun in the jumptable */
        entry->jmptable_index = htovm_32(*(uint32_t*)(cloc));
        cloc += 4;

        /* the length of the name of the function */
        fun_name_len =  htovm_16(*(uint16_t*)(cloc));
        cloc += 2;

        /* read the name */
        fun_name = (char*)(calloc(fun_name_len + 1, sizeof(char)));
        if(fun_name == NULL)
        {
            return NAP_FAILURE;
        }
        memcpy(fun_name, cloc, fun_name_len);
        cloc += fun_name_len;
        entry->function_name = fun_name;

        /* read the number of parameters*/
        entry->parameter_count = *cloc;
        cloc ++;

        /* read the parameters if any*/
        if(entry->parameter_count > 0)
        {
            pars = (uint8_t*)calloc(entry->parameter_count, sizeof(uint8_t));
            memcpy(pars, cloc, entry->parameter_count);
            entry->parameter_types = pars;
        }
        vm->funtable[func ++] = entry;

    }
    return NAP_SUCCESS;
}
