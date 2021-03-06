#include "mov.h"
#include "opcodes.h"
#include "stack.h"
#include "metatbl.h"
#include "nbci_impl.h"
#include "nap_consts.h"
#include "nbci.h"
#include <errno.h>

#include <stdlib.h>
#include <string.h>


static int nap_int_set_index_overflow(struct nap_vm* vm, 
				      const char* var_name, 
				      nap_int_t requested, size_t available)
{
	char s[256] = {0};
    SNPRINTF(s, MAX_BUF_SIZE(255),
             "Index out of range for [%s]."
             "Requested index: [%" PRINT_u "] "
             "Available length: [%" PRINT_st "] ",
             var_name,
             requested,
             available);
    return nap_vm_set_error_description(vm, s);
}

/**
 * Moves a string
 */
static int move_string_into_substring(struct nap_vm* vm, nap_int_t start_index, nap_int_t end_index,
                                      char** target, size_t* target_len,
                                      char* source, size_t source_len,
                                      char* helper)
{
    char* first_part; /* from 0 to start_index (excluding), but at the end the final value */
    size_t new_len = 0;
    /* in these scenarios nothing will be done*/
    if(end_index < start_index || end_index < 0 || start_index < 0)
    {
        return NAP_SUCCESS;
    }

    if((size_t)end_index >= *target_len)
    {
        end_index = *target_len - 1;
    }

    /* calculate the new length of the target */
    new_len = *target_len -
            ((size_t)end_index - (size_t)start_index - 1) +
            source_len; /* the new length of the var*/

    if((size_t)start_index >= *target_len)
    {
        return nap_int_set_index_overflow(vm, helper, start_index, *target_len);
    }

    NAP_STRING_ALLOC(vm, first_part, new_len);

    /* taking the first part from the string */
    NAP_STRING_COPY(first_part, *target, (size_t)start_index);

    /* the second part */
    NAP_STRING_COPY(first_part + NAP_STRING_LEN(start_index), source, source_len);

    NAP_STRING_COPY(first_part + NAP_STRING_LEN(start_index + source_len),
           *target + NAP_STRING_LEN(end_index + 1), /* +1 because we don't include end_index*/
           (*target_len - (size_t)end_index - 1)
                    ); /* and fetching in what remained, -1 see above */

    NAP_MEM_FREE(*target);
    *target = first_part;
    *target_len = new_len;
    return NAP_SUCCESS;
}

