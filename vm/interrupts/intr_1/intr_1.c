#include "intr_1.h"
#include <nap_consts.h>

#include <nbci.h>
#include <nbci_impl.h>

uint16_t intr_1(struct nap_vm *vm)
{
    /* peek the last value on stack. It contains the number of parameters that were passed in. */
    vm = vm;
    return NAP_SUCCESS;
}
