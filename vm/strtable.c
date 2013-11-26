#include "strtable.h"
#include "nbci.h"
#include "byte_order.h"

#include <stdlib.h>
#include <string.h>

/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
void read_stringtable(struct nap_vm *vm, FILE* fp)
{
    uint32_t count = 0;
    uint32_t strt_size;

    fseek(fp, vm->stringtable_location + 4, SEEK_SET); /* skip the ".str" */
    fread(&count, sizeof(uint32_t), 1, fp);
    count = htovm_32(count);
    if(count == 0)
    {
        return;
    }
    strt_size = count;
    vm->stringtable = (struct strtable_entry**) calloc(strt_size + 1,
                                               sizeof(struct strtable_entry*));
    for(;;) 
    {
        uint32_t index = 0;
        int end_of_file = 0;
        fread(&index, vm->file_bitsize, 1, fp);
        index = htovm_32(index);

        end_of_file = feof(fp);

        if(index == 1886218798 || end_of_file) /* .jmp */
        {
            if(end_of_file)
            {
                fprintf(stderr, "wrong content: jumptable not found\n");
                exit(1);
            }
            break;
        }
        else
        {
            uint32_t len = 0;
            char* str = NULL;
            struct strtable_entry* new_strentry = NULL;
            fread(&len, sizeof(uint32_t), 1, fp);
            len = htovm_32(len);
            str = (char*)calloc(sizeof(char), len + 1);
            fread(str, sizeof(uint8_t), len, fp);
            if(strt_size < index + 1)
            {
                vm->stringtable = (struct strtable_entry**) realloc(vm->stringtable,
                                 sizeof(struct strtable_entry**) * (index + 1));
            }
            new_strentry = (struct strtable_entry*)calloc(1, sizeof(struct strtable_entry));
            new_strentry->index = index;
            new_strentry->string = str;
            new_strentry->len = len;
            vm->stringtable[index] = new_strentry;
        }
    }
}

void interpret_stringtable(struct nap_vm *vm, uint8_t *start_location, uint32_t len)
{
    uint8_t* cloc = start_location + 4; /* skip the .str TODO: chec is this .str?*/
    uint32_t strt_size = htovm_32(*(uint32_t*)(cloc)); /* the next: count*/
    if(strt_size == 0)
    {
        return;
    }
    cloc += 4;

    vm->stringtable = (struct strtable_entry**) calloc(strt_size + 1,
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

            if(strt_size < index + 1)
            {
                vm->stringtable = (struct strtable_entry**) realloc(vm->stringtable,
                                 sizeof(struct strtable_entry**) * (index + 1));
            }
            new_strentry = (struct strtable_entry*)calloc(1, sizeof(struct strtable_entry));
            new_strentry->index = index;
            new_strentry->string = str;
            new_strentry->len = len;
            vm->stringtable[index] = new_strentry;
        }
    }
}
