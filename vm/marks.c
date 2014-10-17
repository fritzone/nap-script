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

    marker = NAP_MEM_ALLOC(1, struct stack_entry);
    NAP_NN_ASSERT(vm, marker); /* sort of ugly but will do what we want */

    marker->type = STACK_ENTRY_MARKER_NAME;
    marker->value = 0;
    marker->len = marker_code;

    /* TODO: Reallocate if required */
    vm->cec->stack[++ vm->cec->stack_pointer] = marker;
    return NAP_SUCCESS;
}
