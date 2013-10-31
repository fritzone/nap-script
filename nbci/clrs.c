#include "clrs.h"
#include "nbci.h"
#include "stack.h"
#include "opcodes.h"

#include <stdlib.h>

void nap_clrs(struct nap_vm* vm)
{
    nap_mark_t marker = nap_fetch_mark(vm);

    while(vm->stack_pointer > -1
          && vm->stack[vm->stack_pointer]->type != STACK_ENTRY_MARKER_NAME
          && vm->stack[vm->stack_pointer]->value
          && *(nap_mark_t*)(vm->stack[vm->stack_pointer]->value) != marker)
    {
        if(vm->stack[vm->stack_pointer]->type == OPCODE_REG
                || vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
        {
            free(vm->stack[vm->stack_pointer]->value); /* this frees the stuff allocated at push */
            vm->stack[vm->stack_pointer]->value = NULL;
        }

        free(vm->stack[vm->stack_pointer]);
        vm->stack[vm->stack_pointer] = NULL;
        vm->stack_pointer --;
    }

    if(vm->stack_pointer == -1)
    {
        fprintf(stderr, "stack underflow error. exiting.\n");
        exit(1);
    }

    if(vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
    {
        free(vm->stack[vm->stack_pointer]->value);
        vm->stack[vm->stack_pointer]->value = NULL;
        free(vm->stack[vm->stack_pointer]);
        vm->stack[vm->stack_pointer] = NULL;
        vm->stack_pointer --;
    }

}
