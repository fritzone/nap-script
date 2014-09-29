#include "comparison.h"
#include "opcodes.h"
#include "nbci.h"
#include "metatbl.h"
#include "stack.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>
#include <stdint.h>

/**
 * Sets the last boolean flag according to the operation found int current_opcode
 * ie. as a result between the operation performed on the two integer type
 * parameters.
 *
 * @param vm - the virtual machine
 * @param first - the first operand compared
 * @param second - against this value
 * @param current_opcode - the operation which is supposed to be executed
 *
 * @return NAP_SUCCESS/NAP_FAILURE depending on the state of the operation.
 */
static int nap_vm_set_lbf_to_op_result_int(struct nap_vm* vm, nap_int_t first,
                                        nap_int_t second, uint8_t opcode)
{
    uint8_t temp_lbf;

    if(opcode == OPCODE_EQ)
    {
        temp_lbf = (first == second);
    }
    else
    if(opcode == OPCODE_NEQ)
    {
        temp_lbf = (first != second);
    }
    else
    if(opcode == OPCODE_LT)
    {
        temp_lbf = (first <  second);
    }
    else
    if(opcode == OPCODE_GT)
    {
        temp_lbf = (first >  second);
    }
    else
    if(opcode == OPCODE_LTE)
    {
        temp_lbf = (first <= second);
    }
    else
    if(opcode == OPCODE_GTE)
    {
        temp_lbf = (first >= second);
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }

    if(vm->cec->lbf == UNDECIDED)
    {
        vm->cec->lbf = temp_lbf;
    }
    else
    {
        vm->cec->lbf &= temp_lbf;
    }

    return NAP_SUCCESS;
}

/**
 * Sets the last boolean flag according to the operation found int current_opcode
 * ie. as a result between the operation performed on the two byte type
 * parameters.
 *
 * @param vm - the virtual machine
 * @param first - the first operand compared
 * @param second - against this value
 * @param current_opcode - the operation which is supposed to be executed
 *
 * @return NAP_SUCCESS/NAP_FAILURE depending on the state of the operation.
 */
static int nap_vm_set_lbf_to_op_result_byte(struct nap_vm* vm, nap_byte_t first,
                                        nap_byte_t second, uint8_t opcode)
{
    uint8_t temp_lbf;

    if(opcode == OPCODE_EQ)
    {
        temp_lbf = (first == second);
    }
    else
    if(opcode == OPCODE_NEQ)
    {
        temp_lbf = (first != second);
    }
    else
    if(opcode == OPCODE_LT)
    {
        temp_lbf = (first <  second);
    }
    else
    if(opcode == OPCODE_GT)
    {
        temp_lbf = (first >  second);
    }
    else
    if(opcode == OPCODE_LTE)
    {
        temp_lbf = (first <= second);
    }
    else
    if(opcode == OPCODE_GTE)
    {
        temp_lbf = (first >= second);
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }

    if(vm->cec->lbf == UNDECIDED)
    {
        vm->cec->lbf = temp_lbf;
    }
    else
    {
        vm->cec->lbf &= temp_lbf;
    }

    return NAP_SUCCESS;
}

/**
 * Sets the last boolean flag according to the operation found int current_opcode
 * ie. as a result between the operation performed on the two real type
 * parameters.
 *
 * @param vm - the virtual machine
 * @param first - the first operand compared
 * @param second - against this value
 * @param current_opcode - the operation which is supposed to be executed
 *
 * @return NAP_SUCCESS/NAP_FAILURE depending on the state of the operation.
 */
static int nap_vm_set_lbf_to_op_result_real(struct nap_vm* vm, nap_real_t first,
                                        nap_real_t second, uint8_t opcode)
{
    uint8_t temp_lbf;

    if(opcode == OPCODE_EQ)
    {
        temp_lbf = (first - second < REAL_COMPARISON_DELTA);
    }
    else
    if(opcode == OPCODE_NEQ)
    {
        temp_lbf = (first - second > REAL_COMPARISON_DELTA);
    }
    else
    if(opcode == OPCODE_LT)
    {
        temp_lbf = (first <  second);
    }
    else
    if(opcode == OPCODE_GT)
    {
        temp_lbf = (first >  second);
    }
    else
    if(opcode == OPCODE_LTE)
    {
        temp_lbf = (first <= second);
    }
    else
    if(opcode == OPCODE_GTE)
    {
        temp_lbf = (first >= second);
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }

    if(vm->cec->lbf == UNDECIDED)
    {
        vm->cec->lbf = temp_lbf;
    }
    else
    {
        vm->cec->lbf &= temp_lbf;
    }

    return NAP_SUCCESS;
}


