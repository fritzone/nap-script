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

    NAP_NN_ASSERT(vm, vm->stack);

    for(;;)
    {
        /* the stack pointer is still pointing into the stack */
        if( !(vm->stack_pointer > -1))
        {
            break;
        }

        /* we have something where the stack pointer points */
        if( vm->stack[vm->stack_pointer] == NULL)
        {
            break;
        }

        /* the stack entru has no value, this is error */
        if( vm->stack[vm->stack_pointer]->value == NULL )
        {
            break;
        }

        /* we have found the marker we need to delete */
        if(   (vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
           && (*(nap_mark_t*)(vm->stack[vm->stack_pointer]->value) == marker) )
        {
            break;
        }

        /* if this was a normal */
        if(   vm->stack[vm->stack_pointer]->type == STACK_ENTRY_INT
           || vm->stack[vm->stack_pointer]->type == STACK_ENTRY_BYTE
           || vm->stack[vm->stack_pointer]->type == STACK_ENTRY_STRING
           || vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
        {
            /* was this a variable declaration? */
            if(vm->stack[vm->stack_pointer]->var_def)
            {
                /* free the variable instantiation since this variable just
                   went out of scope */

                /* TODO: in case there is an object on the stack call their destructor */

                /* the actual value, but also frees the stack stuff allocated at push
                   due to:
                      se->value = ve->instantiation->value;
                      se->var_def = ve;
                in push.c (when creating a new variable) !!! */
                NAP_MEM_FREE(vm->stack[vm->stack_pointer]->var_def->instantiation->value);
                vm->stack[vm->stack_pointer]->var_def->instantiation->value = NULL;

                /* the stack entry of the instantiation */
                NAP_MEM_FREE(vm->stack[vm->stack_pointer]->var_def->instantiation);
                vm->stack[vm->stack_pointer]->var_def->instantiation = NULL;


                /* but do not free the vm->stack[vm->stack_pointer]->var_def
                   since it is freed on the cleanup */
            }
            else
            {
                /* this frees the stack stuff allocated at push or marks (such as
                * the int value allocated when pushed a number) */
                NAP_MEM_FREE(vm->stack[vm->stack_pointer]->value);
                vm->stack[vm->stack_pointer]->value = NULL;
            }
        }

        /* this actualyl frees the stack_entry*/
        NAP_MEM_FREE(vm->stack[vm->stack_pointer]);
        vm->stack[vm->stack_pointer] = NULL;
        vm->stack_pointer --;
    }

    if(vm->stack_pointer == -1)
    {
        return NAP_FAILURE;
    }

    /* and here delete the marker itself */
    if(vm->stack[vm->stack_pointer] != NULL
      && vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
    {

        NAP_MEM_FREE(vm->stack[vm->stack_pointer]->value);
        vm->stack[vm->stack_pointer]->value = NULL;
        NAP_MEM_FREE(vm->stack[vm->stack_pointer]);
        vm->stack[vm->stack_pointer] = NULL;
        vm->stack_pointer --;
    }
    return NAP_SUCCESS;
}
