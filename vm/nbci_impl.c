/******************************************************************************/
/*                             Other functions section                        */
/******************************************************************************/

#include "nbci.h"
#include "metatbl.h"
#include "strtable.h"
#include "jmptable.h"
#include "opcodes.h"
#include "stack.h"
#include "byte_order.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "string.h"

void nap_vm_set_lbf_to_op_result(struct nap_vm* vm, nap_int_t reg, nap_int_t immediate, uint8_t opcode)
{
    register uint8_t temp_lbf;

    if(opcode == OPCODE_EQ)
    {
        temp_lbf = (reg == immediate);
    }
    else
    if(opcode == OPCODE_NEQ)
    {
        temp_lbf = (reg != immediate);
    }
    else
    if(opcode == OPCODE_LT)
    {
        temp_lbf = (reg <  immediate);
    }
    else
    if(opcode == OPCODE_GT)
    {
        temp_lbf = (reg >  immediate);
    }
    else
    if(opcode == OPCODE_LTE)
    {
        temp_lbf = (reg <= immediate);
    }
    else
    if(opcode == OPCODE_GTE)
    {
        temp_lbf = (reg >= immediate);
    }
    else
    {
        _NOT_IMPLEMENTED
    }

    if(vm->lbf == UNDECIDED)
    {
        vm->lbf = temp_lbf;
    }
    else
    {
        vm->lbf &= temp_lbf;
    }
}

