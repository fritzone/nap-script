#include "mov.h"
#include "opcodes.h"
#include "stack.h"
#include "metatbl.h"
#include "nbci_impl.h"
#include "nap_consts.h"
#include "nbci.h"

#include <stdlib.h>
#include <string.h>

/**
 * @brief init_string_register initializes the string register to a given value.
 *
 * Firstly it deallocates the memory (if any) which was previously allocated to
 * this string register. Then copies the string into the register. Also sets
 * the length registers.
 *
 * In case of error it preserves the value of the register.
 *
 * @param vm - the VM in which all this is happening
 * @param reg_idx - the target register index
 * @param target - the target string (UTF-32BE)
 * @param target_len - the length of the target string (string len, not memory
 * area length)
 *
 * @return NULL in case of error, or the new value of vm->regs[reg_idx]
 */
static char* init_string_register(struct nap_vm* vm, uint8_t reg_idx,
                                  const char* target, size_t target_len)
{
    char* tmp = (char*)(calloc(target_len * 4, sizeof(char)));
    if(!tmp)
    {
        return NULL;
    }

    memcpy(tmp, target, target_len * 4);


    if(vm->regs[reg_idx])
    {
        /* set teh memory value to zero to avoid hacking */
        memset(vm->regs[reg_idx], 0, vm->regslens[reg_idx] * 4);
        MEM_FREE(vm->regs[reg_idx]);
    }

    vm->regs[reg_idx] = tmp;
    vm->regslens[reg_idx] = target_len;
    return tmp;
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
static int64_t deliver_flat_index(struct nap_vm* vm,
                                   const struct variable_entry* ve,
                                   uint8_t used_indexes, char** error)
{
    int64_t to_ret = 0;
    int i = 0;

    /* moving block of arrays is not permitted yet :( */

    /* checking the validity of the requested index count */
    if(used_indexes != ve->dimension_count)
    {
        char* s = (char*)calloc(256, sizeof(char));
        SNPRINTF(s, 256,
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
        if(vm->regidx[i] >= ve->dimensions[i] || vm->regidx[i] < 0)
        {
            char* s = (char*)calloc(256, sizeof(char));
            SNPRINTF(s, 256,
                    "Index out of range for [%s]. "
                     "Index: %d, requested: %" PRId64 " available: %d",
                     ve->name, i, vm->regidx[i], ve->dimensions[i]);
            *error = s;
            return INDEX_OUT_OF_RANGE; /* an index overflow */
        }

        /* the big dimension multiplier */
        while(j < used_indexes - i - 1)
        {
            dim_multiplier *= ve->dimensions[j ++];
        }

        /* and multiplying it with the requested index */
        dim_multiplier *= vm->regidx[i];

        /* updating the final index*/
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

static int mov_into_register(struct nap_vm* vm)
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
        if(move_source == OPCODE_RV) /* moving the return value of some function in a reg*/
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
        if(move_source == OPCODE_REG) /* moving a register in another int reg */
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
        if(move_source == OPCODE_CCIDX) /* moving an indexed in an int reg */
        {
            uint8_t ccidx_target = vm->content[vm->cc ++];  /* should be a variable */
            if(ccidx_target == OPCODE_VAR)
            {
                nap_index_t var_index = nap_fetch_index(vm);
                struct variable_entry* var = nap_fetch_variable(vm, var_index);
                uint8_t ctr_used_index_regs = 0;

                ASSERT_NOT_NULL_VAR(var)
                CHECK_VARIABLE_INSTANTIATON(var)

                /* now should come the index reg counter of vm->ccidx,
                   a simple byte since there are max 256 indexes */
                ctr_used_index_regs = vm->content[vm->cc ++];

                /* moving only if this int register goes to an int var*/
                if(var->instantiation->type == OPCODE_INT)
                {
                    char* error = NULL;
                    int64_t idx = deliver_flat_index(vm, var,
                                                     ctr_used_index_regs,
                                                     &error);
                    if(idx < 0) /* error? */
                    {
                        vm->error_description = error;
                        return NAP_FAILURE;
                    }
                    else
                    {
                        /* casting it to nap_int_t is ok, since
                         * var->instantiation->type == OPCODE_INT so we have
                         * allocated nap_int_t variable in the grow */
                        vm->regi[register_index] =
                                ((nap_int_t*)var->instantiation->value)[idx];
                    }
                }
                else
                {
                    _NOT_IMPLEMENTED
                }

            }
            else /* what else can be indexed? */
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

            init_string_register(vm, register_index,
                                 vm->stringtable[str_index]->string,
                                 vm->stringtable[str_index]->len);
        }
        else
        if(move_source == OPCODE_VAR) /* moving a variable into a string reg*/
        {
            nap_index_t var_index = nap_fetch_index(vm);
            /* fetch the variable from the given index */
            struct variable_entry* var = nap_fetch_variable(vm, var_index);
            ASSERT_NOT_NULL_VAR(var)
            CHECK_VARIABLE_INSTANTIATON(var)
            CHECK_VARIABLE_TYPE(var, STACK_ENTRY_STRING)

            /* and moving the value in the regsiter itself */
            init_string_register(vm, register_index,
                                 (char*)var->instantiation->value,
                                 var->instantiation->len);
        }
        else
        if(move_source == OPCODE_REG)
        {
            uint8_t second_register_type = vm->content[vm->cc ++]; /* int/string/float...*/
            if(second_register_type == OPCODE_STRING)
            {
                uint8_t second_register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
                init_string_register(vm, register_index,
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

    return NAP_SUCCESS;
}

static int mov_into_variable(struct nap_vm* vm)
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

                memcpy(temp, vm->regs[register_index],
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

    return NAP_SUCCESS;
}

static int mov_into_indexed(struct nap_vm* vm)
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
        {
            /* moving a register in the indexed destination */
            uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/

            if(register_type == OPCODE_STRING)
            {
                /* moving a string register into a string variable at a specific location */
                if(var->instantiation->type == OPCODE_STRING)
                {
                    if(ctr_used_index_regs == 1)
                    {
                        /* this is a string, accessing a character from it:
                         so calculate the "real" index ofthe variable based
                         on the regidx vector and ctr_used_index_regs
                        */
                        uint64_t real_index = vm->regidx[0];

                        /* do we fit in? */
                        if(real_index + vm->regslens[register_index] > var->instantiation->len)
                        {
                            char* s = (char*)calloc(256, sizeof(char));
                            SNPRINTF(s, 256,
                                     "Index overflow error for [%s]."
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

                        /* and finally do a memcpy */
                        memcpy((char*)var->instantiation->value + real_index * 4,
                               vm->regs[register_index],
                               vm->regslens[register_index] * 4); /* UTF-32 BE */
                    }
                    else /* string[2,5] = "ABC" - removes from the string the substring [2,5] and puts in the new string */
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
            if(register_type == OPCODE_INT)
            {
                /* moving only if this int register goes to an int var*/
                if(var->instantiation->type == OPCODE_INT)
                {
                    char* error = NULL;
                    int64_t idx = deliver_flat_index(vm, var,
                                                     ctr_used_index_regs,
                                                     &error);
                    if(idx < 0) /* error? */
                    {
                        vm->error_description = error;
                        return NAP_FAILURE;
                    }
                    else
                    {
                        /* casting it to nap_int_t is ok, since
                         * var->instantiation->type == OPCODE_INT so we have
                         * allocated nap_int_t variable in the grow */
                        ((nap_int_t*)var->instantiation->value)[idx]
                                =  vm->regi[register_index];
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

    return NAP_SUCCESS;
}

int nap_mov(struct nap_vm* vm)
{
    uint8_t mov_target = vm->content[vm->cc ++];   /* where we move (reg, var)*/

    if(mov_target == OPCODE_REG) /* do we move in a register? */
    {
        return mov_into_register(vm);
    }
    else
    if(mov_target == OPCODE_VAR) /* we move into a variable */
    {
        return mov_into_variable(vm);
    }
    else
    if(mov_target == OPCODE_CCIDX)
    {
        return mov_into_indexed(vm);
    }
    else
    {
        _NOT_IMPLEMENTED
    }

    return NAP_SUCCESS;
}
