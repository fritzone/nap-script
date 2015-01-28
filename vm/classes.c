#include "classes.h"
#include "nap_consts.h"
#include "nbci.h"
#include "nbci_impl.h"
#include "byte_order.h"

#include <stdlib.h>
#include <string.h>

int interpret_classtable(struct nap_vm *vm, uint8_t *start_location, uint32_t len)
{
    uint8_t* cloc = 0;
    size_t class_c = 0; /* counts the inserted classes */

    /* check is this .class?*/
    /* TODO: make sure we actually have 5 more characters */
    if(*(start_location) != '.'
        || *(start_location + 1) != 'c'
        || *(start_location + 2) != 'l'
        || *(start_location + 3) != 'a'
        || *(start_location + 4) != 's'
        || *(start_location + 5) != 's')
    {
        return NAP_FAILURE;
    }

    cloc = start_location + 6; /* skip the .class */

    /* number of classes */
    vm->classtable_size = htovm_32(*(uint32_t*)(cloc));

    if(vm->classtable_size == 0)
    {
        return NAP_SUCCESS;
    }

    vm->classtable = NAP_MEM_ALLOC(vm->classtable_size + 1, struct class_descriptor*);
    NAP_NN_ASSERT(vm, vm->classtable);

    cloc += 4;
    for(;;)
    {
        uint16_t class_name_len = 0;
        uint32_t class_variable_count = 0;
        uint32_t class_var_ctr = 0;

        char* class_name = NULL;

        struct class_descriptor* entry = NULL;

        if( (cloc + 4) > (vm->content + len) || class_c == vm->classtable_size + 1)
        {
            return NAP_SUCCESS;
        }

        entry = NAP_MEM_ALLOC(1, struct class_descriptor);
        NAP_NN_ASSERT(vm, entry);

        /* the length of the name of the class */
        class_name_len =  htovm_16(*(uint16_t*)(cloc));
        cloc += 2;

        /* read the name */
        class_name = NAP_MEM_ALLOC(class_name_len + 1, char);
        NAP_NN_ASSERT(vm, class_name);

        memcpy(class_name, cloc, class_name_len);
        entry->name = class_name;
        cloc += class_name_len;

        /* read in the number of variables */
        class_variable_count = htovm_32(*(uint32_t*)(cloc));
        cloc += 4;

        /* read in the indexes */
        entry->mtbl_indexes = NAP_MEM_ALLOC(class_variable_count, uint32_t);
        for(;class_var_ctr < class_variable_count; class_var_ctr++)
        {
            entry->mtbl_indexes[class_var_ctr] = htovm_32(*(uint32_t*)(cloc));
            cloc += 4;
        }

        entry->varcount = class_variable_count;

        /* insert in the VM structure */
        vm->classtable[class_c ++] = entry;

    }
}


struct class_instantiation *instantiate(struct nap_vm *vm, struct class_descriptor *cd)
{
    size_t i = 0;
    struct class_instantiation *inst = NAP_MEM_ALLOC(1, struct class_instantiation);
    inst->cd = cd;

    /* create the members for the insantiation */
    inst->members_instantiation = NAP_MEM_ALLOC(cd->varcount, struct variable_entry*);

    for(;i<cd->varcount; i++)
    {
        struct variable_entry* ve = NAP_MEM_ALLOC(1, struct variable_entry);
        NAP_NN_ASSERT(vm, ve);

        /* the actual instantiation of the variable */
        ve->instantiation = NAP_MEM_ALLOC(1, struct stack_entry);
        NAP_NN_ASSERT(vm, ve->instantiation);

        ve->instantiation->type = vm->metatable[cd->mtbl_indexes[i]]->datatype; /* must match the type */

        if(ve->instantiation->type == STACK_ENTRY_INT) /* pushing an integer */
        {
            ve->instantiation->value = NAP_MEM_ALLOC(1, nap_int_t);
            NAP_NN_ASSERT(vm, ve->instantiation->value);
            *(nap_int_t*)ve->instantiation->value = 0;
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_REAL) /* pushing a real */
        {
            ve->instantiation->value = NAP_MEM_ALLOC(1, nap_real_t);
            NAP_NN_ASSERT(vm, ve->instantiation->value);
            *(nap_real_t*)ve->instantiation->value = 0.0;
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_BYTE) /* pushing a byte */
        {
            ve->instantiation->value = NAP_MEM_ALLOC(1, nap_byte_t);
            NAP_NN_ASSERT(vm, ve->instantiation->value);
            *(nap_byte_t*)ve->instantiation->value = 0;
        }
        else
        if(ve->instantiation->type == STACK_ENTRY_STRING) /* pushing a string */
        {
            ve->instantiation->value = NAP_MEM_ALLOC(1, char);
            NAP_NN_ASSERT(vm, ve->instantiation->value);
            *(char*)ve->instantiation->value = 0;
        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }

        inst->members_instantiation[i] = ve;
    }

    return inst;
}
