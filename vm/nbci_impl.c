/******************************************************************************/
/*                             Other functions section                        */
/******************************************************************************/

#include "nbci_impl.h"

#include "nbci.h"
#include "metatbl.h"
#include "strtable.h"
#include "funtable.h"
#include "jmptable.h"
#include "opcodes.h"
#include "stack.h"
#include "byte_order.h"

#include "nap_consts.h"

/* interrupts */

#include "intr_1.h"

#if RUNTIME_COMPILATION
    #include "intr_2.h"
    #include "intr_3.h"
#endif

#include "intr_4.h"

/* opcode handlers */
#include "push.h"
#include "comparison.h"
#include "mov.h"
#include "jump.h"
#include "marks.h"
#include "clrs.h"
#include "call.h"
#include "peek.h"
#include "pop.h"
#include "return.h"
#include "store.h"
#include "serve.h"
#include "restore.h"
#include "inc.h"
#include "dec.h"
#include "clidx.h"
#include "poke.h"
#include "operation.h"
#include "leave.h"
#include "call_intern.h"
#include "unary.h"

/* character comverter from the compiler */
#include "charconverter.h"

/* system headers */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <iconv.h>
#include <stddef.h>

#define ERROR_COUNT 26

/* section for defining the constants */
static char* error_table[ERROR_COUNT + 1] =
{
    "[VM-0001] not enough memory ",
    "[VM-0002] stack underflow",
    "[VM-0003] cannot allocate a child VM in intr 3",
    "[VM-0004] unimplemented interrupt",
    "[VM-0005] \"eq\" works only on register/variable",
    "[VM-0006] call frame stack overflow (too deep recursion)",
    "[VM-0007] call frame stack empty, cannot popall (no pushall)",
    "[VM-0008] internal VM error: a variable is not defined",
    "[VM-0009] cannot decrement something",
    "[VM-0010] cannot increment something",
    "[VM-0011] cannot execute a move operation",
    "[VM-0012] cannot execute an operation",
    "[VM-0013] cannot peek from the stack",
    "[VM-0014] cannot pop from the stack",
    "[VM-0015] cannot push onto the stack",
    "[VM-0016] cannot create a temporay marker",
    "[VM-0017] unimplemented interrupt",
    "[VM-0018] a variable was not initialized correctly",
    "[VM-0019] too deep recursion.",
    "[VM-0020] Invalid jump index",
    "[VM-0021] Cannot leave from the bottom of the call frame pit",
    "[VM-0022] Invalid internal call",
    "[VM-0023] Division by zero",
    "[VM-0024] Cannot store/restore a value",
    "[VM-0025] Only indexed variables can be served",
    "[VM-0026] Cannot poke a non variable",

    "LAST_ENTRY_FOR_FUNNY_COMPILERS_WHO_DONT_LIKE_COMMAS_AT_LAST_POSITON"
};

/**
 * Cleans the allocated memory
 */
void nap_vm_cleanup(struct nap_vm* vm)
{
    uint64_t i;
    int64_t tempst;
    int64_t tempjmi;

    /* free the marks list */
    for(i=0; i<=vm->max_marks; i++)
    {
        NAP_MEM_FREE(vm->marks_list[i]);
    }
    NAP_MEM_FREE(vm->marks_list);

    /* free the metatable */
    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i])
        {
            if(vm->metatable[i]->name)
            {
                NAP_MEM_FREE(vm->metatable[i]->name);
                vm->metatable[i]->name = NULL;
            }
            if(vm->metatable[i]->instantiation)
            {
                if(vm->metatable[i]->instantiation->value)
                {
                    NAP_MEM_FREE(vm->metatable[i]->instantiation->value);
                    vm->metatable[i]->instantiation->value = NULL;
                }

                NAP_MEM_FREE(vm->metatable[i]->instantiation);
                vm->metatable[i]->instantiation = NULL;
            }

            NAP_MEM_FREE(vm->metatable[i]);
            vm->metatable[i] = NULL;
        }
    }
    NAP_MEM_FREE(vm->metatable);

    /* free the allocated stack */
    for(tempst = vm->cec->stack_pointer; tempst > -1; tempst --)
    {
        /* The variables are freed in the metatable, this is reserved for
         * freeing stuff pushed for registers */
        if(vm->cec->stack[tempst] && vm->cec->stack[tempst]->var_def == NULL)
        {
            NAP_MEM_FREE(vm->cec->stack[tempst]->value);
            vm->cec->stack[tempst]->value = NULL;
        }

        NAP_MEM_FREE(vm->cec->stack[tempst]);
        vm->cec->stack[tempst] = NULL;
    }

#ifdef PREFER_DYNAMIC_ALLOCATION
    NAP_MEM_FREE(vm->cec->stack);

    NAP_MEM_FREE(vm->cec->call_frames);
