#include "call.h"
#include "nbci.h"
#include "jmptable.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_call(struct nap_vm *vm)
{
    nap_addr_t jmpt_index = nap_fetch_address(vm);

    /* can we create a new location? */
    if(vm->cfsize == DEEPEST_RECURSION)
    {
        return NAP_FAILURE;
    }
    vm->call_frames[vm->cfsize ++] = vm->cc;

    /* is this a valid jump index? */
    if(jmpt_index >= vm->jumptable_size)
    {
        char* s = (char*)calloc(256, sizeof(char));
        SNPRINTF(s, 256, "Invalid jump index [%d]."
                 " Max is ["JL_SIZE_T_SPECIFIER"]",
                 jmpt_index, vm->jumptable_size);
        vm->error_description = s;

        return NAP_FAILURE;
    }
    /* and simply set cc to be where we need to go */
    vm->cc = vm->jumptable[jmpt_index]->location;

    return NAP_SUCCESS;
}
