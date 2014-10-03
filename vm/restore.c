#include "restore.h"
#include "nbci.h"
#include "metatbl.h"
#include "opcodes.h"
#include "stack.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_restore(struct nap_vm* vm)
{
    uint8_t store_what = vm->content[nap_step_ip(vm)]; /* variable, register, indexed*/
    if(store_what == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);
        int64_t tempst = nap_sp(vm) + 1; /* The first stored element may start 1 above the current SP */
        int64_t save_tempst = tempst;    /* This will be the actual element */
        struct stack_entry* se = NULL;   /* The new stack entry */
        ASSERT_NOT_NULL_VAR(ve)
        CHECK_VARIABLE_INSTANTIATON(ve)

        while(vm->cec->stack[tempst] && vm->cec->stack[tempst]->stored)
        {
            save_tempst = tempst;
            tempst ++;
        }

        /* Now simply throw away the ve's instantiation and take over the one
         * from the stored variable */

        if(ve->instantiation)
        {
            if(ve->instantiation->value)
            {
                NAP_MEM_FREE(ve->instantiation->value);
            }

            NAP_MEM_FREE(ve->instantiation);
        }

        ve->data_size = vm->cec->stack[save_tempst]->var_def->data_size;
        ve->dimension_count = vm->cec->stack[save_tempst]->var_def->dimension_count;
        memcpy(ve->dimensions, vm->cec->stack[save_tempst]->var_def->dimensions, 256 * sizeof(nap_int_t));

        /* create a new stack entry */
        se = NAP_MEM_ALLOC(1, struct stack_entry);
        NAP_NN_ASSERT(vm, se);
        se->len = vm->cec->stack[save_tempst]->var_def->instantiation->len;
        se->type = vm->cec->stack[save_tempst]->var_def->instantiation->type;
        se->value = calloc(vm->cec->stack[save_tempst]->var_def->data_size * se->len, 1);
        memcpy(se->value, vm->cec->stack[save_tempst]->var_def->instantiation->value,
               vm->cec->stack[save_tempst]->var_def->data_size * se->len);

        ve->instantiation = se;

        /* And free the last stack entry */
        NAP_MEM_FREE(vm->cec->stack[save_tempst]);
        vm->cec->stack[save_tempst] = NULL;

        return NAP_SUCCESS;
    }
    return NAP_FAILURE;

}
