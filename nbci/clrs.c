#include "clrs.h"
#include "nbci.h"
#include "stack.h"
#include "opcodes.h"

#include <stdlib.h>

void nap_clrs(struct nap_vm* vm)
{
    uint32_t* p_marker_code = (uint32_t*)(vm->content + vm->cc);
    vm->cc += sizeof(uint32_t);

    while(vm->stack_pointer > -1
          && vm->stack[vm->stack_pointer]->type != STACK_ENTRY_MARKER_NAME
          && vm->stack[vm->stack_pointer]->value
          && *(uint32_t*)(vm->stack[vm->stack_pointer]->value) != *p_marker_code)
    {
        if(vm->stack[vm->stack_pointer]->type == OPCODE_REG
                || vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
        {
            free(vm->stack[vm->stack_pointer]->value);
        }

        free(vm->stack[vm->stack_pointer]);
        vm->stack_pointer --;
    }

    if(vm->stack[vm->stack_pointer]->type == STACK_ENTRY_MARKER_NAME)
    {
        free(vm->stack[vm->stack_pointer]->value);
        free(vm->stack[vm->stack_pointer]);
        vm->stack_pointer --;
    }

}
