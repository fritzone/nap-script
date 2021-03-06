#include "funtable.h"
#include "nap_consts.h"
#include "nbci.h"
#include "nbci_impl.h"
#include "byte_order.h"

#include <stdlib.h>
#include <string.h>

int interpret_funtable(struct nap_vm *vm, uint8_t *start_location, uint32_t len)
{
    uint8_t* cloc = 0;
    size_t func = 0; /* counts the inserted functions */
    
    /* check is this .jmp?*/
    /* TODO: make sure we actually have 3 more characters */
    if(*(start_location) != '.' 
        || *(start_location + 1) != 'f'
        || *(start_location + 2) != 'u'
        || *(start_location + 3) != 'n')
    {
        return NAP_FAILURE;
    }

    cloc = start_location + 4; /* skip the .fun */
    
    /* number of functions */
    vm->funtable_size = htovm_32(*(uint32_t*)(cloc));
    
    if(vm->funtable_size == 0)
    {
        return NAP_SUCCESS;
    }
    
    vm->funtable = NAP_MEM_ALLOC(vm->funtable_size + 1, struct funtable_entry*);
    NAP_NN_ASSERT(vm, vm->funtable);

    cloc += 4;
    for(;;)
    {
        uint16_t fun_name_len = 0;
        char* fun_name = NULL;
        uint8_t* pars = NULL;

        struct funtable_entry* entry = NULL;

        if( (cloc + 4) > (vm->content + len) || func == vm->funtable_size + 1)
        {
            return NAP_SUCCESS;
        }

        entry = NAP_MEM_ALLOC(1, struct funtable_entry);
        NAP_NN_ASSERT(vm, entry);

        /* the index of this fun in the jumptable */
        entry->jmptable_index = htovm_32(*(uint32_t*)(cloc));
        cloc += 4;

        /* the length of the name of the function */
        fun_name_len =  htovm_16(*(uint16_t*)(cloc));
        cloc += 2;

        /* read the name */
        fun_name = NAP_MEM_ALLOC(fun_name_len + 1, char);
        NAP_NN_ASSERT(vm, fun_name);

        memcpy(fun_name, cloc, fun_name_len);
        cloc += fun_name_len;
        entry->function_name = fun_name;

        /* read the return type */
        entry->return_type= *cloc;
        cloc ++;

        /* read the number of parameters*/
        entry->parameter_count = *cloc;
        cloc ++;

        /* read the parameters if any*/
        if(entry->parameter_count > 0)
        {
            pars = NAP_MEM_ALLOC(entry->parameter_count, uint8_t);
            NAP_NN_ASSERT(vm, pars);

            memcpy(pars, cloc, entry->parameter_count);
            entry->parameter_types = pars;
        }
        vm->funtable[func ++] = entry;

    }
}
