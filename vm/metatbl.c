#include "metatbl.h"
#include "nbci.h"
#include "byte_order.h"

#include <string.h>
#include <stdlib.h>

/*
 * Read the metatable of the bytecode file. Exits on error.
 */
void interpret_metatable(struct nap_vm* vm, uint8_t* start_location, uint32_t len)
{
    uint8_t* cloc = start_location + 5; /* skip the .meta TODO :check if it is .meta */
    uint32_t count = htovm_32(*(uint32_t*)(cloc));
    cloc += 4;

    if(count == 0)
    {
        return;
    }

    vm->meta_size = count;
    vm->metatable = (struct variable_entry**) calloc(vm->meta_size + 1,
                                               sizeof(struct variable_entry*));
    for(;;)
    {
        uint32_t index = htovm_32(*(uint32_t*)(cloc));
        cloc += 4;

        if(index == htovm_32(1920234286) || cloc > start_location + len) /* ".str" */
        {
            break;
        }
        else
        {
            char* name = NULL;
            struct variable_entry* new_var = NULL;

            /* the type of the variable */
            uint8_t type = *cloc;
            cloc ++;

            /* length of the variable name */
            uint16_t len = htovm_16(*(uint16_t*)(cloc));
            cloc += 2;

            if(len != 0)
            {
                name = (char*)calloc(sizeof(char), len + 1);
                if(name == NULL)
                {
                    fprintf(stderr, "cannot allocate a metatable entry\n");
                    exit(EXIT_FAILURE);
                }

                memcpy(name, cloc, len);
                cloc += len;
            }

            fprintf(stderr, "add: %d, %s\n", index, name);

            if(vm->meta_size < index + 1)
            { /* seems there is something wrong with the metatable size*/
                struct variable_entry** tmp = (struct variable_entry**) realloc(vm->metatable,
                                               sizeof(struct variable_entry**) * (index + 1));
                if(tmp == NULL)
                {
                    fprintf(stderr, "cannot reallocate the metatable\n");
                    exit(EXIT_FAILURE);
                }

                vm->metatable = tmp;
            }
            new_var = (struct variable_entry*)
                                    calloc(1, sizeof(struct variable_entry));
            new_var->index = index;
            new_var->name = name;
            new_var->type = type;

            new_var->instantiation = 0;
            vm->metatable[index] = new_var;
        }
    }
}
