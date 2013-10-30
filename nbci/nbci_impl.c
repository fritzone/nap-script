/******************************************************************************/
/*                             Other functions section                        */
/******************************************************************************/

#include "nbci.h"
#include "metatbl.h"
#include "strtable.h"
#include "opcodes.h"
#include "stack.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void nap_vm_set_lbf_to_op_result(struct nap_vm* vm, nap_number_t reg, nap_number_t immediate, uint8_t opcode)
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

void do_operation(struct nap_vm* vm, nap_number_t* target, nap_number_t operand, uint8_t opcode)
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
    dump(vm, stdout);
    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                free(vm->metatable[i]->instantiation->value);
            }

            free(vm->metatable[i]->instantiation);
        }
    }
    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->name)
        {
            free(vm->metatable[i]->name);
        }

        if(vm->metatable[i])
        {
            free(vm->metatable[i]);
        }
    }
    free(vm->metatable);

    /* free the allocated stack */
    for(tempst = vm->stack_pointer; tempst > -1; tempst --)
    {
        if(vm->stack[tempst] && vm->stack[tempst]->type == OPCODE_INT) /* or float/string */
        {
            /* this wa already freed in the metatable */
        }
        else /* register type */
        if(vm->stack[tempst] && (vm->stack[tempst]->type == OPCODE_REG
                                 || vm->stack[tempst]->type == STACK_ENTRY_MARKER_NAME))
        {
            free(vm->stack[tempst]->value);
        }

        free(vm->stack[tempst]);
    }
    free(vm->stack);

    /* freeing the jumptable */
    for(tempjmi = vm->jumptable_size - 1; tempjmi >= 0; tempjmi --)
    {
        free(vm->jumptable[tempjmi]);
    }

    free(vm->jumptable);

    /* freeing the content */
    free(vm->content);

    /* and the VM itself */
    free(vm);
}


struct nap_vm *nap_vm_load(const char *filename)
{
    long fsize = 0;
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
    vm->content = (uint8_t *) calloc(sizeof(uint8_t), fsize);
    fseek(fp, 0, SEEK_SET);
    fread(vm->content, sizeof(uint8_t ), fsize, fp);

    fseek(fp, 0, SEEK_SET);

    /* create the stack */
    vm->stack_size = STACK_INIT;
    vm->stack_pointer = 0;
    vm->stack = (struct stack_entry**)calloc(sizeof(struct stack_entry*), STACK_INIT);
    if(vm->stack == NULL)
    {
        fprintf(stderr, "Cannot allocate stack\n");
        exit(45);
    }

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
    fread(&vm->meta_location, vm->file_bitsize, 1, fp);
    fread(&vm->stringtable_location, vm->file_bitsize, 1, fp);
    fread(&vm->jumptable_location, vm->file_bitsize, 1, fp);

    /* prepare the meta table of the application */
    read_metatable(vm, fp);

    /* read the stringtable */
    read_stringtable(vm, fp);

    /* read the jumptable */
    read_jumptable(vm, fp);

    /* done with the file */
    fclose(fp);

    /* cc is the instruction pointer: skip the 3 addresses and the startbyte */
    vm->cc = 3 * vm->file_bitsize + 1;

    return vm;
}

nap_addr_t nap_fetch_address(struct nap_vm *vm)
{
    nap_addr_t* p_addr = (nap_addr_t*)(vm->content + vm->cc);
    vm->cc += sizeof(nap_addr_t);
    return *p_addr;
}

nap_mark_t nap_fetch_mark(struct nap_vm* vm)
{
    nap_mark_t* p_marker_code = (nap_mark_t*)(vm->content + vm->cc);
    vm->cc += sizeof(nap_mark_t);
    return *p_marker_code;
}

nap_index_t nap_fetch_index(struct nap_vm* vm)
{
    nap_index_t* p_var_index = (nap_index_t*)(vm->content + vm->cc);
    vm->cc += sizeof(nap_index_t);
    return *p_var_index;
}

