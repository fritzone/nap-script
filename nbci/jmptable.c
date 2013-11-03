#include "jmptable.h"
#include "nbci.h"

#include <stdlib.h>

/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
void read_jumptable(struct nap_vm* vm, FILE* fp)
{
    size_t count = 0;
    size_t jmpc = 0;

    fseek(fp, vm->jumptable_location + 4, SEEK_SET); /* skip the ".str" */
    fread(&count, sizeof(uint32_t), 1, fp);
    if(count == 0)
    {
        return;
    }
    vm->jumptable_size = count;
    vm->jumptable = (struct jumptable_entry**) calloc(vm->jumptable_size + 1,
                                              sizeof(struct jumptable_entry**));
    for(;;)
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
		if(! new_jmpentry)
		{
			fprintf(stderr, "out of memory when creating a jmpentry\n");
			exit(1);
		}
        new_jmpentry->location = index;
        vm->jumptable[ jmpc++ ] = new_jmpentry;
    }
}
