#include "mov.h"
#include "opcodes.h"
#include "stack.h"
#include "metatbl.h"
#include "nbci_impl.h"
#include "nap_consts.h"
#include "nbci.h"

#include <stdlib.h>
#include <string.h>

/* delviers the flat index according to the index regsiters of the vm for
 * the given variable */
static int64_t deliver_flat_index(struct nap_vm* vm,
                                   const struct variable_entry* ve,
                                   uint8_t used_indexes)
{
    int64_t to_ret = 0;
    int i = 0;

    /* moving block of arrays is not permitted yet :( */
    if(used_indexes != ve->dimension_count)
    {
        return -1;
    }

    for(; i<used_indexes; i++)
    {
        int j = 0;
        int64_t dim_multiplier = 1;

        if(vm->regidx[i] >= ve->dimensions[i])
        {
            return -2; /* an index overflow */
        }
        while(j < used_indexes - i - 1)
        {
            dim_multiplier *= ve->dimensions[j ++];
        }
        dim_multiplier *= vm->regidx[i];
        to_ret += dim_multiplier;
    }

    return to_ret;
}

/* returns a number from the given string */
static nap_int_t nap_int_string_to_number(const char* to_conv, size_t len)
{
    int base = 10;
    char* endptr = NULL;
    size_t dest_len = len * 4, real_len = 0;
    char* t = convert_string_from_bytecode_file((char*)to_conv,
                                                len * 4, dest_len, &real_len);
    char *save_t = t;
    if(!t)
    {
        return 0;
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
    nap_int_t v = strtoll(t, &endptr, base);;
    free(save_t);
    return v;
}

int nap_mov(struct nap_vm* vm)
{
    uint8_t mov_target = vm->content[vm->cc ++];   /* where we move (reg, var)*/

    if(mov_target == OPCODE_REG) /* do we move in a register? */
    {
        uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)  /* to handle mov reg int x, <something> */
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
            uint8_t move_source = vm->content[vm->cc ++]; /* what are we moving in*/

            if(move_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
            {
                vm->regi[register_index] = nap_read_immediate(vm);
            }
            else
            if(move_source == OPCODE_VAR) /* movin a variable into reg*/
            {
                nap_index_t var_index = nap_fetch_index(vm);
                /* and fetch the variable from the given index */
                struct variable_entry* var = nap_fetch_variable(vm, var_index);
                ASSERT_NOT_NULL_VAR(var)
                CHECK_VARIABLE_INSTANTIATON(var)
                CHECK_VARIABLE_TYPE(var, STACK_ENTRY_INT)

                /* and moving the value in the regsiter itself */
                vm->regi[register_index] = *(nap_int_t*)var->instantiation->value;
            }
            else
            if(move_source == OPCODE_RV)
            {
                uint8_t return_type = vm->content[vm->cc ++]; /* int/string/float...*/
                if(return_type == OPCODE_INT)                 /* handles: mov reg int 0, rv int*/
                {
                    vm->regi[register_index] = vm->rvi;
                }
                else
                if(return_type == OPCODE_STRING)              /* handles: mov reg int 0, rv string*/
                {
                    vm->regi[register_index] = nap_int_string_to_number(vm->rvs, vm->rvl);
                }
                else
                {
                    _NOT_IMPLEMENTED
                }
            }
            else
            if(move_source == OPCODE_REG)
            {
                uint8_t second_register_type = vm->content[vm->cc ++]; /* int/string/float...*/
                if(second_register_type == OPCODE_INT) /* moving an int register in another int register */
                {
                    uint8_t second_register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
                    vm->regi[register_index] = vm->regi[second_register_index];
                }
                else
                if(second_register_type == OPCODE_STRING) /* moving a string into an int register */
                {
                    uint8_t second_register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
                    vm->regi[register_index] = nap_int_string_to_number(
                                vm->regs[second_register_index],
                                vm->regslens[second_register_index]);
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
                vm->regslens[register_index] = vm->stringtable[str_index]->len;
            }
            else
            if(move_source == OPCODE_VAR) /* movin a variable into a string reg*/
            {
                nap_index_t var_index = nap_fetch_index(vm);
                /* fetch the variable from the given index */
                struct variable_entry* var = nap_fetch_variable(vm, var_index);
                ASSERT_NOT_NULL_VAR(var)
                CHECK_VARIABLE_INSTANTIATON(var)
                CHECK_VARIABLE_TYPE(var, STACK_ENTRY_STRING)

                /* and moving the value in the regsiter itself */
                vm->regs[register_index] = (char*)var->instantiation->value;
                vm->regslens[register_index] = var->instantiation->len;
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
                vm->regidx[register_index] = nap_read_immediate(vm);
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
        struct variable_entry* var = nap_fetch_variable(vm, var_index);
        uint8_t move_source = 0;
        ASSERT_NOT_NULL_VAR(var)
        CHECK_VARIABLE_INSTANTIATON(var)

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
                        if(*(nap_int_t*)var->instantiation->value != vm->regi[register_index])
                        {
                            *(nap_int_t*)var->instantiation->value = vm->regi[register_index];
                        }
                    }
                    else /* allocate the memory for the value */
                    {
                        nap_int_t* temp = (nap_int_t*)calloc(1, sizeof(nap_int_t));
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
                        MEM_FREE(var->instantiation->value);
                    }
                    char* temp = (char*)calloc(
                                vm->regslens[register_index] * 4 + 1, /* UTF-32BE */
                                sizeof(char));

                    strncpy(temp, vm->regs[register_index],
                            vm->regslens[register_index] * 4);

                    var->instantiation->value = temp;
                    var->instantiation->len = vm->regslens[register_index];
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
        else /* moving another variable or an immediate into the variable */
        {
            _NOT_IMPLEMENTED
        }
    }
    else
    if(mov_target == OPCODE_CCIDX)
    {
        uint8_t ccidx_target = vm->content[vm->cc ++];  /* should be a variable */
        if(ccidx_target == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            uint8_t ctr_used_index_regs = 0;
            uint8_t move_src = 0;

            struct variable_entry* var = nap_fetch_variable(vm, var_index);

            ASSERT_NOT_NULL_VAR(var)
            CHECK_VARIABLE_INSTANTIATON(var)

            /* first time usage of this variable? */
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

                        if(real_index + vm->regslens[register_index] > strlen((char*)var->instantiation->value))
                        {
                            char* s = (char*)calloc(256, sizeof(char));
                            SNPRINTF(s, 256,
                                    "[ERR-INT-3] Index overflow error for [%s]."
                                     "Requested index: [%" PRIu64 "] "
                                     "Available length: [%ld] "
                                     "Assumed length: [%" PRIu64 "]\n",
                                     var->name,
                                     real_index,
                                     strlen((char*)var->instantiation->value),
                                     real_index + vm->regslens[register_index]);
                            vm->error_description = s;
                            return NAP_FAILURE;
                        }
                        /* and finally do a strcpy */
                        strncpy((char*)var->instantiation->value + real_index,
                                vm->regs[register_index],
                                vm->regslens[register_index]);

                    }
                    else
                    {
                        _NOT_IMPLEMENTED
                    }
                }
                else
                if(register_type == OPCODE_INT)
                {
                    /* moving only if this int registers goes to an int var*/
                    if(var->instantiation->type == OPCODE_INT)
                    {
                        int64_t idx = deliver_flat_index(vm, var, ctr_used_index_regs);
                        if(idx == -1) /* not used all the indexes */
                        {

                        }
                        else
                        if(idx == -2) /* index overflow */
                        {

                        }
                        else
                        {
                            /* casting it to nap_int_t is ok, since var->instantiation->type == OPCODE_INT */
                            ((nap_int_t*)var->instantiation->value)[idx] = vm->regi[register_index];
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

    return NAP_SUCCESS;
}
