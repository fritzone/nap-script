#include "clidx.h"
#include "nbci.h"
#include "nap_consts.h"

#include <string.h>

int nap_clidx(struct nap_vm *vm)
{
    memset(vm->cec->regidx, 0, sizeof(vm->cec->regidx));
    return NAP_SUCCESS;
}