void do_operation(struct nap_vm* vm, nap_int_t* target, nap_int_t operand, uint8_t opcode)
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
void nap_vm_cleanup(struct nap_vm* vm)
{
    uint64_t i;
    int64_t tempst;
    int64_t tempjmi;
    //dump(vm, stdout);
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

struct nap_vm* nap_vm_inject(uint8_t* bytecode, int bytecode_len)
{
    struct nap_vm* vm = NULL;
    uint8_t* cloc = bytecode;

    vm = (struct nap_vm*)(calloc(1, sizeof(struct nap_vm)));
    /* TODO: check memory */

    vm->content = (uint8_t *) calloc(sizeof(uint8_t), bytecode_len);
    /* TODO: check memory */

    /* create the stack */
    vm->stack_size = STACK_INIT;
    vm->stack_pointer = 0;
    vm->stack = (struct stack_entry**)calloc(sizeof(struct stack_entry*), STACK_INIT);
    if(vm->stack == NULL)
    {
        fprintf(stderr, "Cannot allocate stack\n");
        exit(45);
    }

    memcpy(vm->content, bytecode, bytecode_len);

    /* start interpreting the bytecode */

    /* fist step: the bits */
    if(*cloc == 0x32)
    {
        vm->file_bitsize = sizeof(uint32_t);
    }
    else
    {
        vm->file_bitsize = sizeof(uint64_t);
    }
    cloc ++;

    /* next: read in the important addresses from the bytecode file*/

    /* meta location */
    vm->meta_location = htovm_32(*(uint32_t*)(cloc));
    cloc += 4;

    /* stringtable location */
    vm->stringtable_location = htovm_32(*(uint32_t*)(cloc));
    cloc += 4;

    /* jumptable location */
    vm->jumptable_location = htovm_32(*(uint32_t*)(cloc));
    cloc += 4;

    /* registers used */
    vm->mrc = *cloc;
    cloc ++;

    /* flags of the file */
    vm->flags = *cloc;
    cloc ++;

    interpret_metatable(vm, bytecode + vm->meta_location, bytecode_len);

    interpret_stringtable(vm, bytecode + vm->stringtable_location, bytecode_len);

    interpret_jumptable(vm, bytecode + vm->jumptable_location, bytecode_len);

    /* cc is the instruction pointer:
     * skip the:
     * - startbyte            - 8 bits
     * - 3 addresses          - 3 * 32 (64) bits
     * - the register count   - 8 bits
     * - the flags            - 8 bits */
    vm->cc = 1 + 3 * vm->file_bitsize + 1
            + 1; /* to point to the first instruction */

    /* initially the last boolean flag is in an unknow state */
    vm->lbf = UNDECIDED;

    return vm;
}

struct nap_vm *nap_vm_load(const char *filename)
{
    long fsize = 0;
    uint8_t* file_content;
    struct nap_vm* vm = NULL;
    FILE* fp = fopen(filename, "rb");
    if(!fp)
    {
        fprintf(stderr, "cannot load file [%s]", filename);
        return 0;
    }

    /* read in all the data in memory. Should be faster */
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    file_content = (uint8_t *) calloc(sizeof(uint8_t), fsize);
    /* TODO: check memory */

    fseek(fp, 0, SEEK_SET);
    fread(file_content, sizeof(uint8_t ), fsize, fp);

    fclose(fp);

    vm = nap_vm_inject(file_content, fsize);
    free(file_content);

    return vm;
}

nap_addr_t nap_fetch_address(struct nap_vm *vm)
{
    nap_addr_t* p_addr = (nap_addr_t*)(vm->content + vm->cc);
    vm->cc += sizeof(nap_addr_t);
    return htovm_32(*p_addr);
}

nap_mark_t nap_fetch_mark(struct nap_vm* vm)
{
    nap_mark_t* p_marker_code = (nap_mark_t*)(vm->content + vm->cc);
    vm->cc += sizeof(nap_mark_t);
    return htovm_32(*p_marker_code);
}

nap_index_t nap_fetch_index(struct nap_vm* vm)
{
    nap_index_t* p_var_index = (nap_index_t*)(vm->content + vm->cc);
    vm->cc += sizeof(nap_index_t);
    return htovm_32(*p_var_index);
}

nap_int_t nap_read_immediate(struct nap_vm* vm)
{
    uint8_t imm_size = vm->content[vm->cc ++];
    nap_int_t nr = 0;
    /* and now read the number according to the size */
    if(imm_size == OPCODE_BYTE)
    {
        int8_t* immediate = (int8_t*)(vm->content + vm->cc);
        nr = *immediate;
        vm->cc ++;
    }
    else
    if(imm_size == OPCODE_SHORT)
    {
        int16_t* immediate = (int16_t*)(vm->content + vm->cc);
        int16_t temp_16 = htovm_16(*immediate);
        temp_16 = temp_16;
        nr = temp_16;
        vm->cc += 2;
    }
    else
    if(imm_size == OPCODE_LONG)
    {
        int32_t* immediate = (int32_t*)(vm->content + vm->cc);
        int32_t temp_32 = htovm_32(*immediate);
        nr = temp_32;
        vm->cc += 4;
    }
    else
    if(imm_size == OPCODE_HUGE)
    {
        int64_t* immediate = (int64_t*)(vm->content + vm->cc);
        int64_t temp_64 = htovm_64(*immediate);
        nr = temp_64;
        vm->cc += 8;
    }
    else
    {
        printf("invalid immediate size [push]: 0x%x", imm_size);
        _NOT_IMPLEMENTED
    }
    return nr;
}

/* the register "stack" used to save and restore the registers */
static nap_int_t* regi_stack[DEEPEST_RECURSION] = {0};
static nap_index_t regi_stack_idx = 0;

void nap_save_registers(struct nap_vm* vm)
{
    nap_int_t* tmp = calloc(REGISTER_COUNT, sizeof(nap_int_t));
    memcpy(tmp, vm->regi, vm->mrc * sizeof(nap_int_t));
    regi_stack[regi_stack_idx] = tmp;
    regi_stack_idx ++;
    if(regi_stack_idx == DEEPEST_RECURSION)
    {
        fprintf(stderr, "too deep recursion. (ie: too many pushall's)\n");
        nap_vm_cleanup(vm);
        exit(EXIT_FAILURE);
    }
}

void nap_restore_registers(struct nap_vm* vm)
{
    if(regi_stack_idx == 0)
    {
        fprintf(stderr, "cannot popall without a pushall\n");
        nap_vm_cleanup(vm);
        exit(EXIT_FAILURE);
    }

    regi_stack_idx --;
    memcpy(vm->regi, regi_stack[regi_stack_idx], vm->mrc * sizeof(nap_int_t));
    free(regi_stack[regi_stack_idx]);

}