#endif

    /* freeing the jumptable */
	for(tempjmi = vm->jumptable_size; tempjmi > 0; tempjmi --)
	{
	    if(vm->jumptable[tempjmi - 1]->label_name)
		{
            NAP_MEM_FREE(vm->jumptable[tempjmi - 1]->label_name);
            vm->jumptable[tempjmi - 1]->label_name = NULL;
		}
        NAP_MEM_FREE(vm->jumptable[tempjmi - 1]);
        vm->jumptable[tempjmi - 1] = NULL;
	}

    NAP_MEM_FREE(vm->jumptable);

    /* freeing the content */
    NAP_MEM_FREE(vm->content);

    /* freeing the allocated bytecode chunks */
    for(i = 0; i<vm->allocated_chunks; i++)
    {
        if(vm->btyecode_chunks[i])
        {
            if(vm->btyecode_chunks[i]->code)
            {
                NAP_MEM_FREE(vm->btyecode_chunks[i]->code);
            }
            NAP_MEM_FREE(vm->btyecode_chunks[i]);
        }
    }

    NAP_MEM_FREE(vm->btyecode_chunks);

    /* the stringtable */
    for(i = 0; i<vm->strt_size; i++)
    {
        NAP_MEM_FREE(vm->stringtable[i]->string);
        NAP_MEM_FREE(vm->stringtable[i]);
    }
    NAP_MEM_FREE(vm->stringtable);

    /* free the funtable */
    for(i = 0; i<vm->funtable_entries; i++)
    {
        NAP_MEM_FREE(vm->funtable[i]->function_name);
        NAP_MEM_FREE(vm->funtable[i]->parameter_types);
        NAP_MEM_FREE(vm->funtable[i]);
    }
    NAP_MEM_FREE(vm->funtable);

    /* the error message will be freed only if it was not allocated */
    if(vm->error_message)
    {
        int needs_free = 1;
        for(i=0; i<ERROR_COUNT; i++)
        {
            if(vm->error_message == error_table[i])
            {
                needs_free = 0;
            }
        }

        if(needs_free)
        {
            NAP_MEM_FREE(vm->error_message);
        }
    }

    /* the string registers */
    for(i=0; i<REGISTER_COUNT; i++)
    {
#ifdef _WINDOWS
#pragma warning(suppress: 6001)
#endif
        NAP_MEM_FREE(vm->cec->regs[i].s);
    }

    /* the string return value */
    NAP_MEM_FREE(vm->cec->rvs);

    /* the current execution context */
    for(i=0; i<vm->ecs_cnt; i++)
    {
        NAP_MEM_FREE(vm->ecs[i]);
    }
    NAP_MEM_FREE(vm->ecs);

#ifdef PREFER_DYNAMIC_ALLOCATION
    for(i=0; i<vm->config->deepest_recursion; i++)
    {
        NAP_MEM_FREE(vm->regi_stack[i]);
        NAP_MEM_FREE(vm->regb_stack[i]);
    }

    NAP_MEM_FREE(vm->regi_stack);
    NAP_MEM_FREE(vm->regb_stack);


#endif


    /* and the VM itself */
    NAP_MEM_FREE(vm);
}

struct nap_vm* nap_vm_inject(uint8_t* bytecode, int bytecode_len, enum environments target, const struct startup_configuration *config)
{
    struct nap_vm* vm = NULL;
    uint8_t* cloc = bytecode;
    uint16_t i = 0; /* for walking through various stuffs */

#ifdef PREFER_DYNAMIC_ALLOCATION
    size_t stack_size_to_alloc = STACK_INIT;
    size_t register_recursion_stack_size = DEEPEST_RECURSION;

    if(config)
    {
        stack_size_to_alloc = config->stack_size?config->stack_size:STACK_INIT;
        register_recursion_stack_size = config->deepest_recursion?config->deepest_recursion:DEEPEST_RECURSION;
    }
#endif

    vm = NAP_MEM_ALLOC(1, struct nap_vm);
    if(vm == NULL)
    {
        /* TODO: provide a mechanism for error reporting */
        return NULL;
    }

    /* initializing the execution contexts vector */
    vm->ecs = NAP_MEM_ALLOC(1, struct nap_execution_context*);
    if(vm->ecs == NULL)
    {
        free(vm);
        return NULL;
    }

    /* creating the initial execution context */
    vm->cec = NAP_MEM_ALLOC(1, struct nap_execution_context);
    if(vm->cec == NULL)
    {
        free(vm->ecs);
        free(vm);
        return NULL;
    }

    /* initial setup for the ecs vector */
    vm->ecs_cnt = 1;
    vm->ecs[vm->ecs_cnt - 1] = vm->cec;

    /* the bytecode content of the vm */
    vm->content = NAP_MEM_ALLOC(bytecode_len, uint8_t);
    if(vm->content == NULL)
    {
        /* TODO: provide a mechanism for error reporting */
        free(vm->cec);
        free(vm);
        return NULL;
    }

