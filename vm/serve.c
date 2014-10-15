#include "serve.h"
#include "nbci.h"
#include "opcodes.h"
#include "stack.h"
#include "metatbl.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>

int nap_serve(struct nap_vm *vm)
{
    uint8_t serve_target = vm->content[nap_step_ip(vm)]; /* only variable */
    if(serve_target != OPCODE_VAR)
    {
        return NAP_FAILURE;
    }

    nap_index_t target_var_index = nap_fetch_index(vm);
    struct variable_entry* target_ve = nap_fetch_variable(vm, target_var_index);
    ASSERT_NOT_NULL_VAR(target_ve);
    /* there supposed to be no instantiation at this point for the target var */
    if(target_ve->instantiation)
    {
        if(target_ve->instantiation->value)
        {
            NAP_MEM_FREE(target_ve->instantiation->value);
        }
    }

    uint8_t serve_source = vm->content[nap_step_ip(vm)]; /* only variable */
    if(serve_source != OPCODE_VAR)
    {
        return NAP_FAILURE;
    }

    nap_index_t source_var_index = nap_fetch_index(vm);
    struct variable_entry* source_ve = nap_fetch_variable(vm, source_var_index);
    ASSERT_NOT_NULL_VAR(source_ve);
    CHECK_VARIABLE_INSTANTIATON(source_ve);

    size_t size_to_copy = source_ve->data_size * source_ve->instantiation->len;
    char* tmp = NAP_MEM_ALLOC(size_to_copy, char);
    memcpy(tmp, source_ve->instantiation->value, size_to_copy);
    target_ve->instantiation->value = tmp;
    target_ve->data_size = source_ve->data_size;
    target_ve->instantiation->len = source_ve->instantiation->len;
    target_ve->dimension_count = source_ve->dimension_count;
    memcpy(target_ve->dimensions, source_ve->dimensions, sizeof(target_ve->dimensions));

    return NAP_SUCCESS;
}
