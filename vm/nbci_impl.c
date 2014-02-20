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
#include "intr_2.h"
#include "intr_3.h"
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
#include "inc.h"
#include "dec.h"
#include "clidx.h"
#include "operation.h"
#include "leave.h"
#include "call_intern.h"

/* system headers */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "string.h"
#include <locale.h>
#include <errno.h>
#include <iconv.h>
#include <stddef.h>

#define ERROR_COUNT 23

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
    "[VM-0012] cannot execute an arithmetic operation",
    "[VM-0013] cannot peek from the stack",
    "[VM-0014] cannot pop from the stack",
    "[VM-0015] cannot push onto the stack",
    "[VM-0016] cannot create a temporay marker",
    "[VM-0017] unimplemented interrupt",
    "[VM-0018] a variable was not initialized correctly",
    "[VM-0019] too deep recursion. Max 4096 nested calls are allowed",
    "[VM-0020] Invalid jump index",
    "[VM-0021] Cannot leave from the bottom of the call frame pit",
    "[VM-0022] Invalid internal call",
    "[VM-0023] Division by zero",

    "LAST_ENTRY_FOR_FUNNY_COMPILERS_WHO_DONT_LIKE_COMMAS"
};

/**
 * Cleans the allocated memory
 */
void nap_vm_cleanup(struct nap_vm* vm)
{
    uint64_t i;
    int64_t tempst;
    int64_t tempjmi;
    /* dump(vm, stdout); */

    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                /* fprintf(stderr, "%s=%d\n", vm->metatable[i]->name, *(int*)vm->metatable[i]->instantiation->value); */
                MEM_FREE(vm->metatable[i]->instantiation->value);
            }

            MEM_FREE(vm->metatable[i]->instantiation);
        }
    }
    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->name)
        {
            MEM_FREE(vm->metatable[i]->name);
        }

        if(vm->metatable[i])
        {
            MEM_FREE(vm->metatable[i]);
        }
    }
    MEM_FREE(vm->metatable);

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
            MEM_FREE(vm->stack[tempst]->value);
        }

        MEM_FREE(vm->stack[tempst]);
    }
    MEM_FREE(vm->stack);

    /* freeing the jumptable */
	for(tempjmi = vm->jumptable_size; tempjmi > 0; tempjmi --)
	{
	    if(vm->jumptable[tempjmi - 1]->label_name)
		{
	        MEM_FREE(vm->jumptable[tempjmi - 1]->label_name);
		}
		MEM_FREE(vm->jumptable[tempjmi - 1]);
	}

    MEM_FREE(vm->jumptable);

    /* freeing the content */
    MEM_FREE(vm->content);

    /* freeing the allocated bytecode chunks */
    for(i = 0; i<vm->allocated_chunks; i++)
    {
        if(vm->btyecode_chunks[i])
        {
            if(vm->btyecode_chunks[i]->code)
            {
                MEM_FREE(vm->btyecode_chunks[i]->code);
            }
            MEM_FREE(vm->btyecode_chunks[i]);
        }
    }

    MEM_FREE(vm->btyecode_chunks);

    /* the stringtable */
    for(i = 0; i<vm->strt_size; i++)
    {
        MEM_FREE(vm->stringtable[i]->string);
        MEM_FREE(vm->stringtable[i]);
    }
    MEM_FREE(vm->stringtable);

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
            MEM_FREE(vm->error_message);
        }
    }

    if(vm->error_description)
    {
        MEM_FREE(vm->error_description);
    }

    /* the string registers */
    for(i=0; i<REGISTER_COUNT; i++)
    {
        MEM_FREE(vm->regs[i]);
    }

    /* and the VM itself */
    MEM_FREE(vm);
}

struct nap_vm* nap_vm_inject(uint8_t* bytecode, int bytecode_len, enum environments target)
{
    struct nap_vm* vm = NULL;
    uint8_t* cloc = bytecode;

    vm = (struct nap_vm*)(calloc(1, sizeof(struct nap_vm)));
    if(vm == NULL)
    {
        /* TODO: provide a mechanism for error reporting */
        return NULL;
    }

    vm->content = (uint8_t *) calloc(sizeof(uint8_t), bytecode_len);
    if(vm->content == NULL)
    {
        /* TODO: provide a mechanism for error reporting */
        free(vm);
        return NULL;
    }