    /* create the stack */
    vm->cec->stack_pointer = -1; /* this is first ++'d then used */
#ifdef PREFER_DYNAMIC_ALLOCATION
    vm->cec->stack = NAP_MEM_ALLOC(stack_size_to_alloc, struct stack_entry*);
#endif
    if(vm->cec->stack == NULL)
    {
        /* TODO: provide a mechanism for error reporting */
        free(vm->content);
        free(vm->cec);
        free(vm);
        return NULL;
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

    /* funtable location */
    vm->funtable_location = htovm_32(*(uint32_t*)(cloc));
    cloc += 4;

    /* registers used */
    vm->mrc = *cloc;
    cloc ++;

    /* flags of the file */
    vm->flags = *cloc;
    cloc ++;

    if(NAP_SUCCESS != interpret_metatable(vm, vm->content + vm->meta_location, bytecode_len))
    {
        nap_vm_cleanup(vm);
        return NULL;
    }

    if(NAP_SUCCESS != interpret_stringtable(vm, vm->content + vm->stringtable_location, bytecode_len))
    {
        nap_vm_cleanup(vm);
        return NULL;
    }

    if(NAP_SUCCESS != interpret_jumptable(vm, vm->content + vm->jumptable_location, bytecode_len))
    {
        nap_vm_cleanup(vm);
        return NULL;
    }

    if(NAP_SUCCESS != interpret_funtable(vm, vm->content + vm->funtable_location, bytecode_len))
    {
        nap_vm_cleanup(vm);
        return NULL;
    }

    /* cc is the instruction pointer:
     * skip the:
     * - startbyte            - 8 bits
     * - 3 addresses          - 4 * 32 (64) bits
     * - the register count   - 8 bits
     * - the flags            - 8 bits */
    nap_set_ip(vm, 1 + 4 * vm->file_bitsize + 1 + 1); /* to point to the first instruction */

    /* initially the last boolean flag is in an unknow state */
    vm->cec->lbf = UNDECIDED;

    /* allocating bytecode chunks */
    vm->btyecode_chunks = NAP_MEM_ALLOC(MAX_BYTECODE_CHUNKS, struct nap_bytecode_chunk*);
    if(vm->btyecode_chunks == NULL)
    {
        nap_vm_cleanup(vm);
        return NULL;
    }
    vm->chunk_counter = 0;
    vm->allocated_chunks = 255;

    /* and setting the interrupts */
    vm->interrupts[1] = intr_1;
#if RUNTIME_COMPILATION
    vm->interrupts[2] = intr_2;
    vm->interrupts[3] = intr_3;
#endif
    vm->interrupts[4] = intr_4;

    /* setting the opcode handlers */
    vm->opcode_handlers[OPCODE_PUSH] = nap_push; vm->opcode_error_codes[OPCODE_PUSH] = ERR_VM_0015;
    vm->opcode_handlers[OPCODE_EQ] = nap_comparison; vm->opcode_error_codes[OPCODE_EQ] = ERR_VM_0005;
    vm->opcode_handlers[OPCODE_LT] = nap_comparison; vm->opcode_error_codes[OPCODE_LT] = ERR_VM_0005;
    vm->opcode_handlers[OPCODE_GT] = nap_comparison; vm->opcode_error_codes[OPCODE_GT] = ERR_VM_0005;
    vm->opcode_handlers[OPCODE_NEQ] = nap_comparison; vm->opcode_error_codes[OPCODE_NEQ] = ERR_VM_0005;
    vm->opcode_handlers[OPCODE_LTE] = nap_comparison; vm->opcode_error_codes[OPCODE_LTE] = ERR_VM_0005;
    vm->opcode_handlers[OPCODE_GTE] = nap_comparison; vm->opcode_error_codes[OPCODE_GTE] = ERR_VM_0005;
    vm->opcode_handlers[OPCODE_MOV] = nap_mov; vm->opcode_error_codes[OPCODE_MOV] = ERR_VM_0011;
    vm->opcode_handlers[OPCODE_JLBF] = nap_jump; vm->opcode_error_codes[OPCODE_JLBF] = ERR_VM_0020;
    vm->opcode_handlers[OPCODE_JMP] = nap_jump; vm->opcode_error_codes[OPCODE_JMP] = ERR_VM_0020;
    vm->opcode_handlers[OPCODE_MARKS_NAME] = nap_marks; vm->opcode_error_codes[OPCODE_MARKS_NAME] = ERR_VM_0016;
    vm->opcode_handlers[OPCODE_CLRS_NAME] = nap_clrs; vm->opcode_error_codes[OPCODE_CLRS_NAME] = ERR_VM_0002;
    vm->opcode_handlers[OPCODE_CALL] = nap_call; vm->opcode_error_codes[OPCODE_CALL] = ERR_VM_0019;
    vm->opcode_handlers[OPCODE_CALL_INT] = nap_call_intern; vm->opcode_error_codes[OPCODE_CALL_INT] = ERR_VM_0022;
    vm->opcode_handlers[OPCODE_PEEK] = nap_peek; vm->opcode_error_codes[OPCODE_PEEK] = ERR_VM_0013;
    vm->opcode_handlers[OPCODE_POP] = nap_pop; vm->opcode_error_codes[OPCODE_POP] = ERR_VM_0014;
    vm->opcode_handlers[OPCODE_RETURN] = nap_return; vm->opcode_error_codes[OPCODE_RETURN] = 0;
    vm->opcode_handlers[OPCODE_INC] = nap_inc; vm->opcode_error_codes[OPCODE_INC] = ERR_VM_0010;
    vm->opcode_handlers[OPCODE_DEC] = nap_dec; vm->opcode_error_codes[OPCODE_DEC] = ERR_VM_0009;
    vm->opcode_handlers[OPCODE_POPALL] = nap_restore_registers; vm->opcode_error_codes[OPCODE_POPALL] = ERR_VM_0007;
    vm->opcode_handlers[OPCODE_PUSHALL] = nap_save_registers; vm->opcode_error_codes[OPCODE_PUSHALL] = ERR_VM_0006;
    vm->opcode_handlers[OPCODE_ADD] = nap_operation; vm->opcode_error_codes[OPCODE_ADD] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_MUL] = nap_operation; vm->opcode_error_codes[OPCODE_MUL] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_SUB] = nap_operation; vm->opcode_error_codes[OPCODE_SUB] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_DIV] = nap_operation; vm->opcode_error_codes[OPCODE_DIV] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_MOD] = nap_operation; vm->opcode_error_codes[OPCODE_MOD] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_SHL] = nap_operation; vm->opcode_error_codes[OPCODE_SHL] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_SHR] = nap_operation; vm->opcode_error_codes[OPCODE_SHR] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_AND] = nap_operation; vm->opcode_error_codes[OPCODE_AND] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_OR] = nap_operation; vm->opcode_error_codes[OPCODE_OR] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_XOR] = nap_operation; vm->opcode_error_codes[OPCODE_XOR] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_NOT] = nap_unary; vm->opcode_error_codes[OPCODE_NOT] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_NEG] = nap_unary; vm->opcode_error_codes[OPCODE_NEG] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_POS] = nap_unary; vm->opcode_error_codes[OPCODE_POS] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_BCOM] = nap_unary; vm->opcode_error_codes[OPCODE_BCOM] = ERR_VM_0012;
    vm->opcode_handlers[OPCODE_INTR] = nap_handle_interrupt; vm->opcode_error_codes[OPCODE_INTR] = ERR_VM_0017;
    vm->opcode_handlers[OPCODE_CLIDX] = nap_clidx; vm->opcode_error_codes[OPCODE_CLIDX] = 0;
    vm->opcode_handlers[OPCODE_LEAVE] = nap_leave; vm->opcode_error_codes[OPCODE_LEAVE] = ERR_VM_0021;
    vm->opcode_handlers[OPCODE_STORE] = nap_store; vm->opcode_error_codes[OPCODE_STORE] = ERR_VM_0024;
    vm->opcode_handlers[OPCODE_RESTORE] = nap_restore; vm->opcode_error_codes[OPCODE_STORE] = ERR_VM_0024;
    vm->opcode_handlers[OPCODE_SERVE] = nap_serve; vm->opcode_error_codes[OPCODE_SERVE] = ERR_VM_0025;
    vm->opcode_handlers[OPCODE_POKE] = nap_poke; vm->opcode_error_codes[OPCODE_POKE] = ERR_VM_0026;

    /* setting the mov handlers */
    vm->mov_handlers[OPCODE_REG] = mov_into_register;
    vm->mov_handlers[OPCODE_VAR] = mov_into_variable;
    vm->mov_handlers[OPCODE_CCIDX] = mov_into_indexed;

    /* setting the container type of the VM*/
    vm->environment = target;

    /* Fixing the register "stack" used to save and restore the registers */
    vm->reg_stack_idx = 0;