/* compares an integer register to something */
static int nap_compare_reg_int(struct nap_vm* vm)
{
    uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
    uint8_t cmp_second = vm->content[nap_step_ip(vm)]; /* what are we checking against*/

    if(cmp_second == OPCODE_IMMEDIATE_INT) /* comparing int register with immediate int value (1,..) */
    {
        int success = NAP_SUCCESS;
        nap_int_t immediate = nap_read_immediate_int(vm, &success);
        if(success == NAP_FAILURE)
        {
            return NAP_FAILURE;
        }

        return nap_vm_set_lbf_to_op_result_int(vm, nap_regi(vm, register_index),
                                           immediate, vm->cec->current_opcode);
    }
    else
    if(cmp_second == OPCODE_IMMEDIATE_REAL) /* comparing int register with immediate real value (1,..) */
    {
        nap_real_t immediate = nap_read_immediate_real(vm);
        return nap_vm_set_lbf_to_op_result_int(vm,
                                           nap_regi(vm, register_index),
                                           (nap_int_t)immediate, vm->cec->current_opcode);
    }
    else
    if(cmp_second == OPCODE_VAR)            /* int register compared to a variable */
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);

        ASSERT_NOT_NULL_VAR(ve);
        CHECK_VARIABLE_INSTANTIATON(ve);

        if(ve->instantiation->type == STACK_ENTRY_INT) /* comparing int register with an int variable */
        {
            return nap_vm_set_lbf_to_op_result_int(vm,
                                               nap_regi(vm, register_index),
                                               *(nap_int_t*)ve->instantiation->value,
                                               vm->cec->current_opcode);
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_BYTE) /* comparing int register with a byte variable */
        {
            return nap_vm_set_lbf_to_op_result_int(vm,
                                               nap_regi(vm, register_index),
                                               (nap_int_t)(*(nap_byte_t*)ve->instantiation->value),
                                               vm->cec->current_opcode);
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_REAL) /* comparing int register with a real variable */
        {
            return nap_vm_set_lbf_to_op_result_int(vm,
                                               nap_regi(vm, register_index),
                                               (nap_int_t)(*(nap_real_t*)ve->instantiation->value),
                                               vm->cec->current_opcode);
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_STRING)
        {
            int error = NAP_SUCCESS; /* might lose some numbers */
            nap_int_t v = nap_string_to_number_int(
                        vm, (nap_string_t)ve->instantiation->value,
                        ve->instantiation->len, &error);

            if(error == NAP_FAILURE)
            {
                return NAP_FAILURE;
            }

            return nap_vm_set_lbf_to_op_result_int(vm, nap_regi(vm, register_index),
                                               v, vm->cec->current_opcode);
        }

        else /* comparing int register with another kind of variable */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(cmp_second == OPCODE_REG)
    {
        uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        if(second_register_type == OPCODE_INT) /* comparing int reg with another INT type register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            return nap_vm_set_lbf_to_op_result_int(vm,
                                               nap_regi(vm, register_index),
                                               nap_regi(vm, second_register_index),
                                               vm->cec->current_opcode);
        }
        else
        if(second_register_type == OPCODE_BYTE) /* comparing int reg with a byte register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            return nap_vm_set_lbf_to_op_result_int(vm,
                                               nap_regi(vm, register_index),
                                               (nap_int_t)nap_regb(vm, second_register_index),
                                               vm->cec->current_opcode);
        }
        else
        if(second_register_type == OPCODE_REAL) /* comparing int reg with a real register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            return nap_vm_set_lbf_to_op_result_int(vm,
                                               nap_regi(vm, register_index),
                                               (nap_int_t)nap_regr(vm, second_register_index),
                                               vm->cec->current_opcode);
        }
        else
        if(second_register_type == OPCODE_STRING) /* comparing int reg with a string register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            int error = NAP_SUCCESS;
            int other_error = nap_vm_set_lbf_to_op_result_int(vm,
                                                   nap_regi(vm, register_index),
                                                   nap_string_to_number_int(vm,
                                                                            nap_regs(vm, second_register_index)->s,
                                                                            nap_regs(vm, second_register_index)->l,
                                                                            &error),
                                                   vm->cec->current_opcode);
            return error & other_error;
        }
        else /* comparing int reg with another type register */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(cmp_second == OPCODE_LEN) /* the length of a variable */
    {
        uint8_t length_of = vm->content[nap_step_ip(vm)];
        if(length_of == OPCODE_VAR)
        {
            nap_index_t var_index = nap_fetch_index(vm);
            /* and fetch the variable from the given index */
            struct variable_entry* var = nap_fetch_variable(vm, var_index);
            ASSERT_NOT_NULL_VAR(var)
            CHECK_VARIABLE_INSTANTIATON(var)

            /* and do the comparison */
            return nap_vm_set_lbf_to_op_result_int(vm,
                                                   nap_regi(vm, register_index),
                                                   var->instantiation->len,
                                                   vm->cec->current_opcode);
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(cmp_second == OPCODE_RV) /* cmping the return value of some function to a reg*/
    {
        uint8_t return_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(return_type == OPCODE_INT)                 /* handles: cmp reg int 0, rv int*/
        {
            return nap_vm_set_lbf_to_op_result_int(vm,
                                                   nap_regi(vm, register_index),
                                                   vm->cec->rvi,
                                                   vm->cec->current_opcode);
        }
        else
        if(return_type == OPCODE_STRING)              /* handles: cmp reg int 0, rv string*/
        {
            int error = NAP_SUCCESS;
            int other_error = nap_vm_set_lbf_to_op_result_int(vm,
                                                   nap_regi(vm, register_index),
                                                   nap_string_to_number_int(vm, vm->cec->rvs,
                                                                            vm->cec->rvl, &error),
                                                   vm->cec->current_opcode);
            return error & other_error;
        }
        else
        if(return_type == OPCODE_BYTE)                 /* handles: cmp reg int 0, rv byte*/
        {
            return nap_vm_set_lbf_to_op_result_int(vm,
                                                   nap_regi(vm, register_index),
                                                   (nap_int_t)vm->cec->rvb,
                                                   vm->cec->current_opcode);
        }
        else
        if(return_type == OPCODE_REAL)                 /* handles: cmp reg int 0, rv real*/
        {
            return nap_vm_set_lbf_to_op_result_int(vm,
                                                   nap_regi(vm, register_index),
                                                   (nap_int_t)vm->cec->rvr,
                                                   vm->cec->current_opcode);
        }
        else
        {
            NAP_NOT_IMPLEMENTED /* UNKNOWN return type */
        }
    }

    else /* comparing an int register to something else. What might that be? */
    {
        NAP_NOT_IMPLEMENTED
    }
}

/* compares a byte register to something */
static int nap_compare_reg_byte(struct nap_vm* vm)
{
    uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
    uint8_t cmp_second = vm->content[nap_step_ip(vm)]; /* what are we checking against*/

    if(cmp_second == OPCODE_IMMEDIATE_INT) /* comparing byte register with immediate int value (1,..) */
    {
        int success = NAP_SUCCESS;
        nap_byte_t immediate = (nap_byte_t)nap_read_immediate_int(vm, &success);
        if(success == NAP_FAILURE)
        {
            return NAP_FAILURE;
        }

        return nap_vm_set_lbf_to_op_result_byte(vm,
                                                nap_regb(vm, register_index),
                                                immediate,
                                                vm->cec->current_opcode);
    }
    else
    if(cmp_second == OPCODE_IMMEDIATE_REAL) /* comparing byte register with immediate real value (1,..) */
    {
        nap_byte_t immediate = (nap_byte_t)nap_read_immediate_real(vm);
        return nap_vm_set_lbf_to_op_result_byte(vm,
                                           nap_regb(vm, register_index),
                                           immediate, vm->cec->current_opcode);
    }
    else
    if(cmp_second == OPCODE_VAR)            /* byte register compared to a variable */
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);

        ASSERT_NOT_NULL_VAR(ve);
        CHECK_VARIABLE_INSTANTIATON(ve);

        if(ve->instantiation->type == STACK_ENTRY_INT) /* comparing byte register with an int variable */
        {
            return nap_vm_set_lbf_to_op_result_byte(vm,
                                               nap_regb(vm, register_index),
                                               (nap_byte_t)(*(nap_int_t*)ve->instantiation->value),
                                               vm->cec->current_opcode);
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_BYTE) /* comparing byte register with a byte variable */
        {
            return nap_vm_set_lbf_to_op_result_byte(vm,
                                               nap_regb(vm, register_index),
                                               *(nap_byte_t*)ve->instantiation->value,
                                               vm->cec->current_opcode);
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_REAL) /* comparing byte register with a real variable */
        {
            return nap_vm_set_lbf_to_op_result_byte(vm,
                                               nap_regb(vm, register_index),
                                               (nap_byte_t)(*(nap_real_t*)ve->instantiation->value),
                                               vm->cec->current_opcode);
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_STRING)
        {
            int error = NAP_SUCCESS; /* might lose some numbers */
            nap_byte_t v = (nap_byte_t)nap_string_to_number_int(
                        vm, (nap_string_t)ve->instantiation->value,
                        ve->instantiation->len, &error);

            if(error == NAP_FAILURE)
            {
                return NAP_FAILURE;
            }

            return nap_vm_set_lbf_to_op_result_byte(vm,
                                                    nap_regb(vm, register_index),
                                                    v, vm->cec->current_opcode);
        }

        else /* comparing int register with another kind of variable */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(cmp_second == OPCODE_REG)
    {
        uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        if(second_register_type == OPCODE_INT) /* comparing byte reg with INT type register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            return nap_vm_set_lbf_to_op_result_byte(vm,
                                               nap_regb(vm, register_index),
                                               (nap_byte_t)nap_regi(vm, second_register_index),
                                               vm->cec->current_opcode);
        }
        else
        if(second_register_type == OPCODE_BYTE) /* comparing byte reg with a byte register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            return nap_vm_set_lbf_to_op_result_byte(vm,
                                               nap_regb(vm, register_index),
                                               nap_regb(vm, second_register_index),
                                               vm->cec->current_opcode);
        }
        else
        if(second_register_type == OPCODE_REAL) /* comparing byte reg with a real register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            return nap_vm_set_lbf_to_op_result_byte(vm,
                                               nap_regb(vm, register_index),
                                               (nap_byte_t)nap_regr(vm, second_register_index),
                                               vm->cec->current_opcode);
        }
        else
        if(second_register_type == OPCODE_STRING) /* comparing byte reg with a string register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            int error = NAP_SUCCESS;
            int other_error = nap_vm_set_lbf_to_op_result_byte(vm,
                                                   nap_regb(vm, register_index),
                                                   (nap_byte_t)nap_string_to_number_int(vm,
                                                                            nap_regs(vm, second_register_index)->s,
                                                                            nap_regs(vm, second_register_index)->l,
                                                                            &error),
                                                   vm->cec->current_opcode);
            return error & other_error;
        }
        else /* comparing int reg with another type register */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(cmp_second == OPCODE_RV) /* cmping the return value of some function to a reg*/
    {
        uint8_t return_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(return_type == OPCODE_INT)                 /* handles: cmp reg byte 0, rv int*/
        {
            return nap_vm_set_lbf_to_op_result_byte(vm,
                                                   nap_regb(vm, register_index),
                                                   (nap_byte_t)vm->cec->rvi,
                                                   vm->cec->current_opcode);
        }
        else
        if(return_type == OPCODE_STRING)              /* handles: cmp reg byte 0, rv string*/
        {
            int error = NAP_SUCCESS;
            int other_error = nap_vm_set_lbf_to_op_result_byte(vm,
                                                   nap_regb(vm, register_index),
                                                   (nap_byte_t)nap_string_to_number_int(vm, vm->cec->rvs,
                                                                            vm->cec->rvl, &error),
                                                   vm->cec->current_opcode);
            return error & other_error;
        }
        else
        if(return_type == OPCODE_BYTE)                 /* handles: cmp reg byte 0, rv byte*/
        {
            return nap_vm_set_lbf_to_op_result_byte(vm,
                                                   nap_regb(vm, register_index),
                                                   vm->cec->rvb,
                                                   vm->cec->current_opcode);
        }
        else
        if(return_type == OPCODE_REAL)                 /* handles: cmp reg byte 0, rv real*/
        {
            return nap_vm_set_lbf_to_op_result_int(vm,
                                                   nap_regb(vm, register_index),
                                                   (nap_byte_t)vm->cec->rvr,
                                                   vm->cec->current_opcode);
        }
        else
        {
            NAP_NOT_IMPLEMENTED /* UNKNOWN return type */
        }
    }

    else /* comparing a byte register to something else. What might that be? */
    {
        NAP_NOT_IMPLEMENTED
    }
}

/* compares a real register to something */
static int nap_compare_reg_real(struct nap_vm* vm)
{
    uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
    uint8_t cmp_second = vm->content[nap_step_ip(vm)]; /* what are we checking against*/

    if(cmp_second == OPCODE_IMMEDIATE_INT) /* comparing real register with immediate int value (1,..) */
    {
        int success = NAP_SUCCESS;
        nap_real_t immediate = (nap_real_t)nap_read_immediate_int(vm, &success);
        if(success == NAP_FAILURE)
        {
            return NAP_FAILURE;
        }

        return nap_vm_set_lbf_to_op_result_real(vm,
                                                nap_regr(vm, register_index),
                                                immediate,
                                                vm->cec->current_opcode);
    }
    else
    if(cmp_second == OPCODE_IMMEDIATE_REAL) /* comparing real register with immediate real value (1,..) */
    {
        nap_real_t immediate = nap_read_immediate_real(vm);
        return nap_vm_set_lbf_to_op_result_real(vm,
                                           nap_regr(vm, register_index),
                                           immediate, vm->cec->current_opcode);
    }
    else
    if(cmp_second == OPCODE_VAR)            /* real register compared to a variable */
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);

        ASSERT_NOT_NULL_VAR(ve);
        CHECK_VARIABLE_INSTANTIATON(ve);

        if(ve->instantiation->type == STACK_ENTRY_INT) /* comparing real register with an int variable */
        {
            return nap_vm_set_lbf_to_op_result_real(vm,
                                               nap_regr(vm, register_index),
                                               (nap_real_t)(*(nap_int_t*)ve->instantiation->value),
                                               vm->cec->current_opcode);
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_BYTE) /* comparing real register with a byte variable */
        {
            return nap_vm_set_lbf_to_op_result_real(vm,
                                               nap_regr(vm, register_index),
                                               (nap_real_t)(*(nap_byte_t*)ve->instantiation->value),
                                               vm->cec->current_opcode);
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_REAL) /* comparing real register with a real variable */
        {
            return nap_vm_set_lbf_to_op_result_real(vm,
                                               nap_regr(vm, register_index),
                                               *(nap_real_t*)ve->instantiation->value,
                                               vm->cec->current_opcode);
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_STRING)
        {
            int error = NAP_SUCCESS; /* might lose some numbers */
            nap_byte_t v = (nap_byte_t)nap_string_to_number_real(
                        vm, (nap_string_t)ve->instantiation->value,
                        ve->instantiation->len, &error);

            if(error == NAP_FAILURE)
            {
                return NAP_FAILURE;
            }

            return nap_vm_set_lbf_to_op_result_real(vm,
                                                    nap_regr(vm, register_index),
                                                    v, vm->cec->current_opcode);
        }

        else /* comparing int register with another kind of variable */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(cmp_second == OPCODE_REG)
    {
        uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        if(second_register_type == OPCODE_INT) /* comparing real reg with INT type register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            return nap_vm_set_lbf_to_op_result_real(vm,
                                               nap_regr(vm, register_index),
                                               (nap_real_t)nap_regi(vm, second_register_index),
                                               vm->cec->current_opcode);
        }
        else
        if(second_register_type == OPCODE_BYTE) /* comparing real reg with a byte register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            return nap_vm_set_lbf_to_op_result_real(vm,
                                               nap_regr(vm, register_index),
                                               (nap_real_t)nap_regb(vm, second_register_index),
                                               vm->cec->current_opcode);
        }
        else
        if(second_register_type == OPCODE_REAL) /* comparing real reg with a real register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            return nap_vm_set_lbf_to_op_result_real(vm,
                                               nap_regr(vm, register_index),
                                               nap_regr(vm, second_register_index),
                                               vm->cec->current_opcode);
        }
        else
        if(second_register_type == OPCODE_STRING) /* comparing real reg with a string register */
        {
            uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
            int error = NAP_SUCCESS;
            int other_error = nap_vm_set_lbf_to_op_result_real(vm,
                                                   nap_regr(vm, register_index),
                                                   nap_string_to_number_real(vm,
                                                                             nap_regs(vm, second_register_index)->s,
                                                                             nap_regs(vm, second_register_index)->l,
                                                                             &error),
                                                   vm->cec->current_opcode);
            return error & other_error;
        }
        else /* comparing int reg with another type register */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(cmp_second == OPCODE_RV) /* cmping the return value of some function to a reg*/
    {
        uint8_t return_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        if(return_type == OPCODE_INT)                 /* handles: cmp reg real 0, rv int*/
        {
            return nap_vm_set_lbf_to_op_result_real(vm,
                                                   nap_regr(vm, register_index),
                                                   (nap_real_t)vm->cec->rvi,
                                                   vm->cec->current_opcode);
        }
        else
        if(return_type == OPCODE_STRING)              /* handles: cmp reg real 0, rv string*/
        {
            int error = NAP_SUCCESS;
            int other_error = nap_vm_set_lbf_to_op_result_real(vm,
                                                   nap_regr(vm, register_index),
                                                   nap_string_to_number_real(vm, vm->cec->rvs,
                                                                            vm->cec->rvl, &error),
                                                   vm->cec->current_opcode);
            return error & other_error;
        }
        else
        if(return_type == OPCODE_BYTE)                 /* handles: cmp reg real 0, rv byte*/
        {
            return nap_vm_set_lbf_to_op_result_real(vm,
                                                   nap_regr(vm, register_index),
                                                   (nap_real_t)vm->cec->rvb,
                                                   vm->cec->current_opcode);
        }
        else
        if(return_type == OPCODE_REAL)                 /* handles: cmp reg real 0, rv real*/
        {
            return nap_vm_set_lbf_to_op_result_int(vm,
                                                   (nap_int_t)nap_regr(vm, register_index),
                                                   (nap_int_t)vm->cec->rvr,
                                                   vm->cec->current_opcode);
        }
        else
        {
            NAP_NOT_IMPLEMENTED /* UNKNOWN return type */
        }
    }

    else /* comparing a byte register to something else. What might that be? */
    {
        NAP_NOT_IMPLEMENTED
    }
}

int nap_comparison(struct nap_vm* vm)
{
    uint8_t cmp_first = vm->content[nap_step_ip(vm)];   /* what to check (reg only)*/

    if(cmp_first == OPCODE_REG) /* do we check a register? */
    {
        uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

        if(register_type == OPCODE_INT)
        {
            nap_compare_reg_int(vm);
        }
        else
        if(register_type == OPCODE_BYTE)
        {
            nap_compare_reg_byte(vm);
        }
        else
        if(register_type == OPCODE_REAL)
        {
            nap_compare_reg_real(vm);
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    if(cmp_first == OPCODE_VAR) /* do we compare a variable to something? */
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(ve)

        if(ve->instantiation->type == OPCODE_INT) /* comparing int variable to something */
        {
            uint8_t cmp_second = vm->content[nap_step_ip(vm)]; /* what are we checking against*/
            if(cmp_second == OPCODE_VAR) /* int variable compared to some variable */
            {
                nap_index_t second_var_index = nap_fetch_index(vm);
                struct variable_entry* second_ve = nap_fetch_variable(vm, second_var_index);
                ASSERT_NOT_NULL_VAR(second_ve)

                if(second_ve->instantiation->type == OPCODE_INT) /* comparing int register with an int variable */
                {
                    return nap_vm_set_lbf_to_op_result_int(vm, *(nap_int_t*)ve->instantiation->value,
                                                *(nap_int_t*)second_ve->instantiation->value,
                                                vm->cec->current_opcode);
                }
                else /* comparing int variable with another kind of variable */
                {
                    NAP_NOT_IMPLEMENTED
                }
            }
            else
            if(cmp_second == OPCODE_REG) /* comparing int variable against a register */
            {
                uint8_t second_register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/

                if(second_register_type == OPCODE_INT) /* comparing int reg with another INT type register */
                {
                    uint8_t second_register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/
                    return nap_vm_set_lbf_to_op_result_int(vm,
                                                       *(nap_int_t*)ve->instantiation->value,
                                                       nap_regi(vm, second_register_index),
                                                       vm->cec->current_opcode);
                }
                else /* comparing int reg with another type register */
                {
                    NAP_NOT_IMPLEMENTED
                }
            }
        }
        else /* comparing an another type of variable */
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    {
        return NAP_FAILURE;
    }
    return NAP_SUCCESS;
}
