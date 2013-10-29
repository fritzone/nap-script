#include "call.h"
#include "nbci.h"
#include "jmptable.h"

void nap_call(struct nap_vm *vm)
{
    nap_addr_t jmpt_index = nap_fetch_address(vm);

    vm->call_frames[vm->cfsize ++] = vm->cc;

    /* and simply set cc to be where we need to go */
    vm->cc = vm->jumptable[jmpt_index]->location;
}
