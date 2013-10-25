#include "jump.h"
#include "opcodes.h"
#include "nbci.h"
#include "jmptable.h"

void nap_jump(struct nap_vm *vm)
{
    uint32_t* p_jmpt_index = (uint32_t*)(vm->content + vm->cc);

    /* and simply set cc to be where we need to go */
    if(vm->current_opcode == OPCODE_JMP)
    {
        vm->cc = vm->jumptable[*p_jmpt_index]->location;
    }
    else
    {
        if(vm->lbf)
        {
            vm->lbf = 0;
            vm->cc = vm->jumptable[*p_jmpt_index]->location;
        }
        else
        {
            vm->cc += sizeof(uint32_t);
        }
    }
}
