#include "marks.h"
#include "stack.h"
#include "nbci.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_marks(struct nap_vm *vm)
{
    nap_mark_t marker_code = nap_fetch_mark(vm);
    nap_mark_t* temp = temp = (nap_mark_t*)calloc(1, sizeof(int32_t));
	struct stack_entry* marker = NULL;
	if(!temp)
	{
        return NAP_FAILURE;
	}
    marker = (struct stack_entry*)(calloc(sizeof(struct stack_entry), 1));

    *temp = marker_code;

    marker->type = STACK_ENTRY_MARKER_NAME;
    marker->value = temp;

    vm->stack[++ vm->stack_pointer] = marker;
    return NAP_SUCCESS;
}