    /* create the stack */
    vm->stack_size = STACK_INIT;
    vm->stack_pointer = 0;
    vm->stack = (struct stack_entry**)calloc(sizeof(struct stack_entry*), STACK_INIT);
    if(vm->stack == NULL)
    {
        /* TODO: provide a mechanism for error reporting */
        free(vm->content);
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
    vm->cc = 1 + 4 * vm->file_bitsize + 1
            + 1; /* to point to the first instruction */

    /* initially the last boolean flag is in an unknow state */
    vm->lbf = UNDECIDED;
    vm->btyecode_chunks = (struct nap_bytecode_chunk**)calloc(MAX_BYTECODE_CHUNKS,
                                            sizeof(struct nap_bytecode_chunk*));
    vm->chunk_counter = 0;
    vm->allocated_chunks = 255;

    /* and setting the interrupts */
    vm->interrupts[2] = intr_2;
    vm->interrupts[3] = intr_3;
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
    vm->opcode_handlers[OPCODE_INTR] = nap_handle_interrupt; vm->opcode_error_codes[OPCODE_INTR] = ERR_VM_0017;
    vm->opcode_handlers[OPCODE_CLIDX] = nap_clidx; vm->opcode_error_codes[OPCODE_CLIDX] = 0;
    vm->opcode_handlers[OPCODE_LEAVE] = nap_leave; vm->opcode_error_codes[OPCODE_LEAVE] = ERR_VM_0021;

    /* setting the container type of the VM*/
    vm->environment = target;

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
        return NULL;
    }

    /* read in all the data in memory. Should be faster */
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    file_content = (uint8_t *) calloc(sizeof(uint8_t), fsize);
    /* TODO: check memory */

    fseek(fp, 0, SEEK_SET);
    fread(file_content, sizeof(uint8_t ), fsize, fp);

    fclose(fp);

