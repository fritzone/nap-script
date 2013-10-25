#include "clidx.h"
#include "nbci.h"

#include <string.h>

void nap_clidx(struct nap_vm *vm)
{
    memset(vm->regidx, 0, sizeof(vm->regidx));
}