static int mov_into_byte_register(struct nap_vm* vm)
{
    uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
    uint8_t move_source = vm->content[nap_step_ip(vm)]; /* what are we moving in the byte register */

    if(move_source == OPCODE_PEEK) /* TODO: This is very similar to the same section for the "int" register */
    {
        uint8_t peek_base = vm->content[nap_step_ip(vm)]; /* SP or BP */
        if(peek_base != OPCODE_SP && peek_base != OPCODE_BP)
        {
            return NAP_FAILURE;
        }
        uint8_t peek_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        uint8_t peek_index_type = vm->content[nap_step_ip(vm)]; /* what type follows*/
        nap_index_t peek_index = 0; /* the index that's peeked */

        if(peek_index_type == OPCODE_IMMEDIATE_INT) /* immediate value (1,..) */
        {
            int success = 0;
            peek_index = (nap_index_t)nap_read_immediate_int(vm, &success);
            if(success == NAP_FAILURE)
            {
                return NAP_FAILURE;
            }
        }
        else /* nothing else can be peeked from the stack */
        {
            NAP_NOT_IMPLEMENTED
        }

        int64_t idx = (peek_base == OPCODE_BP?vm->cec->bp:vm->cec->stack_pointer) - peek_index;
        struct stack_entry* se = vm->cec->stack[idx];

        if(peek_type == OPCODE_INT) /* we are dealing with an INT type peek */
        {   /* peek int: assumes that on the stack there is a nap_int_t in the value of the stack_entry at the given index*/
            nap_set_regb(vm, register_index, (nap_byte_t)*(nap_byte_t*)se->value); /* STACK VALUE FROM peek_index */
        }
        else
        if(peek_type == OPCODE_BYTE) /* we are dealing with a BYTE type peek */
        {   /* peek byte: assumes that on the stack there is a nap_byte_t in the value of the stack_entry at the given index*/
            nap_set_regb(vm, register_index, *(nap_byte_t*)se->value); /* STACK VALUE FROM peek_index */
        }
        else
        if(peek_type == OPCODE_REAL) /* we are dealing with a real type peek */
        {   /* peek real: assumes that on the stack there is a nap_real_t in the value of the stack_entry at the given index*/
            nap_set_regb(vm, register_index, (nap_byte_t)*(nap_real_t*)se->value); /* STACK VALUE FROM peek_index */
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_IMMEDIATE_INT) /* immediate integer value */
    {
        int success = NAP_SUCCESS;
        nap_set_regb(vm, register_index, (nap_byte_t)nap_read_immediate_int(vm, &success));
        return success;
    }
    else
    if(move_source == OPCODE_IMMEDIATE_REAL) /* immediate real value */
    {
        nap_set_regb(vm, register_index, (nap_byte_t)nap_read_immediate_real(vm));
    }
    else
    if(move_source == OPCODE_VAR) /* movin a variable into reg*/
    {
        nap_index_t var_index = nap_fetch_index(vm);
        /* and fetch the variable from the given index */
        struct variable_entry* var = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(var);
        CHECK_VARIABLE_INSTANTIATON(var);
        if(var->instantiation->type == STACK_ENTRY_INT)
        {
            nap_set_regb(vm, register_index, (nap_byte_t)(*(nap_int_t*)var->instantiation->value) );
        }
        else
        if(var->instantiation->type == STACK_ENTRY_REAL)
        {
            nap_set_regb(vm, register_index, (nap_byte_t)(*(nap_real_t*)var->instantiation->value) );
        }
        else
        if(var->instantiation->type == STACK_ENTRY_STRING)
        {
            int error = NAP_SUCCESS; /* might lose some numbers */
            nap_set_regb(vm,
                         register_index,
                         (nap_byte_t)nap_string_to_number_int(
                             vm, (nap_string_t)var->instantiation->value,
                             var->instantiation->len, &error)
                         );
            return error;
        }
        else
        if(var->instantiation->type == STACK_ENTRY_BYTE)
        {
            nap_set_regb(vm, register_index, *(nap_byte_t*)var->instantiation->value);
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_RV) /* moving the return value of some function in a reg*/
    {
        uint8_t return_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(return_type == OPCODE_INT)                 /* handles: mov reg byte 0, rv int*/
        {
            nap_set_regb(vm, register_index, (nap_byte_t)vm->cec->rvi); /* WARNING! Possile loss of data*/
        }
        else
        if(return_type == OPCODE_STRING)              /* handles: mov reg byte 0, rv string*/
        {
            int error = NAP_SUCCESS; /* might lose some numbers */
            nap_set_regb(vm,
                         register_index,
                         (nap_byte_t)nap_string_to_number_int(
                             vm, vm->cec->rvs, vm->cec->rvl, &error)
                         );
            return error;
        }
        else
        if(return_type == OPCODE_BYTE)                 /* handles: mov reg byte 0, rv byte*/
        {
            nap_set_regb(vm, register_index, vm->cec->rvb);
        }
        else
        if(return_type == OPCODE_REAL)                 /* handles: mov reg byte 0, rv real*/
        {
            nap_set_regb(vm, register_index, (nap_byte_t)vm->cec->rvr); /* WARNING! Possile loss of data*/
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_REG) /* moving a register in another int reg */
    {
        uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(second_register_type == OPCODE_INT) /* moving an int register in the byte register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regb(vm, register_index,
                         (nap_byte_t)nap_regi(vm, second_register_index));
        }
        else
        if(second_register_type == OPCODE_STRING) /* moving a string register into a byte register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            int error = NAP_SUCCESS;
            nap_set_regb(vm,
                         register_index,
                         (nap_byte_t)nap_string_to_number_int(vm,
                                    nap_regs(vm, second_register_index)->s,
                                    nap_regs(vm, second_register_index)->l, &error)
                         );
            return error;
        }
        else
        if(second_register_type == OPCODE_BYTE) /* moving a byte register in the byte register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regb(vm, register_index, nap_regb(vm, second_register_index));
        }
        else
        if(second_register_type == OPCODE_REAL) /* moving a real register in the byte register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regb(vm, register_index,
                         (nap_byte_t)nap_regr(vm, second_register_index));
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_CCIDX) /* moving an indexed in the byte reg */
    {
        uint8_t ccidx_target = vm->content[nap_step_ip(vm)];  /* should be a variable */
        if(ccidx_target == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            struct variable_entry* var = nap_fetch_variable(vm, var_index);
            uint8_t ctr_used_index_regs = 0;
            char* error = NULL;
            int64_t idx = -1;

            ASSERT_NOT_NULL_VAR(var)
            CHECK_VARIABLE_INSTANTIATON(var)

            /* now should come the index reg counter of vm->ccidx,
               a simple byte since there are max 256 indexes */
            ctr_used_index_regs = vm->content[nap_step_ip(vm)];
            idx = deliver_flat_index(vm, var, ctr_used_index_regs, &error);

            if(idx < 0) /* error? */
            {
                /* do not use the set_error here, the string was allocated
                   copying it would be a memory leak*/
                vm->error_description = error;
                return NAP_FAILURE;
            }

            /* moving an indexed int variable into the byte register*/
            if(var->instantiation->type == OPCODE_INT)
            {
                /* casting it to nap_int_t is ok, since
                 * var->instantiation->type == OPCODE_INT so we have
                 * allocated nap_int_t variable in the grow */
                nap_set_regb(vm, register_index,
                             (nap_byte_t)(((nap_int_t*)var->instantiation->value)[idx]));
            }
            else
            /* moving an indexed byte variable into the byte register*/
            if(var->instantiation->type == OPCODE_BYTE)
            {
                nap_set_regb(vm, register_index,
                             ((nap_byte_t*)var->instantiation->value)[idx]);
            }
            else
            /* moving an indexed real variable into the byte register*/
            if(var->instantiation->type == OPCODE_REAL)
            {
                nap_set_regb(vm, register_index,
                             (nap_byte_t)(((nap_real_t*)var->instantiation->value)[idx]));
            }
            else
            if(var->instantiation->type == OPCODE_STRING) /* possibly: mov reg byte(0), string[2]*/
            {
                if(ctr_used_index_regs == 1)
                {
                    int success = NAP_SUCCESS;
                    /* this is a string, accessing a character from it:
                     so calculate the "real" index of the variable based
                     on the regidx vector and ctr_used_index_regs
                    */
                    size_t real_index = (size_t)nap_regidx(vm, 0);
                    if(real_index >= var->instantiation->len)
                    {
                        return nap_int_set_index_overflow(vm, var->name,
                                       real_index, var->instantiation->len);
                    }

                    /* and finally put the "character" in the register */
                    nap_set_regb(vm, register_index,
                                 (nap_byte_t)nap_string_to_number_int(vm,
                                                (char*)var->instantiation->value + real_index * CC_MUL,
                                                1, &success)
                                 ); /* taking only one character */
                    return success;
                }
                else
                if(ctr_used_index_regs == 2) /* string[2,5] = "ABC" - removes from the string the substring [2,5] and puts in the new string */
                {
                    size_t start_index = (size_t)nap_regidx(vm, 0); /* starting from this */
                    size_t end_index = (size_t)nap_regidx(vm, 1);
                    size_t temp_len = end_index - start_index + 1;
                    int success = NAP_SUCCESS;

                    ASSERT_VARIABLE_INDEX_ALLOWED(var, start_index);
                    ASSERT_VARIABLE_INDEX_ALLOWED(var, end_index);
                    ASSERT_INDEX_RELATIONS(var, start_index, end_index);

                    nap_set_regb(vm, register_index,
                                 (nap_byte_t)nap_string_to_number_int(vm,
                                        (nap_string_t)var->instantiation->value + start_index * CC_MUL,
                                        temp_len, &success));
                    return success;
                }
                else /* string [x,y,z] make no sense */
                {
                    return nap_vm_set_error_description(vm,
                              "Cannot use more than 2 indexes on strings");
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED /* No other type of indexables */
            }
        }
        else /* what else can be indexed? a simple string maybe? */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}

static int mov_into_int_register(struct nap_vm* vm)
{
    uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
    uint8_t move_source = vm->content[nap_step_ip(vm)]; /* what are we moving in*/

    if(move_source == OPCODE_PEEK)
    {
        uint8_t peek_base = vm->content[nap_step_ip(vm)]; /* SP or BP */
        if(peek_base != OPCODE_SP && peek_base != OPCODE_BP)
        {
            return NAP_FAILURE;
        }
        uint8_t peek_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        uint8_t peek_index_type = vm->content[nap_step_ip(vm)]; /* what type follows*/
        nap_index_t peek_index = 0; /* the index that's peeked */

        if(peek_index_type == OPCODE_IMMEDIATE_INT) /* immediate value (1,..) */
        {
            int success = 0;
            peek_index = (nap_index_t)nap_read_immediate_int(vm, &success);
            if(success == NAP_FAILURE)
            {
                return NAP_FAILURE;
            }
        }
        else /* nothing else can be peeked from the stack */
        {
            NAP_NOT_IMPLEMENTED
        }


        dump_stack(vm, stdout);
        fflush(stdout);


        int64_t idx = (peek_base == OPCODE_BP?vm->cec->bp:vm->cec->stack_pointer) - peek_index;
        struct stack_entry* se = vm->cec->stack[idx];

        if(peek_type == OPCODE_INT) /* we are dealing with an INT type peek */
        {   /* peek int: assumes that on the stack there is a nap_int_t in the value of the stack_entry at the given index*/
            nap_set_regi(vm, register_index, *(nap_int_t*)se->value); /* STACK VALUE FROM peek_index */
        }
        else
        if(peek_type == OPCODE_BYTE) /* we are dealing with a BYTE type peek */
        {   /* peek byte: assumes that on the stack there is a nap_byte_t in the value of the stack_entry at the given index*/
            nap_set_regi(vm, register_index, (nap_int_t)*(nap_byte_t*)se->value); /* STACK VALUE FROM peek_index */
        }
        else
        if(peek_type == OPCODE_REAL) /* we are dealing with a real type peek */
        {   /* peek real: assumes that on the stack there is a nap_real_t in the value of the stack_entry at the given index*/
            nap_set_regi(vm, register_index, (nap_int_t)*(nap_real_t*)se->value); /* STACK VALUE FROM peek_index */
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_LEN) /* the length of a variable */
    {
        uint8_t length_of = vm->content[nap_step_ip(vm)];
        if(length_of == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            /* and fetch the variable from the given index */
            struct variable_entry* var = nap_fetch_variable(vm, var_index);
            ASSERT_NOT_NULL_VAR(var)
            CHECK_VARIABLE_INSTANTIATON(var)

            nap_set_regi(vm, register_index, var->instantiation->len);
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_IMMEDIATE_INT) /* immediate integer value*/
    {
        int success = NAP_SUCCESS;
        nap_set_regi(vm, register_index, nap_read_immediate_int(vm, &success));
        return success;
    }
    else
    if(move_source == OPCODE_IMMEDIATE_REAL) /* immediate real value */
    {
        nap_set_regi(vm, register_index, (nap_int_t)nap_read_immediate_real(vm));
    }
    else
    if(move_source == OPCODE_VAR) /* movin a variable into reg*/
    {
        nap_index_t var_index = nap_fetch_index(vm);
        /* and fetch the variable from the given index */
        struct variable_entry* var = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(var)
        CHECK_VARIABLE_INSTANTIATON(var)
        if(var->instantiation->type == STACK_ENTRY_INT)
        {
            nap_set_regi(vm, register_index, *(nap_int_t*)var->instantiation->value );
        }
        else
        if(var->instantiation->type == STACK_ENTRY_REAL)
        {
            nap_set_regi(vm, register_index, (nap_int_t)(*(nap_real_t*)var->instantiation->value) );
        }
        else
        if(var->instantiation->type == STACK_ENTRY_STRING)
        {
            int error = NAP_SUCCESS; /* might lose some numbers */
            nap_set_regi(vm,
                         register_index,
                         nap_string_to_number_int(
                             vm, (nap_string_t)var->instantiation->value,
                             var->instantiation->len, &error)
                         );
            return error;
        }
        else
        if(var->instantiation->type == STACK_ENTRY_BYTE)
        {
            nap_set_regi(vm, register_index, (nap_int_t)(*(nap_byte_t*)var->instantiation->value));
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_RV) /* moving the return value of some function in a reg*/
    {
        uint8_t return_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(return_type == OPCODE_INT)                 /* handles: mov reg int 0, rv int*/
        {
            nap_set_regi(vm, register_index, vm->cec->rvi);
        }
        else
        if(return_type == OPCODE_STRING)              /* handles: mov reg int 0, rv string*/
        {
            int error = NAP_SUCCESS;
            nap_set_regi(vm, register_index, nap_string_to_number_int(vm, vm->cec->rvs,
                                                                vm->cec->rvl, &error) );
            return error;
        }
        else
        if(return_type == OPCODE_BYTE)                 /* handles: mov reg int 0, rv byte*/
        {
            nap_set_regi(vm, register_index, (nap_int_t)vm->cec->rvb);
        }
        else
        if(return_type == OPCODE_REAL)                 /* handles: mov reg int 0, rv real*/
        {
            nap_set_regi(vm, register_index, (nap_int_t)vm->cec->rvr); /* WARNING! Possile loss of data*/
        }
        else
        {
            NAP_NOT_IMPLEMENTED /* UNKNOWN return type */
        }
    }
    else
    if(move_source == OPCODE_REG) /* moving a register in another int reg */
    {
        uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(second_register_type == OPCODE_INT) /* moving an int register in another int register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regi(vm, register_index, nap_regi(vm, second_register_index));
        }
        else
        if(second_register_type == OPCODE_STRING) /* moving a string reg into an int register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            int error = NAP_SUCCESS;
            nap_set_regi(vm, register_index,
                         nap_string_to_number_int(vm,
                                nap_regs(vm, second_register_index)->s,
                                nap_regs(vm, second_register_index)->l,
                                &error)
                         );
            return error;
        }
        else
        if(second_register_type == OPCODE_BYTE) /* moving a byte register in the int register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regi(vm, register_index, (nap_int_t)nap_regb(vm, second_register_index));
        }
        else
        if(second_register_type == OPCODE_REAL) /* moving a real register in the int register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regi(vm, register_index,
                         (nap_int_t)nap_regr(vm, second_register_index)); /* WARNING" Possible loss of data*/
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_CCIDX) /* moving an indexed in an int reg */
    {
        uint8_t ccidx_target = vm->content[nap_step_ip(vm)];  /* should be a variable */
        if(ccidx_target == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            struct variable_entry* var = nap_fetch_variable(vm, var_index);
            uint8_t ctr_used_index_regs = 0;
            char* error = NULL;
            int64_t idx = -1;

            ASSERT_NOT_NULL_VAR(var)
            CHECK_VARIABLE_INSTANTIATON(var)

            /* now should come the index reg counter of vm->ccidx,
               a simple byte since there are max 256 indexes */
            ctr_used_index_regs = vm->content[nap_step_ip(vm)];
            idx = deliver_flat_index(vm, var, ctr_used_index_regs, &error);

            if(idx < 0) /* error? */
            {
                /* do not use the set_error here, the string was allocated
                   copying it would be a memory leak*/
                vm->error_description = error;
                return NAP_FAILURE;
            }

            /* moving only if this int register goes to an int var*/
            if(var->instantiation->type == OPCODE_INT)
            {
                /* casting it to nap_int_t is ok, since
                 * var->instantiation->type == OPCODE_INT so we have
                 * allocated nap_int_t variable in the grow */
                nap_set_regi(vm, register_index, ((nap_int_t*)var->instantiation->value)[idx]);
            }
            else
            /* moving an indexed byte variable into the byte register*/
            if(var->instantiation->type == OPCODE_BYTE)
            {
                nap_set_regi(vm, register_index,
                             (nap_int_t)(((nap_byte_t*)var->instantiation->value)[idx]));
            }
            else
            /* moving an indexed real variable into the byte register*/
            if(var->instantiation->type == OPCODE_REAL)
            {
                nap_set_regi(vm, register_index,
                             (nap_int_t)(((nap_real_t*)var->instantiation->value)[idx]));
            }
            else
            if(var->instantiation->type == OPCODE_STRING) /* possibly: mov reg int(0), string[2]*/
            {
                if(ctr_used_index_regs == 1)
                {
                    int success = NAP_SUCCESS;
                    /* this is a string, accessing a character from it:
                     so calculate the "real" index ofthe variable based
                     on the regidx vector and ctr_used_index_regs
                    */
                    size_t real_index = (size_t)nap_regidx(vm, 0);
                    if(real_index >= var->instantiation->len)
                    {
                        return nap_int_set_index_overflow(vm, var->name,
                                       real_index, var->instantiation->len);
                    }

                    /* and finally put the "character" in the register */
                    nap_set_regi(vm, register_index,
                                 nap_string_to_number_int(vm,
                                                          (char*)var->instantiation->value + real_index * CC_MUL,
                                                          1, &success)
                                 ); /* taking only one character */
                    return success;
                }
                else
                if(ctr_used_index_regs == 2) /* string[2,5] = "ABC" - removes from the string the substring [2,5] and puts in the new string */
                {
                    size_t start_index = (size_t)nap_regidx(vm, 0); /* starting from this */
                    size_t end_index = (size_t)nap_regidx(vm, 1);
                    size_t temp_len = end_index - start_index + 1;
                    int success = NAP_SUCCESS;

                    ASSERT_VARIABLE_INDEX_ALLOWED(var, start_index);
                    ASSERT_VARIABLE_INDEX_ALLOWED(var, end_index);
                    ASSERT_INDEX_RELATIONS(var, start_index, end_index);

                    nap_set_regi(vm, register_index,
                                 nap_string_to_number_int(vm,
                                    (char*)var->instantiation->value + start_index * CC_MUL,
                                    temp_len, &success));
                    return success;
                }
                else /* string [x,y,z] make no sense */
                {
                    return nap_vm_set_error_description(vm,
                              "Cannot use more than 2 indexes on strings");
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED /* No other type of indexables */
            }

        }
        else /* what else can be indexed? */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}

static int mov_into_real_register(struct nap_vm* vm)
{
    uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
    uint8_t move_source = vm->content[nap_step_ip(vm)]; /* what are we moving in*/

    if(move_source == OPCODE_PEEK)
    {
        uint8_t peek_base = vm->content[nap_step_ip(vm)]; /* SP or BP */
        if(peek_base != OPCODE_SP && peek_base != OPCODE_BP)
        {
            return NAP_FAILURE;
        }
        uint8_t peek_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        uint8_t peek_index_type = vm->content[nap_step_ip(vm)]; /* what type follows*/
        nap_index_t peek_index = 0; /* the index that's peeked */

        if(peek_index_type == OPCODE_IMMEDIATE_INT) /* immediate value (1,..) */
        {
            int success = 0;
            peek_index = (nap_index_t)nap_read_immediate_int(vm, &success);
            if(success == NAP_FAILURE)
            {
                return NAP_FAILURE;
            }
        }
        else /* nothing else can be peeked from the stack */
        {
            NAP_NOT_IMPLEMENTED
        }

        int64_t idx = (peek_base == OPCODE_BP?vm->cec->bp:vm->cec->stack_pointer) - peek_index;
        struct stack_entry* se = vm->cec->stack[idx];

        if(peek_type == OPCODE_INT) /* we are dealing with an INT type peek */
        {   /* peek int: assumes that on the stack there is a nap_int_t in the value of the stack_entry at the given index*/
            nap_set_regr(vm, register_index, (nap_real_t)*(nap_int_t*)se->value); /* STACK VALUE FROM peek_index */
        }
        else
        if(peek_type == OPCODE_BYTE) /* we are dealing with a BYTE type peek */
        {   /* peek byte: assumes that on the stack there is a nap_byte_t in the value of the stack_entry at the given index*/
            nap_set_regr(vm, register_index, (nap_real_t)*(nap_byte_t*)se->value); /* STACK VALUE FROM peek_index */
        }
        else
        if(peek_type == OPCODE_REAL) /* we are dealing with a real type peek */
        {   /* peek real: assumes that on the stack there is a nap_real_t in the value of the stack_entry at the given index*/
            nap_set_regr(vm, register_index, *(nap_real_t*)se->value); /* STACK VALUE FROM peek_index */
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_IMMEDIATE_INT) /* immediate integer value goes in real register */
    {
        int success = NAP_SUCCESS;
        nap_set_regr(vm, register_index, (nap_real_t)nap_read_immediate_int(vm, &success));
        return success;
    }
    else
    if(move_source == OPCODE_IMMEDIATE_REAL) /* immediate real value */
    {
        nap_set_regr(vm, register_index, nap_read_immediate_real(vm));
    }
    else
    if(move_source == OPCODE_VAR) /* movin a variable into reg*/
    {
        nap_index_t var_index = nap_fetch_index(vm);
        /* and fetch the variable from the given index */
        struct variable_entry* var = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(var)
        CHECK_VARIABLE_INSTANTIATON(var)

        if(var->instantiation->type == STACK_ENTRY_INT)
        {
            nap_set_regr(vm, register_index, (nap_real_t)(*(nap_int_t*)var->instantiation->value));
        }
        else
        if(var->instantiation->type == STACK_ENTRY_REAL)
        {
            nap_set_regr(vm, register_index, *(nap_real_t*)var->instantiation->value);
        }
        else
        if(var->instantiation->type == STACK_ENTRY_STRING)
        {
            int error = NAP_SUCCESS; /* might lose some numbers */
            nap_set_regr(vm,
                         register_index,
                         nap_string_to_number_real(
                             vm, (nap_string_t)var->instantiation->value,
                             var->instantiation->len, &error)
                         );
            return error;
        }
        else
        if(var->instantiation->type == STACK_ENTRY_BYTE)
        {
            nap_set_regi(vm, register_index, (nap_int_t)(*(nap_byte_t*)var->instantiation->value));
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_RV) /* moving the return value of some function in a reg*/
    {
        uint8_t return_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(return_type == OPCODE_INT)                 /* handles: mov reg int 0, rv int*/
        {
            nap_set_regr(vm, register_index, (nap_real_t)vm->cec->rvi);
        }
        else
        if(return_type == OPCODE_STRING)              /* handles: mov reg int 0, rv string*/
        {
            int error = NAP_SUCCESS;
            nap_set_regr(vm, register_index, nap_string_to_number_real(vm, vm->cec->rvs,
                                                                vm->cec->rvl, &error) );
            return error;
        }
        else
        if(return_type == OPCODE_BYTE)                 /* handles: mov reg int 0, rv byte*/
        {
            nap_set_regr(vm, register_index, (nap_real_t)vm->cec->rvb);
        }
        else
        if(return_type == OPCODE_REAL)                 /* handles: mov reg int 0, rv int*/
        {
            nap_set_regr(vm, register_index, vm->cec->rvr);
        }
        else
        {
            NAP_NOT_IMPLEMENTED /* UNKNOWN return type */
        }
    }
    else
    if(move_source == OPCODE_REG) /* moving a register in the real register */
    {
        uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(second_register_type == OPCODE_INT) /* moving an int register in the reak register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regr(vm, register_index, (nap_real_t)nap_regi(vm, second_register_index));
        }
        else
        if(second_register_type == OPCODE_STRING) /* moving a string reg into a real register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            int error = NAP_SUCCESS;
            nap_set_regr(vm, register_index,
                         (nap_real_t)nap_string_to_number_int(vm,
                                nap_regs(vm, second_register_index)->s,
                                nap_regs(vm, second_register_index)->l,
                                &error)
                         );
            return error;
        }
        else
        if(second_register_type == OPCODE_BYTE) /* moving a byte register in the real register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regr(vm, register_index, (nap_real_t)nap_regb(vm, second_register_index));
        }
        else
        if(second_register_type == OPCODE_REAL) /* moving a real registerin the real register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regr(vm, register_index, nap_regr(vm, second_register_index));
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_CCIDX) /* moving an indexed in a real reg */
    {
        uint8_t ccidx_target = vm->content[nap_step_ip(vm)];  /* should be a variable */
        if(ccidx_target == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            struct variable_entry* var = nap_fetch_variable(vm, var_index);
            uint8_t ctr_used_index_regs = 0;
            char* error = NULL;
            int64_t idx = -1;

            ASSERT_NOT_NULL_VAR(var)
            CHECK_VARIABLE_INSTANTIATON(var)

            /* now should come the index reg counter of vm->ccidx,
               a simple byte since there are max 256 indexes */
            ctr_used_index_regs = vm->content[nap_step_ip(vm)];
            idx = deliver_flat_index(vm, var, ctr_used_index_regs, &error);

            if(idx < 0) /* error? */
            {
                /* do not use the set_error here, the string was allocated
                   copying it would be a memory leak*/
                vm->error_description = error;
                return NAP_FAILURE;
            }

            /* moving only if this int register goes to an int var*/
            if(var->instantiation->type == OPCODE_INT)
            {
                /* casting it to nap_int_t is ok, since
                 * var->instantiation->type == OPCODE_INT so we have
                 * allocated nap_int_t variable in the grow */
                nap_set_regr(vm, register_index, (nap_real_t)(((nap_int_t*)var->instantiation->value)[idx]));
            }
            else
            /* moving an indexed byte variable into the byte register*/
            if(var->instantiation->type == OPCODE_BYTE)
            {
                nap_set_regr(vm, register_index,
                             (nap_real_t)(((nap_byte_t*)var->instantiation->value)[idx]));
            }
            else
            /* moving an indexed real variable into the byte register*/
            if(var->instantiation->type == OPCODE_REAL)
            {
                nap_set_regr(vm, register_index,
                             (nap_real_t)(((nap_real_t*)var->instantiation->value)[idx]));
            }
            else
            if(var->instantiation->type == OPCODE_STRING) /* possibly: mov reg int(0), string[2]*/
            {
                if(ctr_used_index_regs == 1)
                {
                    int success = NAP_SUCCESS;
                    /* this is a string, accessing a character from it:
                     so calculate the "real" index ofthe variable based
                     on the regidx vector and ctr_used_index_regs
                    */
                    size_t real_index = (size_t)nap_regidx(vm, 0);
                    if(real_index >= var->instantiation->len)
                    {
                        return nap_int_set_index_overflow(vm, var->name,
                                       real_index, var->instantiation->len);
                    }

                    /* and finally put the "character" in the register */
                    nap_set_regr(vm, register_index,
                                 nap_string_to_number_real(vm,
                                                          (char*)var->instantiation->value + real_index * CC_MUL,
                                                          1, &success)
                                 ); /* taking only one character */
                    return success;
                }
                else
                if(ctr_used_index_regs == 2) /* string[2,5] = "ABC" - removes from the string the substring [2,5] and puts in the new string */
                {
                    size_t start_index = (size_t)nap_regidx(vm, 0); /* starting from this */
                    size_t end_index = (size_t)nap_regidx(vm, 1);
                    size_t temp_len = end_index - start_index + 1;
                    int success = NAP_SUCCESS;

                    ASSERT_VARIABLE_INDEX_ALLOWED(var, start_index);
                    ASSERT_VARIABLE_INDEX_ALLOWED(var, end_index);
                    ASSERT_INDEX_RELATIONS(var, start_index, end_index)

                    nap_set_regr(vm, register_index,
                                 nap_string_to_number_real(vm,
                                    (char*)var->instantiation->value + start_index * CC_MUL,
                                    temp_len, &success));
                    return success;
                }
                else /* string [x,y,z] make no sense */
                {
                    return nap_vm_set_error_description(vm,
                              "Cannot use more than 2 indexes on strings");
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED /* No other type of indexables */
            }
        }
        else /* what else can be indexed? */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}

static int mov_into_string_register(struct nap_vm* vm)
{
    uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
    uint8_t move_source = vm->content[nap_step_ip(vm)]; /* what are we moving in*/

    if(move_source == OPCODE_PEEK)
    {
        uint8_t peek_base = vm->content[nap_step_ip(vm)]; /* SP or BP */
        if(peek_base != OPCODE_SP && peek_base != OPCODE_BP)
        {
            return NAP_FAILURE;
        }
        uint8_t peek_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        uint8_t peek_index_type = vm->content[nap_step_ip(vm)]; /* what type follows*/
        nap_index_t peek_index = 0; /* the index that's peeked */

        if(peek_index_type == OPCODE_IMMEDIATE_INT) /* immediate value (1,..) */
        {
            int success = 0;
            peek_index = (nap_index_t)nap_read_immediate_int(vm, &success);
            if(success == NAP_FAILURE)
            {
                return NAP_FAILURE;
            }
        }
        else /* nothing else can be peeked from the stack */
        {
            NAP_NOT_IMPLEMENTED
        }
        int64_t idx = (peek_base == OPCODE_BP?vm->cec->bp:vm->cec->stack_pointer) - peek_index;
        struct stack_entry* se = vm->cec->stack[idx];

        if(peek_type == OPCODE_STRING) /* we are dealing with an STRING type peek */
        {
            dump_stack(vm, stdout);
            fflush(stdout);

            char* temp = NULL;
            size_t len = se->len;
            temp = NAP_MEM_ALLOC(len * CC_MUL, char);
            NAP_NN_ASSERT(vm, temp);
            memcpy(temp, se->value, len * CC_MUL);
            int res = nap_set_regs(vm, register_index,
                                 temp,
                                 len);
            NAP_MEM_FREE(temp);
            return res;
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_STRING) /* usually we move an immediate string intro string register*/
    {
        nap_index_t str_index = nap_fetch_index(vm);
        return nap_set_regs(vm, register_index,
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
        return nap_set_regs(vm, register_index,
                             (char*)var->instantiation->value,
                             var->instantiation->len);
    }
    else
    if(move_source == OPCODE_REG)
    {
        uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/

        if(second_register_type == OPCODE_STRING)
        {
            return  nap_set_regs(vm, register_index,
                                 nap_regs(vm, second_register_index)->s,
                                 nap_regs(vm, second_register_index)->l);
        }
        else
        if(second_register_type == OPCODE_INT)
        {
            size_t len = 0;
            char* s_nr = nap_int_to_string (nap_regi(vm, second_register_index) , &len);
            nap_set_regs(vm, register_index, s_nr, len / CC_MUL); /* nap_int_to_string returns the full byte length, not the UTF32 length*/
            free(s_nr);
        }
        else
        if(second_register_type == OPCODE_BYTE)
        {
            size_t len = 0;
            char* s_nr = nap_int_to_string ((nap_int_t)nap_regb(vm, second_register_index) , &len);
            nap_set_regs(vm, register_index, s_nr, len / CC_MUL); /* nap_int_to_string returns the full byte length, not the UTF32 length*/
            free(s_nr);
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_CCIDX) /* moving an indexed in the string reg */
    {
        uint8_t ccidx_target = vm->content[nap_step_ip(vm)];  /* should be a variable */
        if(ccidx_target == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            struct variable_entry* var = nap_fetch_variable(vm, var_index);
            uint8_t ctr_used_index_regs = 0;
            char* error = NULL;
            int64_t idx = -1;

            ASSERT_NOT_NULL_VAR(var)
            CHECK_VARIABLE_INSTANTIATON(var)

            /* now should come the index reg counter of vm->ccidx,
               a simple byte since there are max 256 indexes */
            ctr_used_index_regs = vm->content[nap_step_ip(vm)];
            idx = deliver_flat_index(vm, var, ctr_used_index_regs, &error);

            if(idx < 0) /* error? */
            {
                /* do not use the set_error here, the string was allocated
                   copying it would be a memory leak*/
                vm->error_description = error;
                return NAP_FAILURE;
            }

            /* setting the string register to be an element from an int array */
            if(var->instantiation->type == OPCODE_INT)
            {
                size_t len = 0;
                char* s_nr = nap_int_to_string ( (nap_int_t)((nap_int_t*)var->instantiation->value)[idx], &len );
                nap_set_regs(vm, register_index, s_nr, len);
                free(s_nr);
            }
            else
            /* setting the string register to be an element from a byte array */
            if(var->instantiation->type == OPCODE_BYTE)
            {
                size_t len = 0;
                char* s_nr = nap_int_to_string ( (nap_int_t)((nap_byte_t*)var->instantiation->value)[idx], &len );
                nap_set_regs(vm, register_index, s_nr, len);
                free(s_nr);
            }
            else
            /* setting the string register to be an element from a real array */
            if(var->instantiation->type == OPCODE_REAL)
            {
                NAP_NOT_IMPLEMENTED
//                nap_set_regs(vm,
//                             register_index,
//                             nap_real_to_string ( (nap_real_t)((nap_real_t*)var->instantiation->value)[idx] )
//                             );
            }
            else
            if(var->instantiation->type == OPCODE_STRING) /* possibly: mov reg string(0), string[2]*/
            {
                if(ctr_used_index_regs == 1)
                {
                    int success = NAP_SUCCESS;
                    /* this is a string, accessing a character from it:
                     so calculate the "real" index of the variable based
                     on the regidx vector and ctr_used_index_regs
                    */
                    size_t real_index = (size_t)nap_regidx(vm, 0);
                    if(real_index >= var->instantiation->len)
                    {
                        return nap_int_set_index_overflow(vm, var->name,
                                       real_index, var->instantiation->len);
                    }

                    /* and finally put the "character" in the register */
                    nap_set_regs(vm, register_index, (char*)var->instantiation->value + real_index * CC_MUL, 1); /* taking only one character */
                    return success;
                }
                else
                if(ctr_used_index_regs == 2) /* string[2,5] = "ABC" - removes from the string the substring [2,5] and puts in the new string */
                {
                    size_t start_index = (size_t)nap_regidx(vm, 0); /* starting from this */
                    size_t end_index = (size_t)nap_regidx(vm, 1);
                    size_t temp_len = end_index - start_index + 1;
                    int success = NAP_SUCCESS;

                    ASSERT_VARIABLE_INDEX_ALLOWED(var, start_index);
                    ASSERT_VARIABLE_INDEX_ALLOWED(var, end_index);
                    ASSERT_INDEX_RELATIONS(var, start_index, end_index);

                    nap_set_regs(vm, register_index,
                                    (char*)var->instantiation->value + start_index * CC_MUL,
                                    temp_len);
                    return success;
                }
                else /* string [x,y,z] make no sense */
                {
                    return nap_vm_set_error_description(vm,
                              "Cannot use more than 2 indexes on strings");
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED /* No other type of indexables */
            }
        }
        else /* what else can be indexed? */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}

static int mov_into_index_register(struct nap_vm* vm)
{
    uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
    uint8_t move_source = vm->content[nap_step_ip(vm)]; /* the index definition */
    if(move_source == OPCODE_IMMEDIATE_INT) /* immediate value (1,..) */
    {
        int success = NAP_SUCCESS;
        nap_set_regidx(vm, register_index, nap_read_immediate_int(vm, &success));
        return success;
    }
    if(move_source == OPCODE_IMMEDIATE_REAL) /* immediate real value */
    {
        nap_set_regidx(vm, register_index, (nap_int_t)nap_read_immediate_real(vm));
    }
    else
    if(move_source == OPCODE_VAR) /* movin a variable into reg*/
    {
        nap_index_t var_index = nap_fetch_index(vm);
        /* and fetch the variable from the given index */
        struct variable_entry* var = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(var)
        CHECK_VARIABLE_INSTANTIATON(var)
        if(var->instantiation->type == STACK_ENTRY_INT)
        {
            nap_set_regidx(vm, register_index, *(nap_int_t*)var->instantiation->value );
        }
        else
        if(var->instantiation->type == STACK_ENTRY_REAL)
        {
            nap_set_regidx(vm, register_index, (nap_int_t)(*(nap_real_t*)var->instantiation->value) );
        }
        else
        if(var->instantiation->type == STACK_ENTRY_STRING)
        {
            int error = NAP_SUCCESS; /* might lose some numbers */
            nap_set_regidx(vm,
                         register_index,
                         nap_string_to_number_int(
                             vm, (nap_string_t)var->instantiation->value,
                             var->instantiation->len, &error)
                         );
            return error;
        }
        else
        if(var->instantiation->type == STACK_ENTRY_BYTE)
        {
            nap_set_regidx(vm, register_index, (nap_int_t)(*(nap_byte_t*)var->instantiation->value));
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_RV) /* moving the return value of some function in a reg*/
    {
        uint8_t return_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(return_type == OPCODE_INT)                 /* handles: mov reg int 0, rv int*/
        {
            nap_set_regidx(vm, register_index, vm->cec->rvi);
        }
        else
        if(return_type == OPCODE_STRING)              /* handles: mov reg int 0, rv string*/
        {
            int error = NAP_SUCCESS;
            nap_set_regidx(vm, register_index, nap_string_to_number_int(vm, vm->cec->rvs,
                                                                vm->cec->rvl, &error) );
            return error;
        }
        else
        if(return_type == OPCODE_BYTE)                 /* handles: mov reg int 0, rv byte*/
        {
            nap_set_regidx(vm, register_index, (nap_int_t)vm->cec->rvb);
        }
        else
        if(return_type == OPCODE_REAL)                 /* handles: mov reg int 0, rv real*/
        {
            nap_set_regidx(vm, register_index, (nap_int_t)vm->cec->rvr); /* WARNING! Possile loss of data*/
        }
        else
        {
            NAP_NOT_IMPLEMENTED /* UNKNOWN return type */
        }
    }
    else
    if(move_source == OPCODE_REG) /* moving a register in another int reg */
    {
        uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(second_register_type == OPCODE_INT) /* moving an int register in another int register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regidx(vm, register_index, nap_regi(vm, second_register_index));
        }
        else
        if(second_register_type == OPCODE_STRING) /* moving a string reg into an int register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            int error = NAP_SUCCESS;
            nap_set_regidx(vm, register_index,
                         nap_string_to_number_int(vm,
                                nap_regs(vm, second_register_index)->s,
                                nap_regs(vm, second_register_index)->l,
                                &error)
                         );
            return error;
        }
        else
        if(second_register_type == OPCODE_BYTE) /* moving a byte register in the int register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regidx(vm, register_index, (nap_int_t)nap_regb(vm, second_register_index));
        }
        else
        if(second_register_type == OPCODE_REAL) /* moving a real register in the int register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            nap_set_regidx(vm, register_index,
                         (nap_int_t)nap_regr(vm, second_register_index)); /* WARNING" Possible loss of data*/
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(move_source == OPCODE_CCIDX) /* moving an indexed in an indexed reg */
    {
        uint8_t ccidx_target = vm->content[nap_step_ip(vm)];  /* should be a variable */
        if(ccidx_target == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            struct variable_entry* var = nap_fetch_variable(vm, var_index);
            uint8_t ctr_used_index_regs = 0;
            char* error = NULL;
            int64_t idx = -1;

            ASSERT_NOT_NULL_VAR(var)
            CHECK_VARIABLE_INSTANTIATON(var)

            /* now should come the index reg counter of vm->ccidx,
               a simple byte since there are max 256 indexes */
            ctr_used_index_regs = vm->content[nap_step_ip(vm)];
            idx = deliver_flat_index(vm, var, ctr_used_index_regs, &error);

            if(idx < 0) /* error? */
            {
                /* do not use the set_error here, the string was allocated
                   copying it would be a memory leak*/
                vm->error_description = error;
                return NAP_FAILURE;
            }

            /* moving only if this int register goes to an int var*/
            if(var->instantiation->type == OPCODE_INT)
            {
                /* casting it to nap_int_t is ok, since
                 * var->instantiation->type == OPCODE_INT so we have
                 * allocated nap_int_t variable in the grow */
                nap_set_regidx(vm, register_index, ((nap_int_t*)var->instantiation->value)[idx]);
            }
            else
            /* moving an indexed byte variable into the byte register*/
            if(var->instantiation->type == OPCODE_BYTE)
            {
                nap_set_regidx(vm, register_index,
                             (nap_int_t)(((nap_byte_t*)var->instantiation->value)[idx]));
            }
            else
            /* moving an indexed real variable into the byte register*/
            if(var->instantiation->type == OPCODE_REAL)
            {
                nap_set_regidx(vm, register_index,
                             (nap_int_t)(((nap_real_t*)var->instantiation->value)[idx]));
            }
            else
            if(var->instantiation->type == OPCODE_STRING) /* possibly: mov reg int(0), string[2]*/
            {
                if(ctr_used_index_regs == 1)
                {
                    int success = NAP_SUCCESS;
                    /* this is a string, accessing a character from it:
                     so calculate the "real" index ofthe variable based
                     on the regidx vector and ctr_used_index_regs
                    */
                    size_t real_index = (size_t)nap_regidx(vm, 0);
                    if(real_index >= var->instantiation->len)
                    {
                        return nap_int_set_index_overflow(vm, var->name,
                                       real_index, var->instantiation->len);
                    }

                    /* and finally put the "character" in the register */
                    nap_set_regidx(vm, register_index,
                                 nap_string_to_number_int(vm,
                                                          (char*)var->instantiation->value + real_index * CC_MUL,
                                                          1, &success)
                                 ); /* taking only one character */
                    return success;
                }
                else
                if(ctr_used_index_regs == 2) /* string[2,5] = "ABC" - removes from the string the substring [2,5] and puts in the new string */
                {
                    size_t start_index = (size_t)nap_regidx(vm, 0); /* starting from this */
                    size_t end_index = (size_t)nap_regidx(vm, 1);
                    size_t temp_len = end_index - start_index + 1;
                    int success = NAP_SUCCESS;

                    ASSERT_VARIABLE_INDEX_ALLOWED(var, start_index);
                    ASSERT_VARIABLE_INDEX_ALLOWED(var, end_index);
                    ASSERT_INDEX_RELATIONS(var, start_index, end_index);

                    nap_set_regidx(vm, register_index,
                                 nap_string_to_number_int(vm,
                                    (char*)var->instantiation->value + start_index * CC_MUL,
                                    temp_len, &success));
                    return success;
                }
                else /* string [x,y,z] make no sense */
                {
                    return nap_vm_set_error_description(vm,
                              "Cannot use more than 2 indexes on strings");
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED /* No other type of indexables */
            }
        }
        else /* what else can be indexed? */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }

    return NAP_SUCCESS;
}

int mov_into_peek_target(struct nap_vm* vm)
{
    uint8_t peek_base = vm->content[nap_step_ip(vm)]; /* SP/BP*/
    uint8_t peek_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
    uint8_t peek_index_type = vm->content[nap_step_ip(vm)]; /* what are we moving in*/
    uint8_t source_type = 0;
    nap_index_t peek_index = 0; /* the index that's peeked */

    if(peek_index_type == OPCODE_IMMEDIATE_INT) /* immediate value (1,..) */
    {
        int success = 0;
        peek_index = (nap_index_t)nap_read_immediate_int(vm, &success);
        if(success == NAP_FAILURE)
        {
            return NAP_FAILURE;
        }
    }
    else /* nothing else can be peeked from the stack */
    {
        NAP_NOT_IMPLEMENTED
    }

    /* now read the source, ie. what is moved in the peek target */
    source_type = vm->content[nap_step_ip(vm)];
    if(source_type == OPCODE_REG)
    {
        uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
        struct stack_entry* se = vm->cec->stack[peek_base == OPCODE_BP ? vm->cec->bp:nap_sp(vm) - peek_index];

        if(register_type != peek_type)
        {
            NAP_NOT_IMPLEMENTED
        }

        if(register_type == OPCODE_INT)
        {
            *(nap_int_t*)se->value = nap_regi(vm, register_index);
        }
        else
        if(register_type == OPCODE_REAL)
        {
            *(nap_real_t*)se->value = nap_regr(vm, register_index);
        }
        else
        if(register_type == OPCODE_BYTE)
        {
            *(nap_byte_t*)se->value = nap_regb(vm, register_index);
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }

    return NAP_SUCCESS;
}

int mov_into_generic_register(struct nap_vm* vm)
{
    uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
    uint8_t move_source = vm->content[nap_step_ip(vm)]; /* what are we moving in*/

    if(move_source == OPCODE_CREA)
    {
        nap_index_t class_idx = nap_fetch_index(vm); /* the index of the class */
        uint8_t constructor_call_follows = vm->content[vm->cec->cc];
        nap_addr_t ccall_idx = (nap_addr_t)-1;
        if(constructor_call_follows == 0)
        {
            vm->cec->cc ++;
            ccall_idx = nap_fetch_address(vm);
        }
        return NAP_SUCCESS;
    }

    return NAP_SUCCESS;
}

int mov_into_register(struct nap_vm* vm)
{
    uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

    /* we are dealing with an INT type register */
    if(register_type == OPCODE_INT)  /* to handle mov reg int x, <something> */
    {
        return mov_into_int_register(vm);
    }
    else
    if(register_type == OPCODE_STRING)
    {
        return mov_into_string_register(vm);
    }
    else
    if(register_type == OPCODE_IDX)
    {
        return mov_into_index_register(vm);
    }
    else
    if(register_type == OPCODE_BYTE)
    {
        return mov_into_byte_register(vm);
    }
    else
    if(register_type == OPCODE_REAL)
    {
        return mov_into_real_register(vm);
    }
    else
    if(register_type == OPCODE_GENERIC)
    {
        return mov_into_generic_register(vm);
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }
}

int mov_into_variable(struct nap_vm* vm)
{
    nap_index_t var_index = nap_fetch_index(vm);
    struct variable_entry* var = nap_fetch_variable(vm, var_index);
    uint8_t move_source = 0;
    ASSERT_NOT_NULL_VAR(var)
    CHECK_VARIABLE_INSTANTIATON(var)

    /* and now let's see what we move in the variable */
    move_source = vm->content[nap_step_ip(vm)];

    /* moving a register in a variable? */
    if(move_source == OPCODE_REG)
    {
        uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/

        if(register_type == OPCODE_INT) /* we are dealing with an INT type register */
        {
            /* to check if the variable is the same type. If not, convert */
            if(var->instantiation->type == OPCODE_INT)
            {
                /* perform the operation only if the values are not the same already*/
                if(var->instantiation->value)
                {
                    *(nap_int_t*)var->instantiation->value = nap_regi(vm, register_index);
                }
                else /* allocate the memory for the value */
                {
                    nap_int_t* temp = NAP_MEM_ALLOC(1, nap_int_t);
					NAP_NN_ASSERT(vm, temp);
                    *temp = nap_regi(vm, register_index);
                    var->instantiation->value = temp;
                }
            }
            else
            { /* here: convert the value to hold the requested type */
                NAP_NOT_IMPLEMENTED
            }
        }
        else
        if(register_type == OPCODE_REAL) /* we are dealing with a REAL type register */
        {
            /* to check if the variable is the same type. If not, convert */
            if(var->instantiation->type == OPCODE_REAL)
            {
                if(var->instantiation->value)
                {
                    *(nap_real_t*)var->instantiation->value = nap_regr(vm, register_index);
                }
                else /* allocate the memory for the value */
                {
                    nap_real_t* temp = NAP_MEM_ALLOC(1, nap_real_t);
                    NAP_NN_ASSERT(vm, temp);
                    *temp = nap_regr(vm, register_index);
                    var->instantiation->value = temp;
                }
            }
            else
            { /* here: convert the value to hold the requested type */
                NAP_NOT_IMPLEMENTED
            }
        }
        else
        if(register_type == OPCODE_BYTE) /* we are dealing with a BYTE type register */
        {
            /* to check if the variable is the same type. If not, convert */
            if(var->instantiation->type == OPCODE_BYTE)
            {
                /* perform the operation only if the values are not the same already*/
                if(var->instantiation->value)
                {
                    if(*(nap_byte_t*)var->instantiation->value != nap_regb(vm, register_index))
                    {
                        *(nap_byte_t*)var->instantiation->value = nap_regb(vm, register_index);
                    }
                }
                else /* allocate the memory for the value */
                {
                    nap_byte_t* temp = NAP_MEM_ALLOC(1, nap_byte_t);
					NAP_NN_ASSERT(vm, temp);
                    *temp = nap_regb(vm, register_index);
                    var->instantiation->value = temp;
                }
            }
            else
            { /* here: convert the value to hold the requested type */
                NAP_NOT_IMPLEMENTED
            }
        }
        else
        if(register_type == OPCODE_STRING)
        {
            if(var->instantiation->type == OPCODE_STRING)
            {
				char* temp = NULL;
                /* moving a register into the string variable */
                if(var->instantiation->value)
                {
                    NAP_MEM_FREE(var->instantiation->value);
                }
                NAP_STRING_ALLOC(vm, temp, nap_regs(vm, register_index)->l);
                NAP_STRING_COPY(temp, nap_regs(vm, register_index)->s,
                                nap_regs(vm, register_index)->l);

                var->instantiation->value = temp;
                var->instantiation->len = nap_regs(vm, register_index)->l;
            }
            else /* moving a string register into a non string variable */
            if(var->instantiation->type == OPCODE_INT)
            {
                nap_int_t value;
                int error = NAP_SUCCESS; /* might lose some numbers */

                value = nap_string_to_number_int(vm,
                           nap_regs(vm, register_index)->s,
                           nap_regs(vm, register_index)->l, &error);

                *((nap_int_t*)var->instantiation->value) = value;
                return error;
            }
            else
            {
                NAP_NOT_IMPLEMENTED
            }
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else /* moving another variable or an immediate into the variable */
    {
        NAP_NOT_IMPLEMENTED
    }

    return NAP_SUCCESS;
}

/*
 * Resolves moving into indexed destinations. The following scenarios apply:
 * 1. mov string[x], something -> will move into the target string starting
 *    from position x the something. If the length of something is greater than
 *    the remaining space from x till the end of the string, an error will be
 *    signaled
 * 2. mov string[x,y], something -> will remove the characters between [x,y]
 *    from the string and will instead insert the something. If x == y it
 *    removes only one character.
 * 3. mov int[x1,x2, ... ,xn], something -> will put in the array of ints at
 *    the given position the given something value.
 */
int mov_into_indexed(struct nap_vm* vm)
{
    uint8_t ccidx_target = vm->content[nap_step_ip(vm)];  /* should be a variable */
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
        ctr_used_index_regs = vm->content[nap_step_ip(vm)];

        /* and find what is moved into this vm->ccidx destination*/
        move_src = vm->content[nap_step_ip(vm)];
        if(move_src == OPCODE_REG) /* moving a register in the indexed destination */
        {
            uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
            uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/

            if(register_type == OPCODE_STRING) /* moving a string register into an indexed variable at a specific location */
            {
                if(var->instantiation->type == OPCODE_STRING) /*moving in a string type variable a string register to a given pos: mov string[x [,y] ], regs(0) */
                {
                    if(ctr_used_index_regs == 1) /* mov string[x], regs(0) */
                    {
                        /* this is a string, accessing a character from it:
                         so calculate the "real" index ofthe variable based
                         on the regidx vector and ctr_used_index_regs
                        */
                        size_t real_index = (size_t)nap_regidx(vm, 0);
                        if(real_index >= var->instantiation->len)
                        {
                            return nap_int_set_index_overflow(vm, var->name,
                                           real_index, var->instantiation->len);
                        }

                        /* do we fit in? */
                        if(real_index + nap_regs(vm, register_index)->l > var->instantiation->len)
                        {
							char s[256] = {0};
                            SNPRINTF(s, MAX_BUF_SIZE(255),
                                     "Index overflow error for [%s]."
                                     "Requested index: [%" PRINT_st "] "
                                     "Available length: [%" PRINT_st "] "
                                     "Assumed length: [%" PRINT_st "]\n",
                                     var->name,
                                     real_index,
                                     var->instantiation->len,
                                     real_index + nap_regs(vm, register_index)->l);
                            return nap_vm_set_error_description(vm, s);
                        }

                        /* and finally do a memcpy */
                        memcpy((char*)var->instantiation->value + real_index * CC_MUL,
                               nap_regs(vm, register_index)->s,
                               nap_regs(vm, register_index)->l * CC_MUL); /* UTF-32 BE */
                    }
                    else
                    if(ctr_used_index_regs == 2) /* string[2,5] = "ABC" - removes from the string the substring [2,5] and puts in the new string */
                    {
                        size_t start_index = (size_t)nap_regidx(vm, 0); /* starting from this */
                        size_t end_index = (size_t)nap_regidx(vm, 1);
                        return move_string_into_substring(vm, start_index, end_index,
                                    (char**)&var->instantiation->value,
                                    &var->instantiation->len,
                                    nap_regs(vm, register_index)->s,
                                    nap_regs(vm, register_index)->l,
                                    var->name);
                    }
                    else /* string [x,y,z] make no sense */
                    {
                        return nap_vm_set_error_description(vm,
                                  "Cannot use more than 2 indexes on strings");
                    }
                }
                else
                {
                    NAP_NOT_IMPLEMENTED
                }
            }
            else
            if(register_type == OPCODE_INT)
            {
                /* moving only if this int register goes to an int var*/
                if(var->instantiation->type == OPCODE_INT) /* mov int_var [x,y,z], reg int (register_index) */
                {
                    char* error = NULL;
                    int64_t idx = deliver_flat_index(vm, var,
                                                     ctr_used_index_regs,
                                                     &error);
                    if(idx < 0) /* error? */
                    {
                        /* do not use the set_error here, the string was allocated
                        copying it would be a memory leak*/
                        vm->error_description = error;
                        return NAP_FAILURE;
                    }
                    else
                    {
                        /* casting it to nap_int_t is ok, since
                         * var->instantiation->type == OPCODE_INT so we have
                         * allocated nap_int_t variable in the grow */
                        ((nap_int_t*)var->instantiation->value)[idx]
                                =  nap_regi(vm, register_index);
                    }
                }
                else
                {
                    NAP_NOT_IMPLEMENTED
                }
            }
            else
            if(register_type == OPCODE_BYTE)
            {
                /* moving only if this byte register goes to a byte var*/
                if(var->instantiation->type == OPCODE_BYTE) /* mov byte var [x,y,z], reg byte (register_index) */
                {
                    char* error = NULL;
                    int64_t idx = deliver_flat_index(vm, var,
                                                     ctr_used_index_regs,
                                                     &error);
                    if(idx < 0) /* error? */
                    {
                        /* do not use the set_error here, the string was allocated
                           copying it would be a memory leak*/
                        vm->error_description = error;
                        return NAP_FAILURE;
                    }
                    else
                    {
                        /* casting it to nap_int_t is ok, since
                         * var->instantiation->type == OPCODE_INT so we have
                         * allocated nap_int_t variable in the grow */
                        ((nap_byte_t*)var->instantiation->value)[idx] = nap_regb(vm, register_index);
                    }
                }
            }
            else
            if(register_type == OPCODE_REAL)
            {
                /* moving only if this real register goes to a real var*/
                if(var->instantiation->type == OPCODE_REAL) /* mov real var [x,y,z], reg real (register_index) */
                {
                    char* error = NULL;
                    int64_t idx = deliver_flat_index(vm, var,
                                                     ctr_used_index_regs,
                                                     &error);
                    if(idx < 0) /* error? */
                    {
                        /* do not use the set_error here, the string was allocated
                           copying it would be a memory leak*/
                        vm->error_description = error;
                        return NAP_FAILURE;
                    }
                    else
                    {
                        /* casting it to nap_int_t is ok, since
                         * var->instantiation->type == OPCODE_INT so we have
                         * allocated nap_int_t variable in the grow */
                        ((nap_real_t*)var->instantiation->value)[idx] = nap_regr(vm, register_index);
                    }
                }
            }
            else
            {
                NAP_NOT_IMPLEMENTED
            }
        }
        else
        {
            /* moving an immediate value/variable into an index destination */
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    {
        /* moving into something other indexed, than a variable */
        NAP_NOT_IMPLEMENTED
    }

    return NAP_SUCCESS;
}

int nap_mov(struct nap_vm* vm)
{
    uint8_t mov_target = vm->content[nap_step_ip(vm)];   /* where we move (reg, var)*/
    if(vm->mov_handlers[mov_target]) /* do we have a handler for this target? */
    {
        return vm->mov_handlers[mov_target](vm);
    }
    else /* check the user provided mov handlers */
    {
        NAP_NOT_IMPLEMENTED;
    }
}
