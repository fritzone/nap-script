#include "intr_3.h"
#include "nbci.h"
#include "nap_consts.h"
#include "nbci_impl.h"

#include <stdlib.h>

uint16_t intr_3(struct nap_vm* vm)
{
    struct nap_vm* child_vm = NULL;
    struct startup_configuration* config = (struct startup_configuration*)calloc(1, sizeof(struct startup_configuration));
    config->stack_size = STACK_INIT;
    config->deepest_recursion = DEEPEST_RECURSION;
    nap_int_t regi0 = nap_regi(vm, 0);
    child_vm = nap_vm_inject(
                vm->btyecode_chunks[regi0]->code,
                vm->btyecode_chunks[regi0]->length,
                INTERRUPT, config);

    if(child_vm == NULL)
    {
        nap_set_error(vm, ERR_VM_0003);
        return INTR_3_CANNOT_CREATE_VM;
    }

    /* no errors, run the new VM  */
    child_vm->parent = vm;
    nap_vm_run(child_vm);
    if(child_vm->error_code != 0) /* any errors? */
    {
        /* take them over here */
        vm->error_code = child_vm->error_code;
        vm->error_message = child_vm->error_message;

        /* cleanup */
        nap_vm_cleanup(child_vm);

        return INTR_3_COULD_NOT_RUN_CODE;
    }

    /* performs the cleanup */
    nap_vm_cleanup(child_vm);
    return NAP_SUCCESS;
}
