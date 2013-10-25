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
        uint32_t* p_var_index = (uint32_t*)(vm->content + vm->cc);
        struct variable_entry* ve = vm->metatable[*p_var_index];
        vm->cc += sizeof(uint32_t);

        if(ve->instantiation->type == OPCODE_INT)
        {
            (*(int64_t*)ve->instantiation->value) --;
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
