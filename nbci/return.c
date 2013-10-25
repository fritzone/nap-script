#include "return.h"
#include "nbci.h"

void nap_return(struct nap_vm *vm)
{
    vm->cc = vm->call_frames[-- vm->cfsize];
}
