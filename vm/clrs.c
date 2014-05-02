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

        /* the stack entru has no value, this is error */
        if( vm->cec->stack[nap_sp(vm)]->value == NULL )
        {
            break;
        }

        /* we have found the marker we need to delete */
        if(   (vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_MARKER_NAME)
           && (*(nap_mark_t*)(vm->cec->stack[nap_sp(vm)]->value) == marker) )
        {
            break;
        }

        /* if this was a normal */
        if(   vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_INT
           || vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_BYTE
           || vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_STRING
           || vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_MARKER_NAME)
        {
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
                NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]->var_def->instantiation->value);
                vm->cec->stack[nap_sp(vm)]->var_def->instantiation->value = NULL;

                /* the stack entry of the instantiation */
                NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]->var_def->instantiation);
                vm->cec->stack[nap_sp(vm)]->var_def->instantiation = NULL;


                /* but do not free the vm->cec->stack[nap_sp(vm)]->var_def
                   since it is freed on the cleanup */
            }
            else
            {
                /* this frees the stack stuff allocated at push or marks (such as
                * the int value allocated when pushed a number) */
                NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]->value);
                vm->cec->stack[nap_sp(vm)]->value = NULL;
            }
        }

        /* this actualyl frees the stack_entry*/
        NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]);
        vm->cec->stack[nap_sp(vm)] = NULL;
        vm->cec->stack_pointer --;
    }

    if(nap_sp(vm) == -1)
    {
        return NAP_FAILURE;
    }

    /* and here delete the marker itself */
    if(vm->cec->stack[nap_sp(vm)] != NULL
      && vm->cec->stack[nap_sp(vm)]->type == STACK_ENTRY_MARKER_NAME)
    {

        NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]->value);
        vm->cec->stack[nap_sp(vm)]->value = NULL;
        NAP_MEM_FREE(vm->cec->stack[nap_sp(vm)]);
        vm->cec->stack[nap_sp(vm)] = NULL;
        vm->cec->stack_pointer --;
    }
    return NAP_SUCCESS;
}
