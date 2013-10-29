#include "dec.h"
#include "nbci.h"
#include "metatbl.h"
#include "opcodes.h"
#include "stack.h"

#include <stdlib.h>

void nap_dec(struct nap_vm* vm)
{
    uint8_t inc_what = vm->content[vm->cc ++]; /* variable, register*/
    if(inc_what == OPCODE_VAR)
    {
        nap_index_t var_index = nap_fetch_index(vm);

        struct variable_entry* ve = vm->metatable[var_index];

        if(ve->instantiation->type == OPCODE_INT)
        {
            (*(nap_number_t*)ve->instantiation->value) --;
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