#ifdef PREFER_DYNAMIC_ALLOCATION
    vm->regi_stack = NAP_MEM_ALLOC(register_recursion_stack_size, nap_int_t*);
    vm->regb_stack = NAP_MEM_ALLOC(register_recursion_stack_size, nap_byte_t*);
    for(i=0; i<register_recursion_stack_size; i++)
    {
        vm->regi_stack[i] = NAP_MEM_ALLOC(REGISTER_COUNT, nap_int_t);
        vm->regb_stack[i] = NAP_MEM_ALLOC(REGISTER_COUNT, nap_byte_t);
    }

    vm->cec->call_frames = NAP_MEM_ALLOC(register_recursion_stack_size, uint64_t);

#endif

    /* initialize the marks list */
    vm->marks_list = NAP_MEM_ALLOC(vm->max_marks + 1, struct stack_entry*);
    for(i=0; i<=vm->max_marks; i++)
    {
        vm->marks_list[i] = NAP_MEM_ALLOC(1, struct stack_entry);
        vm->marks_list[i]->type = STACK_ENTRY_MARKER_NAME;
        vm->marks_list[i]->len = i;
    }

    vm->config = config;

    return vm;
}


int nap_save_registers(struct nap_vm* vm)
{
    if(vm->reg_stack_idx + 1 == vm->config->deepest_recursion)
    {
        nap_vm_set_error_description(vm, "Too deep recursion. Execution halted");
        return NAP_FAILURE;
    }

    /* save the int registers */
    memcpy(vm->regi_stack[vm->reg_stack_idx], vm->cec->regi, vm->mrc * sizeof(nap_int_t));

    /* save the byte registers */
    memcpy(vm->regb_stack[vm->reg_stack_idx], vm->cec->regb, vm->mrc * sizeof(nap_byte_t));

    vm->reg_stack_idx ++;

    return NAP_SUCCESS;
}

int nap_restore_registers(struct nap_vm* vm)
{
    if(vm->reg_stack_idx == 0)
    {
        nap_vm_set_error_description(vm, "Cannot restore registers: None saved");
        return NAP_FAILURE;
    }

    vm->reg_stack_idx --;

    /* restore the int registers */
    memcpy(vm->cec->regi, vm->regi_stack[vm->reg_stack_idx], vm->mrc * sizeof(nap_int_t));

    /* restore the byte registers */
    memcpy(vm->cec->regb, vm->regb_stack[vm->reg_stack_idx], vm->mrc * sizeof(nap_byte_t));

    return NAP_SUCCESS;
}

struct nap_vm *nap_vm_load(const char *filename, const struct startup_configuration* config)
{
    long fsize = 0;
    uint8_t* file_content;
    struct nap_vm* vm = NULL;
    FILE* fp = fopen(filename, "rb");
    size_t loaded_bytes = 0;
    if(!fp)
    {
        fprintf(stderr, "cannot open file [%s]", filename);
        return NULL;
    }

