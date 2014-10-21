#include "clrs.h"
#include "nbci.h"
#include "stack.h"
#include "opcodes.h"
#include "nbci_impl.h"
#include "nap_consts.h"
#include "metatbl.h"

#include <stdlib.h>

int nap_clrs(struct nap_vm* vm)
{
    nap_mark_t marker = nap_fetch_mark(vm);
    int64_t save_sp = nap_sp(vm);
    int64_t sp_ctr = save_sp;
    for(;;)
    {
        /* the stack pointer is still pointing into the stack */
        if( !(nap_sp(vm) > -1))
        {
            break;
        }

        /* we have something where the stack pointer points */
        if( vm->cec->stack[nap_sp(vm)] == NULL)
        {
            break;
        }

        /* we have found the marker we need to delete */
        if(   (vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_MARKER_NAME)
           && (vm->cec->stack[nap_sp(vm)]->len == marker) )
        {
            break;
        }

        /* was this a variable declaration? */
        if(vm->cec->stack[nap_sp(vm)]->var_def)
        {
            /* free the variable instantiation since this variable just
               went out of scope */

            /* TODO: in case there is an object on the stack call their destructor */

            /* the actual value, but also frees the stack stuff allocated at push
               due to:
                  se->value = ve->instantiation->value;
                  se->var_def = ve;
            in push.c (when creating a new variable) !!! */

            /* And now see if this value was stored or not. If stored it will stay
             * on the stack otherwise it will go away */
            if(!vm->cec->stack[nap_sp(vm)]->var_def->instantiation->stored)
            {
                NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]->var_def->instantiation->value);
                vm->cec->stack[nap_sp(vm)]->var_def->instantiation->value = NULL;

                /* the stack entry of the instantiation */
                NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]->var_def->instantiation);
                vm->cec->stack[nap_sp(vm)]->var_def->instantiation = NULL;

                /* and now restore the variable's instantiation */
                pop_variable_instantiation(vm->cec->stack[nap_sp(vm)]->var_def);

            }
            else
            {
                vm->cec->stack[nap_sp(vm)]->stored = 1;
            }

            /* but do not free the vm->cec->stack[nap_sp(vm)]->var_def
               since it is freed on the cleanup */
        }
        else
        {
            /* this frees the stack stuff allocated at push or marks (such as
-                * the int value allocated when pushed a number) but only if the
             * pushed value < STACK_ENTRY_MARKER_NAME (nomal stuff). Markers
             * should not be freed here */
            if(vm->cec->stack[nap_sp(vm)]->type < STACK_ENTRY_MARKER_NAME)
            {
                NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]->value);
                vm->cec->stack[nap_sp(vm)]->value = NULL; /* XXX Danger! */
            }

            if(vm->cec->stack_pointer == save_sp - 1)
            {
                /* It is impossible to get a stored element in this situation */
                save_sp --;
            }
        }

        /* this actually frees the stack_entry but only if it is not "store"'d */
        if(!vm->cec->stack[nap_sp(vm)]->stored)
        {
            /* but only if not marker */
            if(vm->cec->stack[nap_sp(vm)]->type != STACK_ENTRY_MARKER_NAME)
            {
                NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]);
                vm->cec->stack[nap_sp(vm)] = NULL; /* XXX Danger! */
            }

            if(vm->cec->stack_pointer < save_sp)
            {
                /* and let's see if we have stored values above this */
                relocate_stored_elements(vm);
            }
            vm->cec->stack_pointer --;
        }
        else
        {
            vm->cec->stack_pointer --;
        }
    }

    if(nap_sp(vm) == -1)
    {
        return NAP_FAILURE;
    }

    /* and here delete the marker itself */
    if(vm->cec->stack[nap_sp(vm)] != NULL
      && vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_MARKER_NAME)
    {
        vm->cec->stack[nap_sp(vm)] = NULL;
        if(vm->cec->stack_pointer < save_sp)
        {
            relocate_stored_elements(vm);
        }
        vm->cec->stack_pointer --;
    }

    return NAP_SUCCESS;
}
