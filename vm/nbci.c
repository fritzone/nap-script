#include "opcodes.h"
#include "strtable.h"
#include "metatbl.h"
#include "jmptable.h"
#include "nbci.h"
#include "stack.h"

#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/******************************************************************************/
/*                             Debugging section                              */
/******************************************************************************/

void dump(struct nap_vm* vm, FILE *fp)
{
    uint64_t i;
    puts("");
    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_INT)
                {
                    fprintf(fp, "E:[%s=%" PRId64 "](%" PRIu64 "/%" PRIu64 ")\n",
                           vm->metatable[i]->name,
                           *(nap_int_t*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);
                }
                else
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_STRING)
                {
                    fprintf(fp, "E:[%s=%s](%" PRIu64 "/%" PRIu64 ")\n",
                           vm->metatable[i]->name,
                           (char*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);
                }
                else
                {
                    fprintf(fp, "X:[%s=%"PRId64"](%" PRIu64 "/%" PRIu64 ")\n",
                           vm->metatable[i]->name,
                           *(nap_int_t*)(vm->metatable[i]->instantiation->value)
                           ,i, vm->meta_size);

                }
            }
            else
            {
                fprintf(fp, "N:[%s=??](%" PRIu64 "/%" PRIu64 ")\n", vm->metatable[i]->name,
                       i, vm->meta_size);

            }
        }
        else
        {
            fprintf(fp, "?:[%s=??](%"PRIu64"/%"PRIu64")\n", vm->metatable[i]->name,
                   i, vm->meta_size);
        }
    }
}

nap_int_t nap_vm_get_int(struct nap_vm* vm, char* name, int* found)
{
    uint64_t i;
    char* finame = name;

    for(i=0; i<vm->meta_size; i++)
    {
        if(vm->metatable[i]->instantiation)
        {
            if(vm->metatable[i]->instantiation->value)
            {
                if(vm->metatable[i]->instantiation->type == STACK_ENTRY_INT)
                {
                    if(vm->metatable[i]->name && !strcmp(vm->metatable[i]->name, finame))
                    {
                        if(finame != name)
                        {
                            MEM_FREE(finame);
                        }
                        *found = 1;
                        return *(nap_int_t*)(vm->metatable[i]->instantiation->value);
                    }
                }
            }
        }
    }
    *found = 0;
    return NAP_NO_VALUE;
}

void nap_vm_run(struct nap_vm* vm)
{
    while(vm->cc < vm->meta_location)
    {
        vm->current_opcode = vm->content[vm->cc];
        vm->cc ++;

        if(vm->opcode_handlers[vm->current_opcode] != 0)
        {
            TRY_CALL(vm->opcode_handlers[vm->current_opcode],
                    vm->opcode_error_codes[vm->current_opcode]);
        }
        else
        if(vm->current_opcode == OPCODE_EXIT) /* quit the application ... */
        {
            if(vm->environment == STANDALONE) /* only if not run in a library */
            {
                /* free the allocated metatable */
                nap_vm_cleanup(vm);
                exit(0);
            }
            else
            {
                return;
            }
        }
        else
        if(vm->current_opcode == OPCODE_CLBF) /* clear last boolean flag */
        {
            vm->lbf = UNDECIDED;
        }
        else
        {
            fprintf(stderr, "invalid opcode [%x] at %"PRIu64" (%"PRIx64")\n",
                    vm->current_opcode, vm->cc - 1, vm->cc - 1);
            nap_vm_cleanup(vm);
            exit(5);
        }
    }
}
