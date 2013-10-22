#include "opcodes.h"
#include "strtable.h"
#include "metatbl.h"
#include "jmptable.h"
#include "nbci.h"
#include "stack.h"

#include "push.h"
#include "comparison.h"
#include "mov.h"

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
            if(vm->metatable[i]->instantiation->value)
            {
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_INT)
                {
                    fprintf(fp, "E:[%s=%" PRId64 "](%" PRIu64 "/%" PRIu64 ")\n",
                           vm->metatable[i]->name,
                           *(int64_t*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);
                }
                else
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_STRING)
                {
                    printf("E:[%s=%s](%" PRIu64 "/%" PRIu64 ")\n",
                           vm->metatable[i]->name,
                           (char*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);
                }
                else
                {
                    printf("X:[%s=%"PRId64"](%" PRIu64 "/%" PRIu64 ")\n",
                           vm->metatable[i]->name,
                           *(int64_t*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);

                }
            }
            else
            {
                printf("N:[%s=??](%" PRIu64 "/%" PRIu64 ")\n", vm->metatable[i]->name,
                       i, vm->meta_size);

            }
        }
        else
        {
            printf("?:[%s=??](%"PRIu64"/%"PRIu64")\n", vm->metatable[i]->name,
                   i, vm->meta_size);
        }
    }
}

void nap_vm_run(struct nap_vm *vm)
{
    /* and start interpreting the code */
    while(vm->cc < vm->meta_location)
    {
        vm->current_opcode = vm->content[vm->cc];
        vm->cc ++;

        /* is this a PUSH operation? */
        if(vm->current_opcode == OPCODE_PUSH)
        {
            nap_push(vm);
        }
        else
        /* is this checking for something? */
        if(vm->current_opcode == OPCODE_EQ
                || vm->current_opcode == OPCODE_LT
                || vm->current_opcode == OPCODE_GT
                || vm->current_opcode == OPCODE_NEQ
                || vm->current_opcode == OPCODE_LTE
                || vm->current_opcode == OPCODE_GTE)
        {
            nap_comparison(vm);
        }
        else
        /* is this a MOV operation? */
        if(vm->current_opcode == OPCODE_MOV)
        {
            nap_mov(vm) ;
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

}


/*
 * Main entry point
 */
int main()
{
    struct nap_vm* vm = nap_vm_load("test.ncb");
    if(!vm)
    {
        exit(1);
    }

    return 0;
}

