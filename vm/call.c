#include "call.h"
#include "nbci.h"
#include "jmptable.h"
#include "nbci_impl.h"
#include "nap_consts.h"
#include "funtable.h"
#include "opcodes.h"

#include <stdlib.h>
#include <string.h>

int nap_call(struct nap_vm *vm)
{
    nap_addr_t jmpt_index = nap_fetch_address(vm);

    if(vm->jumptable[jmpt_index]->type != JMPTABLE_ENTRY_TYPE_FROM_PARENT)
    {
        /* can we create a new location? */
        if(vm->cfsize == DEEPEST_RECURSION)
        {
            return NAP_FAILURE;
        }
        vm->call_frames[vm->cfsize ++] = vm->cc;

        /* is this a valid jump index? */
        if(jmpt_index >= vm->jumptable_size)
        {
            char s[256];
            SNPRINTF(s, 256, "Invalid jump index [%d]."
                     " Max is ["JL_SIZE_T_SPECIFIER"]",
                     jmpt_index, vm->jumptable_size);
            nap_vm_set_error_description(vm, s);
            return NAP_FAILURE;
        }
        /* and simply set cc to be where we need to go */
        vm->cc = vm->jumptable[jmpt_index]->location;
    }
    else /* calling a method from the parent_vm */
    {
        int64_t parent_sp = vm->parent->stack_pointer;
        int i = 0;

        /* this will be definitely over the max amount allowed,
         * so the main while loop of nap_vm_run will exit*/
        uint64_t fake_call_frame_exit = (uint64_t)-1;

        /* get the function from the parent*/
        struct funtable_entry* fe = nap_vm_get_method(vm->parent, vm->jumptable[jmpt_index]->label_name);
        if(fe == NULL)
        {
            return NAP_FAILURE;
        }

        /* patch the stack of the parent */
        for(i=0; i<=vm->stack_pointer; i++)
        {
            vm->parent->stack[parent_sp + i + 1] = vm->stack[i];
            vm->parent->stack_pointer ++;
        }

        /* then set the parent's cc to the method*/
        vm->parent->cc = vm->parent->jumptable[fe->jmptable_index]->location;

        /* patch the call frames of the parent so that it will give invalid value
         * but will not affect the correct functionality */
        vm->parent->call_frames[vm->parent->cfsize ++] = fake_call_frame_exit;

        /* and run the method with the patched stack */
        nap_vm_run(vm->parent);

        /* restore the parent's stack_pointer */
        vm->parent->stack_pointer = parent_sp;

        /* fetch over the return values, they might be used by the caller later*/
        vm->rvb = vm->parent->rvb;
        vm->rvi = vm->parent->rvi;
        vm->rvr = vm->parent->rvr;
        vm->rvl = vm->parent->rvl;
        if(vm->parent->rvl)
        {
            NAP_MEM_FREE(vm->rvs);
            NAP_STRING_ALLOC(vm, vm->rvs, vm->parent->rvl);
            NAP_STRING_COPY(vm->rvs, vm->parent->rvs, vm->rvl);
        }
    }

    return NAP_SUCCESS;
}
