#include "call.h"
#include "nbci.h"
#include "jmptable.h"

void nap_call(struct nap_vm *vm)
{
    uint32_t* p_jmpt_index = (uint32_t*)(vm->content + vm->cc);
    vm->cc += sizeof(uint32_t);
    vm->call_frames[vm->cfsize ++] = vm->cc;

    /* and simply set cc to be where we need to go */
    vm->cc = vm->jumptable[*p_jmpt_index]->location;
}
