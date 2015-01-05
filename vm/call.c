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

    /* is this a valid jump index? */
    if(jmpt_index >= vm->jumptable_size)
    {
		char s[256] = {0};
        SNPRINTF(s, MAX_BUF_SIZE(255), "Invalid jump index [%d]."
                 " Max is ["JL_SIZE_T_SPECIFIER"]",
                 jmpt_index, vm->jumptable_size);
        nap_vm_set_error_description(vm, s);
        return NAP_FAILURE;
    }

    if(vm->jumptable[jmpt_index]->type != JMPTABLE_ENTRY_TYPE_FROM_PARENT)
    {
        /* can we create a new location? */
        if(vm->cec->cfsize == vm->config->deepest_recursion)
        {
            return NAP_FAILURE;
        }
        vm->cec->call_frames[vm->cec->cfsize ++] = nap_ip(vm);

        /* behind the scenes: init BP to be the current SP */
        vm->cec->bp = vm->cec->stack_pointer;

        /* and simply set cc to be where we need to go */
        nap_set_ip(vm, vm->jumptable[jmpt_index]->location);
    }
    else /* calling a method from the parent_vm */
    {
        int64_t parent_sp = vm->parent->cec->stack_pointer;
        int i = 0;

        /* this will be definitely over the max amount allowed,
         * so the main while loop of nap_vm_run will exit*/
        uint64_t fake_call_frame_exit = (uint64_t)-1;

        /* get the function from the parent*/
        struct funtable_entry* fe = nap_vm_get_method(vm->parent, 
                                        vm->jumptable[jmpt_index]->label_name);
        if(fe == NULL)
        {
            return NAP_FAILURE;
        }

        /* patch the stack of the parent */
        for(i=0; i<=nap_sp(vm); i++)
        {
            /* TODO: reallocate if required */
            vm->parent->cec->stack[parent_sp + i + 1] = vm->cec->stack[i];
            vm->parent->cec->stack_pointer ++;
        }

        /* then set the parent's cc to the method*/
        vm->parent->cec->cc = vm->parent->jumptable[fe->jmptable_index]->location;

        /* patch the call frames of the parent so that it will give invalid value
         * but will not affect the correct functionality. In leave.c the call
         * frame that will be placed in nap_ip(vm) will be (uint)-1 thus the main
         * loop if nap_vm_run of the parent will exit andwill return here.
         * Kind of hacky, but works.*/
        vm->parent->cec->call_frames[vm->parent->cec->cfsize ++] = fake_call_frame_exit;

        /* and run the method with the patched stack */
        nap_vm_run(vm->parent);

        /* restore the parent's stack_pointer */
        vm->parent->cec->stack_pointer = parent_sp;

        /* fetch over the return values, they might be used by the caller later*/
        nap_copy_return_values(vm->parent, vm);
    }

    return NAP_SUCCESS;
}
