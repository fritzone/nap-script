#include "leave.h"
#include "nbci.h"
#include "nbci_impl.h"
#include "nap_consts.h"

int nap_leave(struct nap_vm *vm)
{
    if(vm->cec->cfsize == (uint32_t)(-1))
    {
        return NAP_FAILURE;
    }
    nap_set_ip(vm, vm->cec->call_frames[-- vm->cec->cfsize]);
    return NAP_SUCCESS;
}
