#include "strtable.h"
#include "nbci.h"
#include "byte_order.h"

#include <stdlib.h>
#include <string.h>

void interpret_stringtable(struct nap_vm *vm, uint8_t *start_location, uint32_t len)
{
    uint8_t* cloc = start_location + 4; /* skip the .str TODO: chec is this .str?*/
    vm->strt_size = htovm_32(*(uint32_t*)(cloc)); /* the next: count*/
    if(vm->strt_size == 0)
    {
        return;
    }
    cloc += 4;

    vm->stringtable = (struct strtable_entry**) calloc(vm->strt_size + 1,
                                                       sizeof(struct strtable_entry*));

    for(;;)
    {
        uint32_t index = htovm_32(*(uint32_t*)(cloc));
        cloc += 4;
        if(index == htovm_32(1886218798) || cloc > start_location + len) /* .jmp */
        {
            break;
        }
        else
        {
            uint32_t len = htovm_32(*(uint32_t*)(cloc));
            char* str = NULL;
            struct strtable_entry* new_strentry = NULL;
            cloc += 4;
            str = (char*)calloc(sizeof(char), len + 1);
            memcpy(str, cloc, len);
            cloc += len;

            if(vm->strt_size < index + 1)
            {
                vm->stringtable = (struct strtable_entry**) realloc(vm->stringtable,
                                 sizeof(struct strtable_entry**) * (index + 1));
                vm->strt_size = index + 1;
            }
            new_strentry = (struct strtable_entry*)calloc(1, sizeof(struct strtable_entry));
            new_strentry->index = index;
            new_strentry->string = str;
            new_strentry->len = len;
            vm->stringtable[index] = new_strentry;
        }
    }
}
