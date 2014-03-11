#include "jump.h"
#include "opcodes.h"
#include "nbci.h"
#include "nbci_impl.h"
#include "jmptable.h"
#include "nap_consts.h"
#include "byte_order.h"

int nap_jump(struct nap_vm *vm)
{
    nap_index_t* p_jmpt_index = (nap_index_t*)(vm->content + vm->cc);
    nap_index_t jmp_idx = htovm_32(*p_jmpt_index);

    /* is this a valid jump index? */
    if(jmp_idx >= vm->jumptable_size)
    {
        return NAP_FAILURE;
    }

    /* and simply set cc to be where we need to go */
    if(vm->current_opcode == OPCODE_JMP)
    {
        vm->cc = vm->jumptable[jmp_idx]->location;
    }
    else
    {
        if(vm->lbf)
        {
            vm->cc = vm->jumptable[jmp_idx]->location;
        }
        else
        {
            vm->cc += sizeof(nap_index_t);
        }
        vm->lbf = UNDECIDED;
    }

    return NAP_SUCCESS;
}
