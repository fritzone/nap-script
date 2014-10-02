#include "restore.h"
#include "nbci.h"
#include "metatbl.h"
#include "opcodes.h"
#include "stack.h"
#include "nbci_impl.h"
#include "nap_consts.h"

int nap_restore(struct nap_vm* vm)
{
    uint8_t store_what = vm->content[nap_step_ip(vm)]; /* variable, register, indexed*/
    if(store_what == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);

        ASSERT_NOT_NULL_VAR(ve)
        CHECK_VARIABLE_INSTANTIATON(ve)

        /* Now parse the stack and fetch the *last* stored value from above the stack_sp
         * and remove that from the stack */
        return NAP_SUCCESS;
    }
    return NAP_FAILURE;

}
