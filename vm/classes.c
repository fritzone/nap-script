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

    /* number of functions */
    vm->classtable_size = htovm_32(*(uint32_t*)(cloc));

    if(vm->classtable_size == 0)
    {
        return NAP_SUCCESS;
    }

    vm->classtable = NAP_MEM_ALLOC(vm->funtable_size + 1, struct class_descriptor*);
    NAP_NN_ASSERT(vm, vm->classtable);

    cloc += 4;
    for(;;)
    {
        uint16_t class_name_len = 0;
        char* class_name = NULL;

        struct class_descriptor* entry = NULL;

        if( (cloc + 4) > (vm->content + len) || class_c == vm->funtable_size + 1)
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
        cloc += class_name_len;
        entry->name = class_name;

        /* read the parameters if any*/
        vm->funtable[class_c ++] = entry;

    }
}
