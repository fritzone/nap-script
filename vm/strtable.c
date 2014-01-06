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

    vm->stringtable = (struct strtable_entry**) calloc(vm->strt_size + 1,
                                                       sizeof(struct strtable_entry*));
    if(vm->stringtable == NULL)
    {
        vm->strt_size = 0;
        return NAP_FAILURE;
    }

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
            char* converted_str = NULL;
            struct strtable_entry* new_strentry = NULL;

            cloc += 4;
            str = (char*)calloc(sizeof(char), len + 1);
            if(str == NULL)
            {
                return NAP_FAILURE;
            }
            memcpy(str, cloc, len * 4); /* UTF-32BE encoding */
            cloc += len * 4;

            if(vm->strt_size < index + 1)
            {
                struct strtable_entry** tmp = (struct strtable_entry**)
                        realloc(vm->stringtable,
                                sizeof(struct strtable_entry**) * (index + 1));
                if(tmp == NULL)
                {
                    return NAP_FAILURE;
                }
                vm->stringtable = tmp;
                vm->strt_size = index + 1;
            }
            new_strentry = (struct strtable_entry*)calloc(1,
                                                 sizeof(struct strtable_entry));
            if(new_strentry == NULL)
            {
                return NAP_FAILURE;
            }

            converted_str = convert_string_from_bytecode_file(str, len * 4, len);

            new_strentry->index = index;
            new_strentry->string = converted_str;
            new_strentry->len = len;
            vm->stringtable[index] = new_strentry;

            if(converted_str != str)
            {
                free(str);
            }
        }
    }

    return NAP_SUCCESS;
}
