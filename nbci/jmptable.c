#include "jmptable.h"
#include "nbci.h"

#include <stdlib.h>

/* variables for the jumptable */
struct jumptable_entry** jumptable = NULL;   /* the jumptable itself */
uint64_t jumptable_size = 0;              /* the size of the jumptable */
uint32_t jmpc = 0;               /* counts the jumptable entries on loading*/
/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
void read_jumptable(FILE* fp, uint64_t stringtable_location)
{
    uint32_t count = 0;
    fseek(fp, stringtable_location + 4, SEEK_SET); /* skip the ".str" */
    fread(&count, sizeof(uint32_t), 1, fp);
    if(count == 0)
    {
        return;
    }
    jumptable_size = count;
    jumptable = (struct jumptable_entry**) calloc(jumptable_size + 1,
                                              sizeof(struct jumptable_entry**));
    while(1)
    {
        uint32_t index = 0;
        int end_of_file = 0;
        struct jumptable_entry* new_jmpentry = NULL;
        fread(&index, sizeof(uint32_t), 1, fp);
        end_of_file = feof(fp);
        if(end_of_file)
        {
            return;
        }


        new_jmpentry = (struct jumptable_entry*)calloc(1, sizeof(struct jumptable_entry));
        new_jmpentry->location = index;
        jumptable[ jmpc++ ] = new_jmpentry;
    }
}
