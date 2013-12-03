#include "clrs.h"
#include "nbci.h"
#include "stack.h"
#include "opcodes.h"

#include <stdlib.h>

void nap_clrs(struct nap_vm* vm)
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
            MEM_FREE(vm->stack[vm->stack_pointer]->value); /* this frees the stuff allocated at push */
            vm->stack[vm->stack_pointer]->value = NULL;
            /* TODO: in case there are object on the stack call their desteructor */
        }

        MEM_FREE(vm->stack[vm->stack_pointer]);
        vm->stack[vm->stack_pointer] = NULL;
        vm->stack_pointer --;
    }

    if(vm->stack_pointer == -1)
    {
        fprintf(stderr, "stack underflow error. exiting.\n");
        nap_vm_cleanup(vm);
        exit(EXIT_FAILURE);
    }

    if(vm->stack[vm->stack_pointer] != NULL
      && vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
    {

        MEM_FREE(vm->stack[vm->stack_pointer]->value);
        vm->stack[vm->stack_pointer]->value = NULL;
        MEM_FREE(vm->stack[vm->stack_pointer]);
        vm->stack[vm->stack_pointer] = NULL;
        vm->stack_pointer --;
    }

}
