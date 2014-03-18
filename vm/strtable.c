#include "strtable.h"
#include "nbci.h"
#include "byte_order.h"
#include "nap_consts.h"
#include "nbci_impl.h"

#include <stdlib.h>
#include <string.h>

int interpret_stringtable(struct nap_vm *vm, uint8_t *start_location, uint32_t len)
{
    uint8_t* cloc = start_location + 4; /* skip the .str TODO: chec is this .str?*/
    vm->strt_size = htovm_32(*(uint32_t*)(cloc)); /* the next: count*/
    if(vm->strt_size == 0)
    {
        return NAP_SUCCESS;
    }
    cloc += 4;

    vm->stringtable = NAP_MEM_ALLOC(vm->strt_size + 1, struct strtable_entry*);
    NAP_NN_ASSERT(vm, vm->stringtable);

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

            NAP_STRING_ALLOC(vm, str, len);
            NAP_STRING_COPY(str, cloc, len);

            cloc += len * CC_MUL;

            if(vm->strt_size < index + 1)
            {
                struct strtable_entry** tmp = (struct strtable_entry**)
                        realloc(vm->stringtable,
                                sizeof(struct strtable_entry**) * (index + 1));
                NAP_NN_ASSERT(vm, tmp);

                vm->stringtable = tmp;
                vm->strt_size = index + 1;
            }
            new_strentry = NAP_MEM_ALLOC(1, struct strtable_entry);
            NAP_NN_ASSERT(vm, new_strentry);

            new_strentry->index = index;
            new_strentry->string = str;
            new_strentry->len = len;
            vm->stringtable[index] = new_strentry;
        }
    }

    return NAP_SUCCESS;
}
