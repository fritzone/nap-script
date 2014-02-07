#include "funtable.h"
#include "nap_consts.h"
#include "nbci.h"

int interpret_funtable(struct nap_vm *vm, uint8_t *start_location, uint32_t len)
{
    uint8_t* cloc = start_location + 4; /* skip the .fun TODO :check if it is .fun */

    /* number of functions */
    size_t count = htovm_32(*(uint32_t*)(cloc));
    if(count == 0)
    {
        return NAP_SUCCESS;
    }
    vm->funtable_entries = count;
    vm->funtable = (struct funtable_entry**) calloc(vm->funtable_size + 1,
                                              sizeof(struct funtable_entry*));
    if(vm->jumptable == NULL)
    {
        vm->jumptable_size = 0;
        return NAP_FAILURE;
    }


    cloc += 4;

    return NAP_SUCCESS;
}
