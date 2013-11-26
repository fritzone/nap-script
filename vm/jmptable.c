#include "jmptable.h"
#include "nbci.h"
#include "byte_order.h"

#include <stdlib.h>

/*
 * Read the stringtable of the bytecode file. Exits on failure.
 */
void read_jumptable(struct nap_vm* vm, FILE* fp)
{
    size_t count = 0;
    size_t jmpc = 0;
    size_t read_bytes = 0;

    int err = fseek(fp, vm->jumptable_location + 4, SEEK_SET); /* skip the ".str" */
    if(err == -1)
    {
        fprintf(stderr, "cannot seek in the file to the jumptable location\n");
        nap_vm_cleanup(vm);
        exit(EXIT_FAILURE);
    }
    read_bytes = fread(&count, sizeof(uint32_t), 1, fp);
    if(read_bytes != 1)
    {
        fprintf(stderr, "cannot read the number of jumptable entries");
        nap_vm_cleanup(vm);
        exit(EXIT_FAILURE);
    }
    count = htovm_32(count);
    if(count == 0)
    {
        return;
    }
    vm->jumptable_size = count;
    vm->jumptable = (struct jumptable_entry**) calloc(vm->jumptable_size + 1,
                                              sizeof(struct jumptable_entry*));
    for(;;)
    {
        uint32_t index = 0;
        int end_of_file = 0;
        struct jumptable_entry* new_jmpentry = NULL;
        read_bytes = fread(&index, sizeof(uint32_t), 1, fp);
        if(read_bytes != 1 && jmpc != count)
        {
            fprintf(stderr, "cannot read jumptable entry: truncated file");
            nap_vm_cleanup(vm);
            exit(EXIT_FAILURE);
        }
        end_of_file = feof(fp);
        if(end_of_file)
        {
            return;
        }

        new_jmpentry = (struct jumptable_entry*)calloc(1, sizeof(struct jumptable_entry));
        if(new_jmpentry == NULL)
		{
			fprintf(stderr, "out of memory when creating a jmpentry\n");
			exit(1);
		}
        index = htovm_32(index);
        new_jmpentry->location = index;
        vm->jumptable[ jmpc++ ] = new_jmpentry;
    }
}


void interpret_jumptable(struct nap_vm* vm, uint8_t* start_location, uint32_t len)
{
    uint8_t* cloc = start_location + 4; /* skip the .jmp TODO :check if it is .jmp */
    size_t count = 0;
    size_t jmpc = 0;
    size_t indx_ctr = 0;

    count = htovm_32(*(uint32_t*)(cloc));
    cloc += 4;
    if(count == 0)
    {
        return;
    }
    vm->jumptable_size = count;
    vm->jumptable = (struct jumptable_entry**) calloc(vm->jumptable_size + 1,
                                              sizeof(struct jumptable_entry*));
    for(;;)
    {
        uint32_t index = 0;

        struct jumptable_entry* new_jmpentry = NULL;

        if(cloc+4 > start_location + len || ++indx_ctr == count+1)
        {
            return;
        }

        index = htovm_32(*(uint32_t*)(cloc));
        cloc += 4;

        new_jmpentry = (struct jumptable_entry*)calloc(1, sizeof(struct jumptable_entry));
        if(new_jmpentry == NULL)
        {
            fprintf(stderr, "out of memory when creating a jmpentry\n");
            nap_vm_cleanup(vm);
            exit(EXIT_FAILURE);
        }
        fprintf(stderr, "j: %d\n", index);
        new_jmpentry->location = index;
        vm->jumptable[ jmpc++ ] = new_jmpentry;
    }
}
