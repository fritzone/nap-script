#include "intr_3.h"
#include "nbci.h"

uint8_t intr_3(struct nap_vm* vm)
{
    struct nap_vm* child_vm = NULL;
    child_vm = nap_vm_inject(vm->btyecode_chunks[vm->regi[0]]->code,
            vm->btyecode_chunks[vm->regi[0]]->length);
    child_vm->parent = vm;
    nap_vm_run(child_vm);
    nap_vm_cleanup(child_vm);
    return 0;
}
