#include "opcodes.h"
#include "strtable.h"
#include "metatbl.h"
#include "jmptable.h"
#include "nbci.h"
#include "stack.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define __STDC_FORMAT_MACROS
#ifndef _WIN32
#include <inttypes.h>
#else
#define PRId64 "lld"
#define PRIu64 "lld"
#define PRIx64 "x"
#endif

/******************************************************************************/
/*                             Debugging section                              */
/******************************************************************************/

#define _NOT_IMPLEMENTED \
    fprintf(stderr, "NI: line [%d] instr [%d] opcode [%x] at %" PRIu64 " (%" PRIx64 ")\n\n", \
            __LINE__, vm->content[vm->cc - 1], vm->current_opcode, vm->cc - 1, vm->cc - 1); \
    exit(99);


/* generic variables regarding the file */
uint8_t file_bitsize = 0;                   /* the bit size: 0x32, 0x64*/


void dump(struct nap_vm* vm, FILE *fp)
{
    uint64_t i;
    puts("");
    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(metatable[i]->instantiation->value)
            {
                if(metatable[i]->instantiation->type == STACK_ENTRY_INT)
                {
                    fprintf(fp, "E:[%s=%" PRId64 "](%" PRIu64 "/%" PRIu64 ")\n",
                           metatable[i]->name,
                           *(int64_t*)(metatable[i]->instantiation->value)
                           ,i, meta_size);
                }
                else
                if(metatable[i]->instantiation->type == STACK_ENTRY_STRING)
                {
                    printf("E:[%s=%s](%" PRIu64 "/%" PRIu64 ")\n",
                           metatable[i]->name,
                           (char*)(metatable[i]->instantiation->value)
                           ,i, meta_size);
                }
                else
                {
                    printf("X:[%s=%"PRId64"](%" PRIu64 "/%" PRIu64 ")\n",
                           metatable[i]->name,
                           *(int64_t*)(metatable[i]->instantiation->value)
                           ,i, meta_size);

                }
            }
            else
            {
                printf("N:[%s=??](%" PRIu64 "/%" PRIu64 ")\n", metatable[i]->name,
                       i, meta_size);

            }
        }
        else
        {
            printf("?:[%s=??](%"PRIu64"/%"PRIu64")\n", metatable[i]->name,
                   i, meta_size);
        }
    }
}


/*
 * Main entry point
 */
