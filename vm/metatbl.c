#include "metatbl.h"
#include "nbci.h"
#include "nbci_impl.h"
#include "byte_order.h"
#include "nap_consts.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Read the metatable of the bytecode file.
 */
int interpret_metatable(struct nap_vm* vm, uint8_t* start_location, uint32_t len)
{
    uint8_t* cloc = 0; 
    
    /* see if this is actually .meta or something else */
    if(*(start_location) != '.' 
        || *(start_location + 1) != 'm'
        || *(start_location + 2) != 'e'
        || *(start_location + 3) != 't'
        || *(start_location + 4) != 'a')
    {
        return NAP_FAILURE;
    }
    
    cloc = start_location + 5; /* skip the .meta */
    vm->meta_size = htovm_32(*(uint32_t*)(cloc)); /* the number of meta entries */
    cloc += 4;

    if(vm->meta_size == 0)
    {
        return NAP_SUCCESS;
    }

    vm->metatable = NAP_MEM_ALLOC(vm->meta_size + 1, struct variable_entry*);
    NAP_NN_ASSERT(vm, vm->metatable);

    for(;;)
    {
        uint32_t index = htovm_32(*(uint32_t*)(cloc));
        cloc += 4;

        if(index == htovm_32(1920234286) || cloc > start_location + len) /* ".str" */
        {
            return NAP_SUCCESS;
        }
        else
        {
            char* name = NULL;
            struct variable_entry* new_var = NULL;
            uint16_t len = 0;                 /* the length of the name */
            uint8_t type = *cloc;             /* the type of the variable */

            cloc ++;

            /* length of the variable name */
            len = htovm_16(*(uint16_t*)(cloc));
            cloc += 2;

            if(len != 0)
            {
                name = NAP_MEM_ALLOC(len + 1, char);
                NAP_NN_ASSERT(vm, name);

                memcpy(name, cloc, len);
                cloc += len;
            }

            if(vm->meta_size < index + 1)
            { /* seems there is something wrong with the metatable size*/
                struct variable_entry** tmp = (struct variable_entry**) realloc(vm->metatable,
                                               sizeof(struct variable_entry**) * (index + 1));
                NAP_NN_ASSERT(vm, tmp);

                vm->metatable = tmp;
            }
            new_var = NAP_MEM_ALLOC(1, struct variable_entry);
            NAP_NN_ASSERT(vm, new_var);

            new_var->index = index;
            new_var->name = name;
            new_var->type = type;

            new_var->instantiation = 0;
            vm->metatable[index] = new_var;
        }
    }

    return NAP_SUCCESS;
}
