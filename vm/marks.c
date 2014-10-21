#include "marks.h"
#include "stack.h"
#include "nbci.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_marks(struct nap_vm *vm)
{
    nap_mark_t marker_code = nap_fetch_mark(vm);

    /* TODO: Reallocate if required */
    vm->cec->stack[++ vm->cec->stack_pointer] = vm->marks_list[marker_code];
    *(int64_t*)vm->marks_list[marker_code]->value = vm->cec->stack_pointer;

    return NAP_SUCCESS;
}
