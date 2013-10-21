#include "strtable.h"
#include "nbci.h"

#include <stdlib.h>

struct strtable_entry** stringtable = NULL;  /* the stringtable itself */
uint64_t strt_size = 0;                 /* the size of the stringtable */

/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
void read_stringtable(FILE* fp, uint64_t stringtable_location)
{
    uint32_t count = 0;
    fseek(fp, stringtable_location + 4, SEEK_SET); /* skip the ".str" */
    fread(&count, sizeof(uint32_t), 1, fp);
    if(count == 0)
    {
        return;
    }
    strt_size = count;
    stringtable = (struct strtable_entry**) calloc(strt_size + 1,
                                               sizeof(struct strtable_entry**));
    while(1)
    {
        uint32_t index = 0;
        int end_of_file = 0;
        fread(&index, file_bitsize, 1, fp);
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
            str = (char*)calloc(sizeof(char), len + 1);
            fread(str, sizeof(uint8_t), len, fp);
            if(strt_size < index + 1)
            {
                stringtable = (struct strtable_entry**) realloc(stringtable,
                                 sizeof(struct strtable_entry**) * (index + 1));
            }
            new_strentry = (struct strtable_entry*)calloc(1, sizeof(struct strtable_entry));
            new_strentry->index = index;
            new_strentry->string = str;
            new_strentry->len = len;
            stringtable[index] = new_strentry;
        }
    }
}
