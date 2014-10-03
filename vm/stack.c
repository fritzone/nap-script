#include "stack.h"
#include "nbci_impl.h"
#include "nbci.h"

void relocate_stored_elements(struct nap_vm* vm)
{
    int64_t tempst = nap_sp(vm) + 1;
    while(vm->cec->stack[tempst] && vm->cec->stack[tempst]->stored)
    {
        vm->cec->stack[tempst - 1] = vm->cec->stack[tempst];
        vm->cec->stack[tempst] = NULL;
        tempst ++;
    }
}
