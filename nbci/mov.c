#include "mov.h"
#include "opcodes.h"
#include "stack.h"
#include "metatbl.h"

#include <stdlib.h>
#include <string.h>

void nap_mov(struct nap_vm* vm)
{
    uint8_t mov_target = vm->content[vm->cc ++];   /* where we move (reg, var)*/

    if(mov_target == OPCODE_REG) /* do we move in a register? */
    {
        uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
            uint8_t move_source = vm->content[vm->cc ++]; /* what are we moving in*/

            if(move_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
            {
                uint8_t imm_size = vm->content[vm->cc ++];
                /* and now write the number avm->ccording to the size */
                if(imm_size == OPCODE_BYTE)
                {
                    int8_t* immediate = (int8_t*)(vm->content + vm->cc);
                    vm->regi[register_index] = *immediate;
                    vm->cc ++;
                }
                else
                if(imm_size == OPCODE_SHORT)
                {
                    int16_t* immediate = (int16_t*)(vm->content + vm->cc);
                    vm->regi[register_index] = *immediate;
                    vm->cc += 2;
                }
                else
                if(imm_size == OPCODE_LONG)
                {
                    int32_t* immediate = (int32_t*)(vm->content + vm->cc);
                    vm->regi[register_index] = *immediate;
                    vm->cc += 4;
                }
                else
                if(imm_size == OPCODE_HUGE)
                {
                    int64_t* immediate = (int64_t*)(vm->content + vm->cc);
                    vm->regi[register_index] = *immediate;
                    vm->cc += 8;
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
                nap_index_t var_index = nap_fetch_index(vm);

                /* fetch the variable from the given index */
                struct variable_entry* var = vm->metatable[var_index];
                if(var->instantiation == 0)
                {
                    fprintf(stderr, "variable [%s] not initialised correctly\n", var->name);
                    exit(3);
                }

                if(var->instantiation->type != STACK_ENTRY_INT)
                {
                    fprintf(stderr, "variable [%s] has wrong type\n", var->name);
                    exit(4);
                }

                /* and moving the value in the regsiter itself */
                vm->regi[register_index] = *(nap_number_t*)var->instantiation->value;

            }
            else
            {
                _NOT_IMPLEMENTED
            }
        }
        else
        if(register_type == OPCODE_STRING)
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
            uint8_t move_source = vm->content[vm->cc ++]; /* what are we moving in*/
            if(move_source == OPCODE_STRING) /* usually we move an immediate string intro string register*/
            {
                nap_index_t str_index = nap_fetch_index(vm);

                /* since this is a simple move operation we are not
                   allocating the memory, since the stringtable
                   should always be the same, never will be freed
                   till we exit */
                vm->regs[register_index] = vm->stringtable[str_index]->string;
            }
            else
            {
                _NOT_IMPLEMENTED
            }
        }
        else
        if(register_type == OPCODE_IDX)
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
            uint8_t move_source = vm->content[vm->cc ++]; /* the index definition */
            if(move_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
            {
                uint8_t imm_size = vm->content[vm->cc ++];
                /* and now write the number avm->ccording to the size */
                if(imm_size == OPCODE_BYTE)
                {
                    int8_t* immediate = (int8_t*)(vm->content + vm->cc);
                    vm->regidx[register_index] = *immediate;
                    vm->cc ++;
                }
                else
                if(imm_size == OPCODE_SHORT)
                {
                    int16_t* immediate = (int16_t*)(vm->content + vm->cc);
                    vm->regidx[register_index] = *immediate;
                    vm->cc += 2;
                }
                else
                if(imm_size == OPCODE_LONG)
                {
                    int32_t* immediate = (int32_t*)(vm->content + vm->cc);
                    vm->regidx[register_index] = *immediate;
                    vm->cc += 4;
                }
                else
                if(imm_size == OPCODE_HUGE)
                {
                    int64_t* immediate = (int64_t*)(vm->content + vm->cc);
                    vm->regidx[register_index] = *immediate;
                    vm->cc += 8;
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
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* var = vm->metatable[var_index];
        uint8_t move_source = 0;

        /* first time usage of this variable? */
        if(var->instantiation == 0)
        {
            fprintf(stderr,
                    "using variable [%s] without being on stack\n",
                    var->name);
            exit(6);
        }

        /* and now let's see what we move in the variable */
        move_source = vm->content[vm->cc ++];

        /* moving a register in a variable? */
        if(move_source == OPCODE_REG)
        {
            uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/

            /* we are dealing with an INT type register */
            if(register_type == OPCODE_INT)
            {
                /* to check if the variable is the same type. If not, convert */
                if(var->instantiation->type == OPCODE_INT)
                {
                    /* perform the operation only if the values are not the same already*/
                    if(var->instantiation->value)
                    {
                        if(*(nap_number_t*)var->instantiation->value != vm->regi[register_index])
                        {
                            *(nap_number_t*)var->instantiation->value = vm->regi[register_index];
                        }
                    }
                    else /* allocate the memory for the value */
                    {
                        nap_number_t* temp = (nap_number_t*)calloc(1, sizeof(nap_number_t));
                        *temp = vm->regi[register_index];
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
                        if(strcmp((char*)var->instantiation->value, vm->regs[register_index]))
                        {
                            char* temp = (char*)calloc(strlen(vm->regs[register_index]) + 1, sizeof(char));
                            strcpy(temp, vm->regs[register_index]);
                            free(var->instantiation->value);
                            var->instantiation->value = temp;
                            var->instantiation->len = strlen(vm->regs[register_index]);
                        }
                    }
                    else /* allocate the memory for the value */
                    {
                        char* temp = (char*)calloc(strlen(vm->regs[register_index]), sizeof(char));
                        strcpy(temp, vm->regs[register_index]);
                        var->instantiation->value = temp;
                        var->instantiation->len = strlen(vm->regs[register_index]);
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
        uint8_t ccidx_target = vm->content[vm->cc ++];  /* should be a variable */
        if(ccidx_target == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            struct variable_entry* var = vm->metatable[var_index];
            uint8_t ctr_used_index_regs = 0;
            uint8_t move_src = 0;

            /* first time usage of this variable? */
            if(var->instantiation == 0)
            {
                fprintf(stderr,
                        "using variable [%s] without being on stack\n",
                        var->name);
                exit(6);
            }

            /* now should come the index reg counter of vm->ccidx,
               a simple byte since there are max 256 indexes */
            ctr_used_index_regs = vm->content[vm->cc ++];

            /* and find what is moved into this vm->ccidx destination*/
            move_src = vm->content[vm->cc ++];
            if(move_src == OPCODE_REG)
            { /* moving a register in the indexed destination */
                uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/
                uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
                if(register_type == OPCODE_STRING)
                { /* moving a string register into a variable at a specific location */
                    if(var->instantiation->type == OPCODE_STRING)
                    { /* this is a string, avm->ccessing a character from it:
                         so calculate the "real" index ofthe variable based
                         on the regidx vector and ctr_used_index_regs
                       */
                        uint64_t real_index = 0;
                        int i;
                        for(i=0; i<ctr_used_index_regs; i++)
                        {
                            /* first step: calculate the deplasation to find the "row"
                               this is actually the size of the "matrix".
                               TODO: The 1 should be replaced by something meaningful
                             */

                            real_index += real_index * 1;

                            /* then the column */
                            real_index += vm->regidx[i];
                        }

                        if(real_index + strlen(vm->regs[register_index]) > strlen((char*)var->instantiation->value))
                        {
                            fprintf(stderr,
                                    "Index overflow error for [%s]. Requested index: [%" PRIu64 "] Available length: [%ld] Assumed length: [%" PRIu64 "]\n",
                                    var->name, real_index, strlen((char*)var->instantiation->value), real_index + strlen(vm->regs[register_index]));
                            exit(18);
                        }
                        /* and finally do a strcpy */
                        strncpy((char*)var->instantiation->value + real_index,
                                vm->regs[register_index],
                                strlen(vm->regs[register_index]));

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
