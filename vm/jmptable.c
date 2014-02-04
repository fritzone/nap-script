#include "jmptable.h"
#include "nbci.h"
#include "byte_order.h"
#include "nap_consts.h"

#include <stdlib.h>
#include <string.h>

int interpret_jumptable(struct nap_vm* vm, uint8_t* start_location, uint32_t len)
{
    uint8_t* cloc = start_location + 4; /* skip the .jmp TODO :check if it is .jmp */
    size_t count = 0;
    size_t jmpc = 0;
    size_t indx_ctr = 0;

    count = htovm_32(*(uint32_t*)(cloc));
    cloc += 4;
    if(count == 0)
    {
        return NAP_SUCCESS;
    }
    vm->jumptable_size = count;
    vm->jumptable = (struct jumptable_entry**) calloc(vm->jumptable_size + 1,
                                              sizeof(struct jumptable_entry*));
    if(vm->jumptable == NULL)
    {
        vm->jumptable_size = 0;
        return NAP_FAILURE;
    }

    for(;;)
    {
        uint32_t index = 0;
        enum label_type type = 0;
        uint16_t loc_name_length = 0;
        char* name = NULL;

        struct jumptable_entry* new_jmpentry = NULL;

        if( (cloc + 4) > (vm->content + len) || ++indx_ctr == count+1)
        {
            return NAP_SUCCESS;
        }

        /* read the index */
        index = htovm_32(*(uint32_t*)(cloc));
        cloc += 4;

        /* the type */
        type = (enum label_type)*cloc;
        cloc ++;

        if(type != JUMP_DESTINATION)
        {
            /* the length */
            loc_name_length = htovm_16(*(uint16_t*)(cloc));
            cloc += 2;

            if(loc_name_length != 0)
            {
                name = (char*)calloc(sizeof(char), loc_name_length + 1);
                if(name == NULL)
                {
                    return NAP_FAILURE;
                }

                memcpy(name, cloc, loc_name_length);
                cloc += loc_name_length;
            }
        }
        new_jmpentry = (struct jumptable_entry*)calloc(1, sizeof(struct jumptable_entry));
        if(new_jmpentry == NULL)
        {
            return NAP_FAILURE;
        }
        new_jmpentry->location = index;
        new_jmpentry->type = type;
        new_jmpentry->label_name = name;

        vm->jumptable[ jmpc++ ] = new_jmpentry;
    }
}