    vm = nap_vm_inject(file_content, fsize, STANDALONE);
    MEM_FREE(file_content);

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

nap_byte_t nap_read_byte(struct nap_vm* vm)
{
    uint8_t imm_size = vm->content[vm->cc ++];
    nap_byte_t nr = 0;
    /* and now read the number according to the size */
    if(imm_size == OPCODE_BYTE)
    {
        nap_byte_t* immediate = (nap_byte_t*)(vm->content + vm->cc);
        nr = *immediate;
        vm->cc ++;
    }
    else
    {
        printf("invalid mov into byte size [mov]: 0x%x", imm_size);
        _NOT_IMPLEMENTED
    }

    return nr;
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
static nap_byte_t* regb_stack[DEEPEST_RECURSION] = {0};
static nap_index_t regi_stack_idx = 0;
static nap_index_t regb_stack_idx = 0;

int nap_save_registers(struct nap_vm* vm)
{
    nap_int_t* tmp_ints = NULL;
    nap_byte_t* tmp_bytes = NULL;

    /* save the int registers */
    tmp_ints = calloc(REGISTER_COUNT, sizeof(nap_int_t));
    memcpy(tmp_ints, vm->regi, vm->mrc * sizeof(nap_int_t));
    regi_stack[regi_stack_idx] = tmp_ints;
    regi_stack_idx ++;
    if(regi_stack_idx == DEEPEST_RECURSION)
    {
        return NAP_FAILURE;
    }

    /* save the byte registers */
    tmp_bytes = calloc(REGISTER_COUNT, sizeof(nap_byte_t));
    memcpy(tmp_bytes, vm->regb, vm->mrc * sizeof(nap_byte_t));
    regb_stack[regb_stack_idx] = tmp_bytes;
    regb_stack_idx ++;
    if(regb_stack_idx == DEEPEST_RECURSION)
    {
        return NAP_FAILURE;
    }

    return NAP_SUCCESS;
}

int nap_restore_registers(struct nap_vm* vm)
{
    if(regi_stack_idx == 0 || regb_stack_idx == 0)
    {
        return NAP_FAILURE;
    }

    /* restore the int registers */
    regi_stack_idx --;
    memcpy(vm->regi, regi_stack[regi_stack_idx], vm->mrc * sizeof(nap_int_t));
    MEM_FREE(regi_stack[regi_stack_idx]);

    /* restore the byte registers */
    regb_stack_idx --;
    memcpy(vm->regb, regb_stack[regb_stack_idx], vm->mrc * sizeof(nap_byte_t));
    MEM_FREE(regb_stack[regb_stack_idx]);

    return NAP_SUCCESS;
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
    uint8_t intr = *(uint8_t*)(vm->content + vm->cc);
    uint8_t int_res = 0;
	char* s = NULL;

    if(vm->interrupts[intr])
    {
        int_res = (vm->interrupts[intr])(vm);
    }
    else
    {
        /* unimplemented interrupt, reporting an error */
        char* s = (char*)calloc(64, sizeof(char));
        sprintf(s, "unimplemented interrupt: %d", intr);
        vm->error_description = s;
        return NAP_FAILURE;
    }
    /* advance to the next position */
    vm->cc ++;
    if(int_res == 0)
    {
        return NAP_SUCCESS;
    }
    s = (char*)calloc(64, sizeof(char));
    sprintf(s, "interrupt [%d] failure code [%d]", intr, int_res);
    vm->error_description = s;
    return NAP_FAILURE;
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
    return NULL;
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
        case STACK_ENTRY_CHAR : return "char";
        case STACK_ENTRY_MARKER : return "mark";
        case STACK_ENTRY_IMMEDIATE_INT : return "imm_int";
        case STACK_ENTRY_MARKER_NAME : return "mark_name";
        default: return "unk";
    }
}

char *convert_string_from_bytecode_file(const char *src, size_t len, size_t dest_len, size_t* real_len)
{
    char* loc_orig = 0; /* the original locale string for LC_ALL */
    int len_loc = 0;    /* the length of the loc_orig */
    char* enc = 0;      /* the actual encoding */
    char* loc_cp = 0;   /* copying over the loc_orig to not to mess with orig */
    char* converted = 0; /* will be used by iconv */
    char* orig_converted = 0; /* will have the beginning of the converted */
    iconv_t converter;  /* the converter itself */
    int ret = -1;       /* errorchecking */
    size_t save_dest_len = dest_len; /* used in real lenth calcualtions */
    char* to_return = NULL; /* what to return */
	char* final_encoding = NULL;
    char* src_copy = (char*)calloc(len + 1, sizeof(char));
    char* src_copy_save = src_copy;

    /*copy the src*/
    memcpy(src_copy, src, len);

    /* get the locale info */
    setlocale(LC_ALL, "");
    loc_orig = setlocale(LC_ALL, NULL);
    len_loc = strlen(loc_orig);
    loc_cp = (char*)calloc(len_loc + 1, sizeof(char));
    strcpy(loc_cp, loc_orig);

    /* the encoding of the locale */
    enc = strchr(loc_cp, '.') + 1;

#ifdef _WINDOWS
	final_encoding = (char*)calloc(8 + 1 + strlen(enc), sizeof(char)); /* WINDOWS- */
	strcpy(final_encoding, "WINDOWS-");
	strcat(final_encoding, enc);
#else
	final_encoding = enc;
#endif

    /* creating an iconv converter */
	converter = iconv_open(final_encoding, "UTF-32BE");
    if((size_t)converter == (size_t)-1)
    {
        free(loc_cp);
        if (errno == EINVAL)
        {
            fprintf(stderr, "[iconv] Conversion to %s is not supported\n", enc);
        }
        else
        {
            fprintf(stderr, "[iconv] Initialization failure");
        }
        exit(1);
    }

    /* creating the work buffer for iconv */
    converted = (char*)calloc(dest_len, sizeof(char));
    orig_converted = converted;

    /* converting */
    ret = iconv(converter, &src_copy, &len, &converted, &dest_len);
    iconv_close(converter);

#ifdef _WINDOWS
	free(final_encoding);
#endif

    free(loc_cp);

    if(ret == -1)
    {
        perror("iconv");
        switch(errno)
        {
        case EILSEQ:
            fprintf(stderr,  "EILSEQ\n");
            break;
        case EINVAL:
            fprintf(stderr,  "EINVAL\n");
            break;
        case E2BIG:
            fprintf(stderr,  "E2BIG\n");
            break;
        }

        free(orig_converted);
        free(src_copy_save);
        return NULL;
    }

    /* calculating the length of the converted string */
    *real_len = save_dest_len - dest_len;

    /* and what to return */
    to_return = (char*)calloc(*real_len + 1, sizeof(char));
    strncpy(to_return, orig_converted, *real_len);

    free(orig_converted);
    free(src_copy_save);
    return to_return;
}