    /* read in all the data in memory. Should be faster */
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    file_content = NAP_MEM_ALLOC(fsize, uint8_t);
    if(file_content == NULL)
    {
        fprintf(stderr, "cannot load file [%s]. Not enough memory", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_SET);
    loaded_bytes = fread(file_content, sizeof(uint8_t ), fsize, fp);
    if(loaded_bytes != fsize * sizeof(uint8_t) || loaded_bytes == 0)
    {
        fprintf(stderr, "cannot load file [%s]. File reading error", filename);
        return NULL;
    }

    fclose(fp);

    vm = nap_vm_inject(file_content, fsize, STANDALONE, config);
    NAP_MEM_FREE(file_content);

    if(vm == NULL)
    {
        fprintf(stderr, "cannot load file [%s]. Cannot create VM", filename);
        return NULL;
    }

    return vm;
}

nap_addr_t nap_fetch_address(struct nap_vm *vm)
{
    nap_addr_t* p_addr = (nap_addr_t*)(vm->content + nap_ip(vm));
    nap_move_ip(vm, sizeof(nap_addr_t), FORWARD);
    return htovm_32(*p_addr);
}

nap_int_t nap_read_immediate_int(struct nap_vm* vm, int* success)
{
    uint8_t imm_size = vm->content[nap_step_ip(vm)];
    nap_int_t nr = 0;
    *success = NAP_SUCCESS;
    /* and now read the number according to the size */
    if(imm_size == OPCODE_BYTE)
    {
        int8_t* immediate = (int8_t*)(vm->content + nap_ip(vm));
        nr = *immediate;
        nap_step_ip(vm);
    }
    else
    if(imm_size == OPCODE_SHORT)
    {
        int16_t* immediate = (int16_t*)(vm->content + nap_ip(vm));
        int16_t temp_16 = htovm_16(*immediate);
        nr = temp_16;
        nap_move_ip(vm, 2, FORWARD);
    }
    else
    if(imm_size == OPCODE_LONG)
    {
        int32_t* immediate = (int32_t*)(vm->content + nap_ip(vm));
        int32_t temp_32 = htovm_32(*immediate);
        nr = temp_32;
        nap_move_ip(vm, 4, FORWARD);
    }
    else
    if(imm_size == OPCODE_HUGE)
    {
        int64_t* immediate = (int64_t*)(vm->content + nap_ip(vm));
        int64_t temp_64 = htovm_64(*immediate);
        nr = temp_64;
        nap_move_ip(vm, 8, FORWARD);
    }
    else
    {
		char s[256] = {0};
        SNPRINTF(s, MAX_BUF_SIZE(255), "invalid immediate size  0x%x at %"PRINT_u" (%"PRINT_x";",
                 (unsigned)imm_size, nap_ip(vm), nap_ip(vm));
        nap_vm_set_error_description(vm, s);
        *success = NAP_FAILURE;
    }
    return nr;
}

int nap_vmi_has_variable(const struct nap_vm* vm, const char* name, int* type)
{
    uint64_t i;

    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                if(vm->metatable[i]->name && !strcmp(vm->metatable[i]->name, name))
                {
                    *type = (int)vm->metatable[i]->instantiation->type;
                    return 1;
                }
            }
        }
    }
    *type = -1;
    return 0;
}

int nap_handle_interrupt(struct nap_vm* vm)
{
    /* CC points to the interrupt number */
    uint8_t intr = *(uint8_t*)(vm->content + nap_ip(vm));
    uint16_t int_res = 0;
	char s[64] = {0};

    if(vm->interrupts[intr])
    {
        int_res = (vm->interrupts[intr])(vm);
    }
    else
    {
        /* unimplemented interrupt, reporting an error */
        SNPRINTF(s, MAX_BUF_SIZE(63), "unimplemented interrupt: %d", intr);
        return nap_vm_set_error_description(vm, s);
    }

    if(int_res == NAP_SUCCESS)
    {
        /* advance to the next position */
        nap_step_ip(vm);

        return NAP_SUCCESS;
    }

    SNPRINTF(s, MAX_BUF_SIZE(63), "interrupt [%d] failure code [%d]", intr, int_res);
    return nap_vm_set_error_description(vm, s);
}


struct variable_entry *nap_fetch_variable(struct nap_vm* vm, nap_index_t var_index)
{
    struct variable_entry* ve = vm->metatable[var_index];
    if(ve->type == EXTERN_VAR)
    {
        /* fetch the variable from the parent VMs */
        struct nap_vm* current_vm = vm->parent;
        while(current_vm)
        {
            struct variable_entry* ve_2 = nap_vmi_get_variable(current_vm, ve->name);
            if(ve_2 && ve_2->type == OWN_VAR)
            {
                return ve_2;
            }
            else
            {
                current_vm = current_vm->parent;
            }
        }

        /* nothing found, need to return NULL */
        return NULL;
    }
    else
    {
        return ve;
    }
}

struct variable_entry *nap_vmi_get_variable(const struct nap_vm *vm, const char *name)
{
    size_t i;
    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                if(vm->metatable[i]->name && !strcmp(vm->metatable[i]->name, name))
                {
                    return vm->metatable[i];
                }
            }
        }
    }
    return NULL;
}

struct funtable_entry* nap_vm_get_method(struct nap_vm* vm, const char* method_name)
{
    /* fist step: fetch the method with the given name */
    size_t i = 0;
    int meth_idx = -1;
    for(i =0; i<vm->funtable_entries; i++)
    {
        if(!strcmp(method_name, vm->funtable[i]->function_name))
        {
            meth_idx = i;
            break; /* TODO: later: method overloading, differrent parameters count*/
        }
    }

    if(meth_idx == -1)
    {
        /* no method found */
        return NULL;
    }

    return vm->funtable[i];
}


void nap_set_error(struct nap_vm *vm, int error_code)
{
    if(vm->error_message != NULL)
    {
        free(vm->error_message);
    }
    vm->error_code = error_code;
    vm->error_message = error_table[error_code - 1];
}

