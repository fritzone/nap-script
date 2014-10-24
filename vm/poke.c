#include "poke.h"
#include "nbci.h"
#include "opcodes.h"
#include "stack.h"
#include "metatbl.h"
#include "nbci_impl.h"
#include "nap_consts.h"

int nap_poke(struct nap_vm* vm)
{
    uint8_t poke_target = 0;    /* this normally is OPCODE_VAR for compiled code*/

    poke_target = vm->content[nap_step_ip(vm)];
    if(poke_target == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(ve);
        /* there supposed to be no instantiation at this point for the var ...
         * but if there is an instantiation this means, a recursive (or a second)
         * call of the method has happened */
        //pop_variable_instantiation(vm, ve);
        return NAP_SUCCESS;
    }
    return NAP_FAILURE;
}
