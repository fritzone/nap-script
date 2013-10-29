#include "marks.h"
#include "stack.h"
#include "nbci.h"

#include <stdlib.h>

void nap_marks(struct nap_vm *vm)
{
    nap_mark_t marker_code = nap_fetch_mark(vm);
    nap_mark_t* temp = temp = (nap_mark_t*)calloc(1, sizeof(int32_t));
    struct stack_entry* marker = (struct stack_entry*)(calloc(sizeof(struct stack_entry), 1));

    *temp = marker_code;

    marker->type = STACK_ENTRY_MARKER_NAME;
    marker->value = temp;

    vm->stack[++ vm->stack_pointer] = marker;

}