const char *nap_get_type_description(StackEntryType t)
{
    switch(t)
    {
        case STACK_ENTRY_INT : return "int";
        case STACK_ENTRY_BYTE : return "byte";
        case STACK_ENTRY_REAL : return "real";
        case STACK_ENTRY_STRING : return "string";
        case STACK_ENTRY_MARKER_NAME : return "mark";
        default: return "unk";
    }
}
char *convert_string_from_bytecode_file(struct nap_vm *vm, const char *src, size_t len, size_t dest_len, size_t* real_len)
{
    char* loc_orig = 0; /* the original locale string for LC_ALL */
    int len_loc = 0;
    char* enc = 0;      /* the actual encoding */
    char* loc_cp = 0;   /* copying over the loc_orig to not to mess with orig */
    char* converted = 0; /* will be used by iconv */
    char* orig_converted = 0; /* will have the beginning of the converted */
    iconv_t converter;  /* the converter itself */
    int ret = -1;       /* errorchecking */
    size_t save_dest_len = dest_len; /* used in real lenth calcualtions */
    char* to_return = NULL;
	char* final_encoding = NULL;
    char* src_copy = NAP_MEM_ALLOC(len + 1, char);
    char* src_copy_save = src_copy;

    if(src_copy == NULL)
    {
        nap_vm_set_error_description(vm, "[iconv] Not enough memory [1]");
        return NULL;
    }

    /*copy the src*/
    memcpy(src_copy, src, len);

    /* get the locale info */
    setlocale(LC_ALL, "");
    loc_orig = setlocale(LC_CTYPE, NULL);
    len_loc = strlen(loc_orig);
    loc_cp = NAP_MEM_ALLOC(len_loc + 1, char);
    if(loc_cp == NULL)
    {
        free(src_copy);
        nap_vm_set_error_description(vm, "[iconv] Not enough memory [2]");
        return NULL;
    }
    strcpy(loc_cp, loc_orig);

    /* the encoding of the locale */
    enc = strchr(loc_cp, '.') + 1;

#ifdef _WINDOWS
    final_encoding = NAP_MEM_ALLOC(8 + 1 + strlen(enc), char); /* WINDOWS- */
    if(final_encoding == NULL)
    {
        free(loc_cp);
        free(src_copy);
        return NULL;
    }
    strcpy(final_encoding, "WINDOWS-";;
	strcat(final_encoding, enc);
#else
	final_encoding = enc;
#endif

    /* creating an iconv converter */
    converter = iconv_open(final_encoding, "UTF-32BE");
    if((size_t)converter == (size_t)-1)
    {
        free(loc_cp);
        free(src_copy);
#ifdef _WINDOWS
        free(final_encoding);
#endif
        if (errno == EINVAL)
        {
			char s[256] = {0};
            SNPRINTF(s, MAX_BUF_SIZE(255), "[iconv] Conversion to %s is not supported\n", enc);
            nap_vm_set_error_description(vm, s);
        }
        else
        {
            nap_vm_set_error_description(vm, "[iconv] Initialization failure");
        }
        return NULL;
    }

    /* creating the work buffer for iconv */
    converted = NAP_MEM_ALLOC(dest_len, char);
    if(converted == NULL)
    {
        free(loc_cp);
        free(src_copy);
#ifdef _WINDOWS
        free(final_encoding);
#endif
        return NULL;
    }

    orig_converted = converted;

    /* converting */
    ret = iconv(converter, &src_copy, &len, &converted, &dest_len);
    iconv_close(converter);

    if(ret == -1)
    {
		char s[256] = {0};

        switch(errno)
        {
        case EILSEQ:
            SNPRINTF(s, MAX_BUF_SIZE(255), "[iconv] %s EILSEQ", strerror(errno));
            break;
        case EINVAL:
            SNPRINTF(s, MAX_BUF_SIZE(255), "[iconv] %s EINVAL\n", strerror(errno));
            break;
        case E2BIG:
            SNPRINTF(s, MAX_BUF_SIZE(255), "[iconv] %s E2BIG\n", strerror(errno));
            break;
        }

        nap_vm_set_error_description(vm, s);

        free(orig_converted);
        free(src_copy_save);
        free(loc_cp);
#ifdef _WINDOWS
        free(final_encoding);
#endif

        // DEBUG
        printf("%s\n", s);
        return NULL;
    }

    /* calculating the length of the converted string */
    *real_len = save_dest_len - dest_len;

    /* and what to return */
    to_return = NAP_MEM_ALLOC(*real_len + 1, char);
    if(to_return == NULL)
    {
        free(orig_converted);
        free(src_copy_save);
        free(loc_cp);
#ifdef _WINDOWS
        free(final_encoding);
#endif
		return NULL;
    }
    strncpy(to_return, orig_converted, *real_len);

    free(orig_converted);
    free(src_copy_save);
    free(loc_cp);
#ifdef _WINDOWS
    free(final_encoding);
#endif
    return to_return;
}

/* Only if the IP and SP handling is not done via macros */
#ifndef VM_IPSP_AS_MACRO

#if !defined(_MSC_VER)
inline 
#endif
uint64_t nap_step_ip(struct nap_vm *vm)
{
    return vm->cec->cc ++;
}

#if !defined(_MSC_VER)
inline
#endif
uint64_t nap_ip(const struct nap_vm *vm)
{
    return vm->cec->cc;
}

#if !defined(_MSC_VER)
inline
#endif
void nap_set_ip(struct nap_vm *vm, uint64_t new_ip)
{
    vm->cec->cc = new_ip;
}

#if !defined(_MSC_VER)
inline
#endif
void nap_move_ip(struct nap_vm *vm, uint64_t delta, signed char direction)
{
    vm->cec->cc = vm->cec->cc + direction * delta;
}

#if !defined(_MSC_VER)
inline
#endif
int64_t nap_sp(struct nap_vm *vm)
{
    return vm->cec->stack_pointer;
}

#if !defined(_MSC_VER)
inline
#endif
void nap_set_regi(struct nap_vm *vm, uint8_t register_index, nap_int_t v)
{
    vm->cec->regi[register_index] = v;
}

nap_index_t nap_fetch_index(struct nap_vm* vm)
{
    nap_index_t* p_var_index = (nap_index_t*)(vm->content + nap_ip(vm));
    nap_move_ip(vm, sizeof(nap_index_t), FORWARD);
    return htovm_32(*p_var_index);
}

nap_mark_t nap_fetch_mark(struct nap_vm* vm)
{
    nap_mark_t* p_marker_code = (nap_mark_t*)(vm->content + nap_ip(vm));
    nap_move_ip(vm, sizeof(nap_mark_t), FORWARD);
    return (*p_marker_code);
}

nap_int_t nap_regi(struct nap_vm *vm, uint8_t register_index)
{
    return vm->cec->regi[register_index];
}
#endif

void nap_set_regb(struct nap_vm *vm, uint8_t register_index, nap_byte_t v)
{
    vm->cec->regb[register_index] = v;
}

nap_byte_t nap_regb(struct nap_vm *vm, uint8_t register_index)
{
    return vm->cec->regb[register_index];
}


nap_real_t nap_regr(struct nap_vm *vm, uint8_t register_index)
{
    return vm->cec->regr[register_index];
}


void nap_set_regidx(struct nap_vm *vm, uint8_t register_index, nap_int_t v)
{
    vm->cec->regidx[register_index] = v;
}

nap_int_t nap_regidx(struct nap_vm *vm, uint8_t register_index)
{
    return vm->cec->regidx[register_index];
}

void nap_set_regr(struct nap_vm *vm, uint8_t register_index, nap_real_t v)
{
    vm->cec->regr[register_index] = v;
}


int nap_copy_return_values(const struct nap_vm *src, struct nap_vm *dst)
{
    if(src->cec->rvl)
    {
		char* tmp = NULL;
        dst->cec->rvl = src->cec->rvl;
        tmp = calloc(src->cec->rvl * CC_MUL, sizeof(char));

        if(tmp != NULL)
        {
            memcpy(tmp, src->cec->rvs, src->cec->rvl  * CC_MUL);
            NAP_MEM_FREE(dst->cec->rvs);
            dst->cec->rvs = tmp;
        }
        else
        {
            return NAP_FAILURE;
        }
    }
    dst->cec->rvb = src->cec->rvb;
    dst->cec->rvi = src->cec->rvi;
    dst->cec->rvr = src->cec->rvr;

    return NAP_SUCCESS;
}

char* nap_int_to_string(nap_int_t value, size_t* len)
{
	char s[1024] = {0}; /* temporary value */
    char* t = NULL; /* the return value */
    SNPRINTF(s, MAX_BUF_SIZE(1023), "%" PRINT_d "", value);
    t = to_nap_format(s, strlen(s), len);
    return t;
}

int nap_set_regs(struct nap_vm* vm, uint8_t register_index,
                                  const char* target, size_t target_len)
{
    char* tmp = NULL;
    NAP_STRING_ALLOC(vm, tmp, target_len);
    NAP_STRING_COPY(tmp, target, target_len);

    if(nap_regs(vm, register_index)->s)
    {
        /* set the memory value to zero */
        memset(vm->cec->regs[register_index].s, 0, vm->cec->regs[register_index].l * CC_MUL);

        /* and actually free the memory */
        NAP_MEM_FREE(vm->cec->regs[register_index].s);
    }

    vm->cec->regs[register_index].s = tmp;
    vm->cec->regs[register_index].l = target_len;

    return NAP_SUCCESS;
}

struct nap_string_register* nap_regs(struct nap_vm *vm, uint8_t register_index)
{
    return &(vm->cec->regs[register_index]);
}

nap_real_t unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    nap_real_t result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (i == 0) return 0.0;

    // pull the significand
    result = (nap_real_t)(i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}

nap_real_t nap_read_immediate_real(struct nap_vm *vm)
{
    uint64_t* immediate_r = (uint64_t*)(vm->content + nap_ip(vm));
    uint64_t temp_64 = htovm_64(*immediate_r);
    nap_real_t v = unpack754_64(temp_64);
    nap_move_ip(vm, 8, FORWARD);
    return v;
}

/**
 * @brief deliver_flat_index Delivers a flat index according to the index
 * registers of the vm for the given variable.
 *
 * The algorithm is like:
 *
 * array[N, M, K] -> flat_memory[N * M * K]
 * flat_index(i, j, k) = (M*N) * i + M * j + k
 * array[N, M, K, L] -> flat_memory[N * M * K * L]
 * flat_index(i, j, k, l) = (M*N*K) * i + (M*N) * j + M* k + l
 *
 * @param vm - the virtual machine in which we are running
 * @param ve - the variable entry
 * @param used_indexes - the number of used indexes
 * @param error - the address of an error string to be populated, just in case
 *
 * @return the real index, or:
 *   INVALID_INDEX_COUNT - if the caller specified an invalid index count (ie:
 *                         the number of indexes of the variable is not equal
 *                         to the requested used_indexes)
 *   INDEX_OUT_OF_RANGE - in case any of the idnexes runs out from the interval
 */
int64_t deliver_flat_index(struct nap_vm* vm,
                                   const struct variable_entry* ve,
                                   uint8_t used_indexes, char** error)
{
    int64_t to_ret = 0;
    uint8_t i = 0;

    /* moving block of arrays is not permitted yet :( */

    /* checking the validity of the requested index count */
    if(used_indexes != ve->dimension_count && ve->instantiation->type != STACK_ENTRY_STRING)
    {
        char* s = NAP_MEM_ALLOC(256, char);
        NAP_NN_ASSERT(vm, s);

        SNPRINTF(s, MAX_BUF_SIZE(255),
                "Invalid index count used for [%s]. "
                 "Requested: %d available: %d",
                 ve->name, used_indexes, ve->dimension_count);
        *error = s;
        return INVALID_INDEX_COUNT;
    }

    /* and calculating the index */
    for(; i<used_indexes; i++)
    {
        int j = 0;
        int64_t dim_multiplier = 1;

        /* is this a valid index? */
        if( ( (nap_regidx(vm, i) < 0) || (nap_regidx(vm, i) >= ve->dimensions[i]))
                && ve->instantiation->type != STACK_ENTRY_STRING )
        {
            char* s = NAP_MEM_ALLOC(256, char);
            NAP_NN_ASSERT(vm, s);

            SNPRINTF(s, MAX_BUF_SIZE(255),
                    "Multi dim index out of range for [%s]. "
                     "Dim Index: %d, requested: %" PRINT_d " available: %" PRINT_d,
                     ve->name, i, nap_regidx(vm, i), ve->dimensions[i]);
            *error = s;
            return INDEX_OUT_OF_RANGE; /* an index overflow */
        }

        /* the big dimension multiplier */
        while(j < used_indexes - i - 1)
        {
            dim_multiplier *= ve->dimensions[j ++];
        }

        /* and multiplying it with the requested index */
        dim_multiplier *= nap_regidx(vm, i);

        /* updating the final index*/
        to_ret += dim_multiplier;
    }

    return to_ret;
}

nap_real_t nap_string_to_number_real(struct nap_vm* vm,
                                     const char* to_conv,
                                     size_t len,
                                     int* error)
{
    vm = vm;
    to_conv = to_conv;
    len = len;
    *error = NAP_SUCCESS;
    return 0;
}

/* Returns a number from the given string */
nap_int_t nap_string_to_number_int(struct nap_vm* vm, const char* to_conv,
                                          size_t len, int* error)
{
    int base = 10;
    char* endptr = NULL;
    size_t dest_len = len * CC_MUL, real_len = 0;
    char* t = convert_string_from_bytecode_file(vm, (char*)to_conv, len * CC_MUL,
                                                dest_len, &real_len);
    char *save_t = t;
    nap_int_t v = 0;
    if(!t)
    {
        *error = NAP_FAILURE; /* the VM has already its error set */
        return NAP_NO_VALUE;
    }

    if(strlen(t) > 1)
    {
        if(t[0] == '0') /* octal? */
        {
            t ++;
            base = 8;
        }
        if(strlen(t) > 1)
        {
            if(t[0] == 'x') /* hex */
            {
                t ++;
                base = 16;
            }
            else
            if(t[0] == 'b') /* binary */
            {
                t ++;
                base = 2;
            }
        }
        else /* this was a simple "0" */
        {
            t --; /* stepping back one */
        }
    }
    v = strtoll(t, &endptr, base);
    free(save_t);

    if(errno == ERANGE || errno == EINVAL)
    {
        *error = NAP_FAILURE;
    }
    else
    {
        *error = NAP_SUCCESS;
    }
    return v;
}

const char* opcode_name(int opcode)
{
    const char* expr = NULL;

    if (opcode == OPCODE_PUSH) expr = "push";
    if (opcode == OPCODE_REF) expr = "ref";
    if (opcode == OPCODE_INT) expr = "int";
    if (opcode == OPCODE_GENERIC) expr = "generic";
    if (opcode == OPCODE_BYTE) expr = "byte";
    if (opcode == OPCODE_STRING) expr = "char";
    if (opcode == OPCODE_REAL) expr = "real";
    if (opcode == OPCODE_STRING) expr = "string";
    if (opcode == OPCODE_IDX) expr = "idx";
    if (opcode == OPCODE_INTR) expr = "intr";
    if (opcode == OPCODE_CALL) expr = "call";
    if (opcode == OPCODE_MOV) expr = "mov";
    if (opcode == OPCODE_INC) expr = "inc";
    if (opcode == OPCODE_DEC) expr = "dec";
    if (opcode == OPCODE_REG) expr = "reg";
    if (opcode == OPCODE_EXIT) expr = "exit";
    if (opcode == OPCODE_STORE) expr = "store";
    if (opcode == OPCODE_RESTORE) expr = "restore";
    if (opcode == OPCODE_ADD) expr = "add";
    if (opcode == OPCODE_SUB) expr = "sub";
    if (opcode == OPCODE_MUL) expr = "mul";
    if (opcode == OPCODE_DIV) expr = "div";
    if (opcode == OPCODE_MOD) expr = "mod";
    if (opcode == OPCODE_SHL) expr = "shl";
    if (opcode == OPCODE_POKE) expr = "poke";
    if (opcode == OPCODE_SHR) expr = "shr";
    if (opcode == OPCODE_AND) expr = "and";
    if (opcode == OPCODE_OR) expr = "or";
    if (opcode == OPCODE_XOR) expr = "xor";
    if (opcode == OPCODE_NOT) expr = "not";
    if (opcode == OPCODE_BCOM) expr = "bcom";
    if (opcode == OPCODE_NEG) expr = "neg";
    if (opcode == OPCODE_POS) expr = "pos";
    if (opcode == OPCODE_EQ) expr = "eq";
    if (opcode == OPCODE_LT) expr = "lt";
    if (opcode == OPCODE_GT) expr = "gt";
    if (opcode == OPCODE_LTE) expr = "lte";
    if (opcode == OPCODE_GTE) expr = "gte";
    if (opcode == OPCODE_NEQ) expr = "neq";
    if (opcode == OPCODE_JLBF) expr = "jlbf";
    if (opcode == OPCODE_JMP) expr = "jmp";
    if (opcode == OPCODE_MARKS_NAME) expr = "marksn";
    if (opcode == OPCODE_CLRS_NAME) expr = "clrsn";
    if (opcode == OPCODE_RETURN) expr = "return";
    if (opcode == OPCODE_RV) expr = "rv";
    if (opcode == OPCODE_POP) expr = "pop";
    if (opcode == OPCODE_PEEK) expr = "peek";
    if (opcode == OPCODE_SERVE) expr = "serve";
    if (opcode == OPCODE_CLIDX) expr = "clidx";
    if (opcode == OPCODE_LEAVE) expr = "leave";
    if (opcode == OPCODE_PUSHALL) expr = "pushall";
    if (opcode == OPCODE_POPALL) expr = "popall";
    if (opcode == OPCODE_CLBF) expr = "clbf";

    return expr;
}

#ifdef NAP_MEM_DEBUG

void* allocator(size_t count, const char *fn, long line)
{
    void* v = calloc(1, count);
    fprintf(stderr, "crea:%p (%s:%ld)\n", v, fn, line);
    return v;
}

#endif
