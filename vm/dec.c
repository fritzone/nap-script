#include "dec.h"
#include "nbci.h"
#include "metatbl.h"
#include "opcodes.h"
#include "stack.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_dec(struct nap_vm* vm)
{
    uint8_t inc_what = vm->content[vm->cc ++]; /* variable, register*/
    if(inc_what == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);
        struct variable_entry* ve = nap_fetch_variable(vm, var_index);
        ASSERT_NOT_NULL_VAR(ve)

        if(ve->instantiation->type == OPCODE_INT)
        {
            (*(nap_int_t*)ve->instantiation->value) --;
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
