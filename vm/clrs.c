#include "clrs.h"
#include "nbci.h"
#include "stack.h"
#include "opcodes.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_clrs(struct nap_vm* vm)
{
    nap_mark_t marker = nap_fetch_mark(vm);

    while(vm->stack_pointer > -1
          && vm->stack != NULL
          && vm->stack[vm->stack_pointer] != NULL
          && vm->stack[vm->stack_pointer]->type != STACK_ENTRY_MARKER_NAME
          && vm->stack[vm->stack_pointer]->value != NULL
          && *(nap_mark_t*)(vm->stack[vm->stack_pointer]->value) != marker)
    {
        if(vm->stack[vm->stack_pointer]->type == (StackEntryType)OPCODE_REG
                || vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
        {
            NAP_MEM_FREE(vm->stack[vm->stack_pointer]->value); /* this frees the stuff allocated at push */
            vm->stack[vm->stack_pointer]->value = NULL;
            /* TODO: in case there are object on the stack call their destructor */
        }

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
