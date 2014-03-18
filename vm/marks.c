#include "marks.h"
#include "stack.h"
#include "nbci.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_marks(struct nap_vm *vm)
{
    nap_mark_t marker_code = nap_fetch_mark(vm);
    struct stack_entry* marker = NULL;

    nap_mark_t* temp = NAP_MEM_ALLOC(1, nap_mark_t);
    NAP_NN_ASSERT(vm, temp);

    marker = NAP_MEM_ALLOC(1, struct stack_entry);
    if(!marker)
    {
        free(temp);
        NAP_NN_ASSERT(vm, marker); /* sort of ugly ... */
    }

    *temp = marker_code;

    marker->type = STACK_ENTRY_MARKER_NAME;
    marker->value = temp;

    vm->stack[++ vm->stack_pointer] = marker;
    return NAP_SUCCESS;
}
