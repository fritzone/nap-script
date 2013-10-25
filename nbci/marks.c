#include "marks.h"
#include "stack.h"
#include "nbci.h"

#include <stdlib.h>

void nap_marks(struct nap_vm *vm)
{
    uint32_t* p_marker_code = NULL;
    int32_t* temp = NULL;
    struct stack_entry* marker = (struct stack_entry*)(
                calloc(sizeof(struct stack_entry), 1));
    marker->type = STACK_ENTRY_MARKER_NAME;

    p_marker_code = (uint32_t*)(vm->content + vm->cc);
    vm->cc += sizeof(uint32_t);

    temp = (int32_t*)calloc(1, sizeof(int32_t));
    *temp = *p_marker_code;

    marker->value = temp;

    vm->stack[++ vm->stack_pointer] = marker;

}
