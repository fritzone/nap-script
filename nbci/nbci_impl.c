/******************************************************************************/
/*                             Other functions section                        */
/******************************************************************************/

#include "nbci.h"
#include "metatbl.h"
#include "strtable.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void nap_vm_set_lbf_to_op_result(struct nap_vm* vm, int64_t reg, int64_t immediate, uint8_t opcode)
{
    if(opcode == OPCODE_EQ)
    {
        vm->lbf = (reg == immediate);
    }
    else
    if(opcode == OPCODE_NEQ)
    {
        vm->lbf = (reg != immediate);
    }
    else
    if(opcode == OPCODE_LT)
    {
        vm->lbf = (reg <  immediate);
    }
    else
    if(opcode == OPCODE_GT)
    {
        vm->lbf = (reg >  immediate);
    }
    else
    if(opcode == OPCODE_LTE)
    {
        vm->lbf = (reg <= immediate);
    }
    else
    if(opcode == OPCODE_GTE)
    {
        vm->lbf = (reg >= immediate);
    }
    else
    {
        _NOT_IMPLEMENTED
    }
}

void do_operation(struct nap_vm* vm, int64_t* target, int64_t operand, uint8_t opcode)
{
    if(opcode == OPCODE_ADD)
    {
        *target += operand;
    }
    else
    if(opcode == OPCODE_SUB)
    {
        *target -= operand;
    }
    else
    if(opcode == OPCODE_DIV)
    {
        *target /= operand;
    }
    else
    if(opcode == OPCODE_MUL)
    {
        *target *= operand;
    }
    else
    if(opcode == OPCODE_MOD)
    {
        *target %= operand;
    }
    else
    {
        _NOT_IMPLEMENTED
    }
}

/**
 * Cleans the allocated memory
 */
void cleanup(struct nap_vm* vm)
{
    uint64_t i;
    int64_t tempst;
    int64_t tempjmi;
    dump();
    for(i=0; i<meta_size; i++)
    {
        if(metatable[i]->instantiation)
        {
            if(metatable[i]->instantiation->value)
            {
                free(metatable[i]->instantiation->value);
            }
            free(metatable[i]->instantiation);
        }
    }
    for(i=0; i<meta_size; i++)
    {
        free(metatable[i]->name);
        free(metatable[i]);
    }
    free(metatable);

    /* free the allocated stack */
    for(tempst = stack_pointer; tempst > -1; tempst --)
    {
        if(stack[tempst]->type == OPCODE_INT) /* or float/string */
        {
            /* this wa already freed in the metatable */
        }
        else /* register type */
        if(stack[tempst]->type == OPCODE_REG || stack[tempst]->type == STACK_ENTRY_MARKER_NAME)
        {
            free(stack[tempst]->value);
        }

        free(stack[tempst]);
    }
    free(stack);

    /* freeing the jumptable */
    for(tempjmi = jumptable_size - 1; tempjmi >= 0; tempjmi --)
    {
        free(jumptable[tempjmi]);
    }

    free(jumptable);

    /* freeing the content */
    free(vm->content);
}


struct nap_vm *nap_vm_load(const char *filename)
{
    uint64_t fsize = 0;
    uint8_t type = 0;
    struct nap_vm* vm = NULL;
    FILE* fp = fopen(filename, "rb");
    if(!fp)
    {
        fprintf(stderr, "cannot load file [%s]", filename);
        return 0;
    }

    /* now we can create t he VM */
    vm = (struct nap_vm*)(calloc(1, sizeof(struct nap_vm)));

    /* read in all the data in memory. Should be faster */
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    content = (uint8_t *) calloc(sizeof(uint8_t), fsize);
    fseek(fp, 0, SEEK_SET);
    fread(content, sizeof(uint8_t ), fsize, fp);

    fseek(fp, 0, SEEK_SET);

    /* create the stack */
    vm->stack = (struct stack_entry**)calloc(sizeof(struct stack_entry*), stack_size);

    /* the format of the addresses in the file 32 or 64 bit addresses */
    fread(&type, sizeof(uint8_t), 1, fp);
    if(type == 0x32)
    {
        vm->file_bitsize = sizeof(uint32_t);
    }
    else
    {
        vm->file_bitsize = sizeof(uint64_t);
    }

    /* read in the important addresses from the bytecode file*/
    fread(&vm->meta_location, file_bitsize, 1, fp);
    fread(&vm->stringtable_location, file_bitsize, 1, fp);
    fread(&vm->jumptable_location, file_bitsize, 1, fp);

    /* prepare the meta table of the application */
    read_metatable(vm, fp, vm->meta_location);

    /* read the stringtable */
    read_stringtable(fp, vm->stringtable_location);

    /* read the jumptable */
    read_jumptable(fp, vm->jumptable_location);

    /* done with the file */
    fclose(fp);

    /* cc is the instruction pointer: skip the 3 addresses and the startbyte */
    cc = 3 * file_bitsize + 1;

}
