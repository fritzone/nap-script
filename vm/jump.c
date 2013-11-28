#include "jump.h"
#include "opcodes.h"
#include "nbci.h"
#include "jmptable.h"

#include "byte_order.h"

void nap_jump(struct nap_vm *vm)
{
    nap_index_t* p_jmpt_index = (nap_index_t*)(vm->content + vm->cc);
    nap_index_t jmp_idx = htovm_32(*p_jmpt_index);

    /* and simply set cc to be where we need to go */
    if(vm->current_opcode == OPCODE_JMP)
    {
        vm->cc = vm->jumptable[jmp_idx]->location;
    }
    else
    {
        if(vm->lbf)
        {
            vm->lbf = UNDECIDED;
            vm->cc = vm->jumptable[jmp_idx]->location;
        }
        else
        {
            vm->lbf = UNDECIDED;
            vm->cc += sizeof(nap_index_t);
        }
    }
}