int main()
{
    FILE* fp = fopen("test.ncb", "rb");
	uint64_t fsize = 0;
    uint32_t meta_location = 0;
    uint32_t stringtable_location = 0;
    uint32_t jumptable_location = 0;
    uint8_t type = 0;

	if(!fp) exit(1);
    fseek(fp, 0, SEEK_END);
    
    /* read in all the data in memory. Should be faster */
    fsize = ftell(fp);
    content = (uint8_t *) calloc(sizeof(uint8_t), fsize);
    fseek(fp, 0, SEEK_SET);
    fread(content, sizeof(uint8_t ), fsize, fp);
    
    fseek(fp, 0, SEEK_SET);
    
    /* create the stack */
    stack = (struct stack_entry**) calloc( 
                                        sizeof(struct stack_entry*), stack_size
                                      );
    
    /* the format of the addresses in the file 32 or 64 bit addresses */
    fread(&type, sizeof(uint8_t), 1, fp);
    if(type == 0x32)
    {
        file_bitsize = sizeof(uint32_t);
    }
    else
    {
        file_bitsize = sizeof(uint64_t);
    }

    /* read in the important addresses from the bytecode file*/
    fread(&meta_location, file_bitsize, 1, fp);
    fread(&stringtable_location, file_bitsize, 1, fp);
    fread(&jumptable_location, file_bitsize, 1, fp);
    
    /* prepare the meta table of the application */
    read_metatable(fp, meta_location);

    /* read the stringtable */
    read_stringtable(fp, stringtable_location);

    /* read the jumptable */
    read_jumptable(fp, jumptable_location);
    
    /* done with the file */
    fclose(fp);
    
    /* cc is the instruction pointer: skip the 3 addresses and the startbyte */
    cc = 3 * file_bitsize + 1;
    
    /* and start interpreting the code */
    while(cc < meta_location)
    {
        current_opcode = content[cc];
        cc ++;

        /* is this a PUSH operation? */
        if(current_opcode == OPCODE_PUSH)
        {
            struct stack_entry* se = (struct stack_entry*)(
                                        calloc(sizeof(struct stack_entry), 1));
			se->type = (StackEntryType)content[cc ++];

            if(se->type == OPCODE_INT || se->type == OPCODE_STRING) /* or float*/
            {
                uint8_t push_what = content[cc ++];

                if(push_what == OPCODE_VAR)
                {
                    uint32_t* p_var_index = (uint32_t*)(content + cc);
                    struct variable_entry* ve = metatable[*p_var_index];

					cc += sizeof(uint32_t);

                    ve->instantiation = (struct stack_entry*)(calloc(sizeof(struct stack_entry), 1));
                    ve->instantiation->type = se->type; /* must match the stack entry */

                    if(se->type == OPCODE_INT) /* pushing an integer */
                    {
                        int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                        *temp = 0;
                        ve->instantiation->value = temp;
                    }
                    else
                    if(se->type == OPCODE_STRING) /* pushing a string */
                    {
                        char* temp = (char*)calloc(1, sizeof(char));
                        *temp = 0;
                        ve->instantiation->value = temp;
                    }

                    /* setting the value of the stack entry */
                    se->value = ve;
                }
                else
                {
                    fprintf(stderr, "unknown push [push int 0x%x]", push_what);
                    exit(66);
                }
            }
            else
            if(se->type == OPCODE_REG) /* pushing a register */
            {
                uint8_t reg_type = content[cc ++];
                uint8_t reg_idx = content[cc ++];

                if(reg_type == OPCODE_INT) /* pushing an int register */
                {
                    int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                    *temp = regi[reg_idx];

                    /* setting the value of the stack entry */
                    se->value = temp;
                }
                else
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            {
                fprintf(stderr, "not implemented push destination: [0x%x]\n",
                        se->type);
                exit(10);
            }
                
            stack[++ stack_pointer] = se;
        }
        else
        /* is this checking for something? */
        if(current_opcode == OPCODE_EQ
                || current_opcode == OPCODE_LT
                || current_opcode == OPCODE_GT
                || current_opcode == OPCODE_NEQ
                || current_opcode == OPCODE_LTE
                || current_opcode == OPCODE_GTE)
        {
            uint8_t mov_target = content[cc ++];   /* what to check (reg only)*/

            if(mov_target == OPCODE_REG) /* do we move in a register? */
            {
                uint8_t register_type = content[cc ++]; /* int/string/float...*/

                /* we are dealing with an INT type register */
                if(register_type == OPCODE_INT)
                {
                    uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/
                    uint8_t move_source = content[cc ++]; /* what are we checking against*/

                    if(move_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
                    {
                        uint8_t imm_size = content[cc ++];
                        if(imm_size == OPCODE_BYTE)
                        {
                            int8_t* immediate = (int8_t*)(content + cc);
                            set_lbf_to_op_result(regi[register_index],
                                                    *immediate,
                                                    current_opcode);
                            cc ++;
                        }
                        else
                        if(imm_size == OPCODE_SHORT)
                        {
                            int16_t* immediate = (int16_t*)(content + cc);
                            set_lbf_to_op_result(regi[register_index],
                                                    *immediate,
                                                    current_opcode);
                            cc += 2;
                        }
                        else
                        if(imm_size == OPCODE_LONG)
                        {
                            int32_t* immediate = (int32_t*)(content + cc);
                            set_lbf_to_op_result(regi[register_index],
                                                    *immediate,
                                                    current_opcode);
                            cc += 4;
                        }
                        else
                        if(imm_size == OPCODE_HUGE)
                        {
                            int64_t* immediate = (int64_t*)(content + cc);
                            set_lbf_to_op_result(regi[register_index],
                                                    *immediate,
                                                    current_opcode);
                            cc += 8;
                        }
                        else
                        {
                            printf("invalid immediate size [cmp]: 0x%x", imm_size);
                            exit(12);
                        }
                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }
                }
                else
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            {
                fprintf(stderr, "eq works only on registers\n");
                exit(8);
            }
        }
        else
        /* is this a MOV operation? */
        if(current_opcode == OPCODE_MOV)
        {
            uint8_t mov_target = content[cc ++];   /* where we move (reg, var)*/

            if(mov_target == OPCODE_REG) /* do we move in a register? */
            {
                uint8_t register_type = content[cc ++]; /* int/string/float...*/

                /* we are dealing with an INT type register */
                if(register_type == OPCODE_INT)
                {
                    uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/
                    uint8_t move_source = content[cc ++]; /* what are we moving in*/

                    if(move_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
                    {
                        uint8_t imm_size = content[cc ++];
                        /* and now write the number according to the size */
                        if(imm_size == OPCODE_BYTE)
                        {
                            int8_t* immediate = (int8_t*)(content + cc);
                            regi[register_index] = *immediate;
                            cc ++;
                        }
                        else
                        if(imm_size == OPCODE_SHORT)
                        {
                            int16_t* immediate = (int16_t*)(content + cc);
                            regi[register_index] = *immediate;
                            cc += 2;
                        }
                        else
                        if(imm_size == OPCODE_LONG)
                        {
                            int32_t* immediate = (int32_t*)(content + cc);
                            regi[register_index] = *immediate;
                            cc += 4;
                        }
                        else
                        if(imm_size == OPCODE_HUGE)
                        {
                            int64_t* immediate = (int64_t*)(content + cc);
                            regi[register_index] = *immediate;
                            cc += 8;
                        }
                        else
                        {
                            printf("invalid immediate size [mov]: 0x%x", imm_size);
                            exit(13);
                        }
                    }
                    else
                    if(move_source == OPCODE_VAR) /* movin a variable into reg*/
                    {
                        uint32_t* p_var_index = (uint32_t*)(content + cc);

                        /* fetch the variable from the given index */
                        struct variable_entry* var = metatable[*p_var_index];
                        if(var->instantiation == 0)
                        {
                            fprintf(stderr,
                                    "variable %s not initialised correctly\n",
                                    var->name);
                            exit(3);
                        }

                        if(var->instantiation->type != STACK_ENTRY_INT)
                        {
                            fprintf(stderr,
                                    "variable %s has wrong type\n",
                                    var->name);
                            exit(4);
                        }

                        /* and moving the value in the regsiter itself */
                        regi[register_index] = *(int64_t*)var->instantiation->value;

                        /* forwarding the instructions pointer to next bytecode*/
                        cc += sizeof(uint32_t);
                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }
                }
                else
                if(register_type == OPCODE_STRING)
                {
                    uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/
                    uint8_t move_source = content[cc ++]; /* what are we moving in*/
                    if(move_source == OPCODE_STRING) /* usually we move an immediate string intro string register*/
                    {
                        uint32_t* p_str_index = (uint32_t*)(content + cc);
                        cc += sizeof(uint32_t);

                        /* since this is a simple move operation we are not
                           allocating the memory, since the stringtable
                           should always be the same, never will be freed
                           till we exit */
                        regs[register_index] = stringtable[*p_str_index]->string;
                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }
                }
                else
                if(register_type == OPCODE_IDX)
                {
                    uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/
                    uint8_t move_source = content[cc ++]; /* the index definition */
                    if(move_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
                    {
                        uint8_t imm_size = content[cc ++];
                        /* and now write the number according to the size */
                        if(imm_size == OPCODE_BYTE)
                        {
                            int8_t* immediate = (int8_t*)(content + cc);
                            regidx[register_index] = *immediate;
                            cc ++;
                        }
                        else
                        if(imm_size == OPCODE_SHORT)
                        {
                            int16_t* immediate = (int16_t*)(content + cc);
                            regidx[register_index] = *immediate;
                            cc += 2;
                        }
                        else
                        if(imm_size == OPCODE_LONG)
                        {
                            int32_t* immediate = (int32_t*)(content + cc);
                            regidx[register_index] = *immediate;
                            cc += 4;
                        }
                        else
                        if(imm_size == OPCODE_HUGE)
                        {
                            int64_t* immediate = (int64_t*)(content + cc);
                            regidx[register_index] = *immediate;
                            cc += 8;
                        }
                        else
                        {
                            printf("invalid immediate size [mov]: 0x%x", imm_size);
                            exit(13);
                        }
                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }

                }
                else
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            if(mov_target == OPCODE_VAR) /* we move into a variable */
            {
                uint32_t* p_var_index = (uint32_t*)(content + cc);
                struct variable_entry* var = metatable[*p_var_index];
				uint8_t move_source = 0;

                cc += sizeof(uint32_t);

                /* first time usage of this variable? */
                if(var->instantiation == 0)
                {
                    fprintf(stderr,
                            "using variable [%s] without being on stack\n",
                            var->name);
                    exit(6);
                }

                /* and now let's see what we move in the variable */
                move_source = content[cc ++];

                /* moving a register in a variable? */
                if(move_source == OPCODE_REG)
                {
                    uint8_t register_type = content[cc ++]; /* int/string/float...*/
                    uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/

                    /* we are dealing with an INT type register */
                    if(register_type == OPCODE_INT)
                    {
                        /* to check if the variable is the same type. If not, convert */
                        if(var->instantiation->type == OPCODE_INT)
                        {
                            /* perform the operation only if the values are not the same already*/
                            if(var->instantiation->value)
                            {
                                if(*(int64_t*)var->instantiation->value != regi[register_index])
                                {
                                    int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                                    *temp = regi[register_index];
                                    free(var->instantiation->value);
                                    var->instantiation->value = temp;
                                }
                            }
                            else /* allocate the memory for the value */
                            {
                                int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                                *temp = regi[register_index];
                                var->instantiation->value = temp;
                            }
                        }
                        else
                        { /* here: convert the value to hold the requested type */
                            _NOT_IMPLEMENTED
                        }
                    }
                    else
                    if(register_type == OPCODE_STRING)
                    {
                        if(var->instantiation->type == OPCODE_STRING)
                        {
                            /* moving a register into the string variable */
                            if(var->instantiation->value)
                            {
                                /* copy only if they are not the same */
                                if(strcmp((char*)var->instantiation->value, regs[register_index]))
                                {
                                    char* temp = (char*)calloc(strlen(regs[register_index]) + 1, sizeof(char));
                                    strcpy(temp, regs[register_index]);
                                    free(var->instantiation->value);
                                    var->instantiation->value = temp;
                                    var->instantiation->len = strlen(regs[register_index]);
                                }
                            }
                            else /* allocate the memory for the value */
                            {
                                char* temp = (char*)calloc(strlen(regs[register_index]), sizeof(char));
                                strcpy(temp, regs[register_index]);
                                var->instantiation->value = temp;
                                var->instantiation->len = strlen(regs[register_index]);
                            }
                        }
                        else
                        {
                            _NOT_IMPLEMENTED
                        }
                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }
                }
                else
                {
                    fprintf(stderr, "only register can be moved to var [%s]\n",
                            var->name);
                    exit(5);
                }
            }
            else
            if(mov_target == OPCODE_CCIDX)
            {
                uint8_t ccidx_target = content[cc ++];  /* should be a variable */
                if(ccidx_target == OPCODE_VAR)
                {
                    uint32_t* p_var_index = (uint32_t*)(content + cc);
                    struct variable_entry* var = metatable[*p_var_index];
					uint8_t ctr_used_index_regs = 0;
                    uint8_t move_src = 0;
					cc += sizeof(uint32_t);

                    /* first time usage of this variable? */
                    if(var->instantiation == 0)
                    {
                        fprintf(stderr,
                                "using variable [%s] without being on stack\n",
                                var->name);
                        exit(6);
                    }

                    /* now should come the index reg counter of ccidx,
                       a simple byte since there are max 256 indexes */
                    ctr_used_index_regs = content[cc ++];

                    /* and find what is moved into this ccidx destination*/
                    move_src = content[cc ++];
                    if(move_src == OPCODE_REG)
                    { /* moving a register in the indexed destination */
                        uint8_t register_type = content[cc ++]; /* int/string/float...*/
                        uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/
                        if(register_type == OPCODE_STRING)
                        { /* moving a string register into a variable at a specific location */
                            if(var->instantiation->type == OPCODE_STRING)
                            { /* this is a string, accessing a character from it:
                                 so calculate the "real" index ofthe variable based
                                 on the regidx vector and ctr_used_index_regs
                               */
                                int real_index = 0;
                                int i;
                                for(i=0; i<ctr_used_index_regs; i++)
                                {
                                    /* first step: calculate the deplasation to find the "row"
                                       this is actually the size of the "matrix".
                                       TODO: The 1 should be replaced by something meaningful
                                     */

                                    real_index += real_index * 1;

                                    /* then the column */
                                    real_index += regidx[i];
                                }

                                if(real_index + strlen(regs[register_index]) > strlen((char*)var->instantiation->value))
                                {
                                    fprintf(stderr,
                                            "Index overflow error for [%s]. Requested index: [%d] Available length: [%ld] Assumed length: [%ld]\n",
                                            var->name, real_index, strlen((char*)var->instantiation->value), real_index + strlen(regs[register_index]));
                                    exit(18);
                                }
                                /* and finally do a strcpy */
                                strncpy((char*)var->instantiation->value + real_index,
                                        regs[register_index],
                                        strlen(regs[register_index]));

                            }
                            else
                            {
                                _NOT_IMPLEMENTED
                            }
                        }
                        else
                        {
                            _NOT_IMPLEMENTED
                        }
                    }
                    else
                    {
                        /* moving an immediate value/variable into an index destination */
                        _NOT_IMPLEMENTED
                    }
                }
                else
                {
                    /* moving into something other indexed, than a variable */
                    _NOT_IMPLEMENTED
                }
            }
            else
            {
                _NOT_IMPLEMENTED
            }
        }
        else
        /* jumping somewhere ? */
        if(current_opcode == OPCODE_JLBF || current_opcode == OPCODE_JMP)
        {
            uint32_t* p_jmpt_index = (uint32_t*)(content + cc);

            /* and simply set cc to be where we need to go */
            if(current_opcode == OPCODE_JMP)
            {
                cc = jumptable[*p_jmpt_index]->location;
            }
            else
            {
                if(lbf)
                {
                    lbf = 0;
                    cc = jumptable[*p_jmpt_index]->location;
                }
                else
                {
                    cc += sizeof(uint32_t);
                }
            }
        }
        else
        if(current_opcode == OPCODE_MARKS_NAME)
        {
			uint32_t* p_marker_code = NULL;
			int32_t* temp = NULL;
            struct stack_entry* marker = (struct stack_entry*)(
                        calloc(sizeof(struct stack_entry), 1));
            marker->type = STACK_ENTRY_MARKER_NAME;

            p_marker_code = (uint32_t*)(content + cc);
            cc += sizeof(uint32_t);

            temp = (int32_t*)calloc(1, sizeof(int32_t));
            *temp = *p_marker_code;

            marker->value = temp;

            stack[++ stack_pointer] = marker;
        }
        else
        /* giving up our territory till a given name */
        if(current_opcode == OPCODE_CLRS_NAME)
        {
            uint32_t* p_marker_code = (uint32_t*)(content + cc);
            cc += sizeof(uint32_t);

            while(stack_pointer > -1
                  && stack[stack_pointer]->type != STACK_ENTRY_MARKER_NAME
                  && stack[stack_pointer]->value
                  && *(uint32_t*)(stack[stack_pointer]->value) != *p_marker_code)
            {
                if(stack[stack_pointer]->type == OPCODE_REG
                        || stack[stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
                {
                    free(stack[stack_pointer]->value);
                }

                free(stack[stack_pointer]);
                stack_pointer --;
            }

            if(stack[stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
            {
                free(stack[stack_pointer]->value);
                free(stack[stack_pointer]);
                stack_pointer --;
            }
        }
        else
        /* calling someone */
        if(current_opcode == OPCODE_CALL)
        {
            uint32_t* p_jmpt_index = (uint32_t*)(content + cc);
            cc += sizeof(uint32_t);
            call_frames[cfsize ++] = cc;

            /* and simply set cc to be where we need to go */
            cc = jumptable[*p_jmpt_index]->location;
        }
        else
        if(current_opcode == OPCODE_PEEK)
        {
            uint8_t peek_type = content[cc ++]; /* int/string/float...*/

            /* we are dealing with an INT type peek */
            if(peek_type == OPCODE_INT)
            {
                uint8_t peek_index_type = content[cc ++]; /* what are we moving in*/
                uint32_t peek_index = 0;
				uint8_t peek_target = 0;

                if(peek_index_type == OPCODE_IMMEDIATE) /* immediate value (1,..) */
                {
                    uint8_t imm_size = content[cc ++];
                    /* and now write the number according to the size */
                    if(imm_size == OPCODE_BYTE)
                    {
                        int8_t* immediate = (int8_t*)(content + cc);
                        peek_index = *immediate;
                        cc ++;
                    }
                    else
                    if(imm_size == OPCODE_SHORT)
                    {
                        int16_t* immediate = (int16_t*)(content + cc);
                        peek_index = *immediate;
                        cc += 2;
                    }
                    else
                    if(imm_size == OPCODE_LONG)
                    {
                        int32_t* immediate = (int32_t*)(content + cc);
                        peek_index = *immediate;
                        cc += 4;
                    }
                    else
                    {
                        printf("invalid immediate size: 0x%x", imm_size);
                        exit(14);
                    }
                }
                else
                {
                    _NOT_IMPLEMENTED
                }

                /* now we know the peek index, see into what are we peeking */
                peek_target = content[cc ++];
                if(peek_target == OPCODE_VAR)
                {
                    uint32_t* p_var_index = (uint32_t*)(content + cc);
                    struct variable_entry* ve = metatable[*p_var_index];
                    cc += sizeof(uint32_t);

                    /* there is no instantioation at this point for the var */
                    ve->instantiation = (struct stack_entry*)(
                                calloc(sizeof(struct stack_entry), 1));
                    ve->instantiation->type = (StackEntryType)peek_type;
                    if(peek_type == OPCODE_INT)
                    {
                        int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                        *temp = *(int64_t*)stack[stack_pointer - peek_index]->value; /* STACK VALUE FROM peek_index */
                        ve->instantiation->value = temp;
                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }
                }
                else
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            {
                _NOT_IMPLEMENTED
            }
        }
        else
        if(current_opcode == OPCODE_POP)
        {
            uint8_t pop_what = content[cc ++];
            if(pop_what == OPCODE_REG)
            {
                uint8_t register_type = content[cc ++]; /* int/string/float...*/

                /* we are dealing with an INT type register */
                if(register_type == OPCODE_INT)
                {
                    uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/

                    if(stack[stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
                    {
                        /*obviously popping something when nothing was returned
                         from a misbehvaing function. Set the register to 0 */
                        regi[register_index] = 0;
                        /* Do not touch the stack for now.*/
                    }
                    else
                    {
                        /* check if they have the same type */
                        if(stack[stack_pointer]->type == STACK_ENTRY_INT)
                        {
                            regi[register_index] = *(int64_t*)stack[stack_pointer]->value;
                        }
                        else
                        if(stack[stack_pointer]->type == OPCODE_REG)
                        {
                            regi[register_index] = *(int64_t*)stack[stack_pointer]->value;
                        }
                        else /* default value */
                        {
                            regi[register_index] = 0;
                        }
                        free(stack[stack_pointer]->value);
                        free(stack[stack_pointer]);

                        stack_pointer --;
                    }

                }
                else
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            {
                _NOT_IMPLEMENTED
            }
        }
        else
        if(current_opcode == OPCODE_RETURN)
        {
            cc = call_frames[--cfsize];
        }
        else
        if(current_opcode == OPCODE_EXIT)
        {
            /* free the allocated metatable */
            cleanup();
            /* and finally leaving */
            exit(0);
        }
        else
        if(current_opcode == OPCODE_INC) /* increment */
        {
            uint8_t inc_what = content[cc ++]; /* variable, register*/
            if(inc_what == OPCODE_VAR)
            {
                uint32_t* p_var_index = (uint32_t*)(content + cc);
                struct variable_entry* ve = metatable[*p_var_index];
                cc += sizeof(uint32_t);
                if(ve->instantiation->type == OPCODE_INT)
                {
                    (*(int64_t*)ve->instantiation->value) ++;
                }
                else
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            {
                _NOT_IMPLEMENTED
            }
        }
        else
        if(current_opcode == OPCODE_DEC) /* decrement */
        {
            uint8_t inc_what = content[cc ++]; /* variable, register*/
            if(inc_what == OPCODE_VAR)
            {
                uint32_t* p_var_index = (uint32_t*)(content + cc);
                struct variable_entry* ve = metatable[*p_var_index];
                cc += sizeof(uint32_t);

                if(ve->instantiation->type == OPCODE_INT)
                {
                    (*(int64_t*)ve->instantiation->value) --;
                }
                else
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            {
                _NOT_IMPLEMENTED
            }
        }
        else
        if(current_opcode == OPCODE_CLIDX)
        {
            memset(regidx, 0, sizeof(regidx));
        }
        else
        /* is this a mathematical operation? */
        if(current_opcode == OPCODE_ADD
                || current_opcode == OPCODE_MUL
                || current_opcode == OPCODE_SUB
                || current_opcode == OPCODE_DIV
                || current_opcode == OPCODE_MOD )
        {
            uint8_t add_target = content[cc ++];   /* where we add (reg, var)*/

            if(add_target == OPCODE_REG) /* we add into a register? */
            {
                uint8_t register_type = content[cc ++]; /* int/string/float...*/

                /* we are dealing with an INT type register */
                if(register_type == OPCODE_INT)
                {
                    uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/
                    uint8_t add_source = content[cc ++]; /* what are we adding to it*/

                    if(add_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
                    {
                        uint8_t imm_size = content[cc ++];
                        if(imm_size == OPCODE_BYTE)
                        {
                            int8_t* immediate = (int8_t*)(content + cc);
                            do_operation(&regi[register_index], *immediate, current_opcode);
                            cc ++;
                        }
                        else
                        if(imm_size == OPCODE_SHORT)
                        {
                            int16_t* immediate = (int16_t*)(content + cc);
                            do_operation(&regi[register_index], *immediate, current_opcode);
                            cc += 2;
                        }
                        else
                        if(imm_size == OPCODE_LONG)
                        {
                            int32_t* immediate = (int32_t*)(content + cc);
                            do_operation(&regi[register_index], *immediate, current_opcode);
                            cc += 4;
                        }
                        else
                        if(imm_size == OPCODE_HUGE)
                        {
                            int64_t* immediate = (int64_t*)(content + cc);
                            do_operation(&regi[register_index], *immediate, current_opcode);
                            cc += 8;
                        }
                        else
                        {
                            printf("invalid immediate size [op]: 0x%x", imm_size);
                            exit(13);
                        }
                    }
                    else
                    if(add_source == OPCODE_VAR) /* adding a variable to a reg*/
                    {
                        uint32_t* p_var_index = (uint32_t*)(content + cc);

                        /* fetch the variable from the given index */
                        struct variable_entry* var = metatable[*p_var_index];
                        if(var->instantiation == 0)
                        {
                            fprintf(stderr,
                                    "variable %s not initialised correctly\n",
                                    var->name);
                            exit(3);
                        }

                        if(var->instantiation->type != STACK_ENTRY_INT)
                        {
                            fprintf(stderr,
                                    "variable %s has wrong type\n",
                                    var->name);
                            exit(4);
                        }

                        /* and moving the value in the regsiter itself */
                        do_operation(&regi[register_index], *(int64_t*)var->instantiation->value, current_opcode);

                        /* forwarding the instructions pointer to next bytecode*/
                        cc += sizeof(uint32_t);
                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }
                }
                else
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            if(add_target == OPCODE_VAR) /* we move into a variable */
            {
                uint32_t* p_var_index = (uint32_t*)(content + cc);
                uint8_t add_source = 0;
                struct variable_entry* var = metatable[*p_var_index];
				cc += sizeof(uint32_t);

                /* first time usage of this variable? */
                if(var->instantiation == 0)
                {
                    fprintf(stderr,
                            "using variable [%s] without being on stack\n",
                            var->name);
                    exit(6);
                }

                /* and now let's see what we move in the variable */
                add_source = content[cc ++];

                /* moving a register in a variable? */
                if(add_source == OPCODE_REG)
                {
                    uint8_t register_type = content[cc ++]; /* int/string/float...*/

                    /* we are dealing with an INT type register */
                    if(register_type == OPCODE_INT)
                    {
                        uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/

                        /* perform the operation only if the values are not the same already*/
                        if(var->instantiation->value)
                        {
                            int64_t* temp = (int64_t*)var->instantiation->value;
                            do_operation(temp, regi[register_index], current_opcode);
                        }
                        else /* allocate the memory for the value */
                        { /* this should generate some error, there should be a value before add */
                            int64_t* temp = (int64_t*)calloc(1, sizeof(int64_t));
                            *temp = regi[register_index];
                            var->instantiation->value = temp;
                        }
                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }
                }
                else
                {
                    fprintf(stderr, "only register can be added to var [%s]\n",
                            var->name);
                    exit(5);
                }
            }
            else
            {
                fprintf(stderr, "cannot add to a target\n");
                exit(9);
            }
        }
        else
        {
            fprintf(stderr, "invalid opcode [%x] at %"PRIu64" (%"PRIx64")\n",
                    current_opcode, cc - 1, cc - 1);
            cleanup();
            exit(5);

        }
    }

    return 0;
}