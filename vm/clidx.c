#include "clidx.h"
#include "nbci.h"
#include "nap_consts.h"

#include <string.h>

int nap_clidx(struct nap_vm *vm)
{
    memset(vm->regidx, 0, sizeof(vm->regidx));
    return NAP_SUCCESS;
}
