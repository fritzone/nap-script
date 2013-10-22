#include "metatbl.h"
#include "nbci.h"

#include <stdlib.h>

/*
 * Read the metatable of the bytecode file. Exits on error.
 */
void read_metatable(struct nap_vm* vm, FILE* fp)
{
    uint64_t meta_location = vm->meta_location;
    uint32_t count = 0;
    fseek(fp, meta_location + 5, SEEK_SET); /* skip the ".meta" */
    fread(&count, sizeof(uint32_t), 1, fp);
    if(count == 0)
    {
        return;
    }
    vm->meta_size = count;
    vm->metatable = (struct variable_entry**) calloc(meta_size + 1,
                                               sizeof(struct variable_entry**));
    while(1)
    {
        uint32_t index = 0;
        int end_of_file = 0;
        fread(&index, file_bitsize, 1, fp);
        end_of_file = feof(fp);
        if(index == 1920234286 || end_of_file) /* ".str" */
        {
            if(end_of_file)
            {
                fprintf(stderr, "wrong content: stringtable not found\n");
                exit(2);
            }
            break;
        }
        else
        {
            uint16_t len = 0;
            char* name = NULL;
            struct variable_entry* new_var = NULL;

            fread(&len, sizeof(uint16_t), 1, fp);
            name = (char*)calloc(sizeof(char), len + 1);
            if(name == NULL)
            {
            }
            fread(name, sizeof(uint8_t), len, fp);
            if(vm->meta_size < index + 1)
            { /* seems there is something wrong with the metatable size*/
                vm->metatable = (struct variable_entry**) realloc(vm->metatable,
                                   sizeof(struct variable_entry**)*(index + 1));
            }
            new_var = (struct variable_entry*)
                                    calloc(1, sizeof(struct variable_entry));
            new_var->index = index;
            new_var->name = name;

            new_var->instantiation = 0;
            vm->metatable[index] = new_var;
        }
    }
}
