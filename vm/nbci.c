#include "opcodes.h"
#include "strtable.h"
#include "metatbl.h"
#include "jmptable.h"
#include "nbci.h"
#include "stack.h"

#include "push.h"
#include "comparison.h"
#include "mov.h"
#include "jump.h"
#include "marks.h"
#include "clrs.h"
#include "call.h"
#include "peek.h"
#include "pop.h"
#include "return.h"
#include "inc.h"
#include "dec.h"
#include "clidx.h"
#include "operation.h"

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
    return 0x0BADF00D;
}



void nap_vm_run(struct nap_vm* vm)
{
    while(vm->cc < vm->meta_location)
    {
        vm->current_opcode = vm->content[vm->cc];
        vm->cc ++;

        if(vm->current_opcode == OPCODE_PUSH)
        {
            nap_push(vm);
        }
        else
        if(vm->current_opcode == OPCODE_EQ
                || vm->current_opcode == OPCODE_LT
                || vm->current_opcode == OPCODE_GT
                || vm->current_opcode == OPCODE_NEQ
                || vm->current_opcode == OPCODE_LTE
                || vm->current_opcode == OPCODE_GTE)
        {
            nap_comparison(vm);
        }
        else
        if(vm->current_opcode == OPCODE_MOV)
        {
            nap_mov(vm) ;
        }
        else
        if(vm->current_opcode == OPCODE_JLBF || vm->current_opcode == OPCODE_JMP)
        {
            nap_jump(vm);
        }
        else
        if(vm->current_opcode == OPCODE_MARKS_NAME)
        {
            nap_marks(vm);
        }
        else
        if(vm->current_opcode == OPCODE_CLRS_NAME)
        {
            nap_clrs(vm);
        }
        else
        if(vm->current_opcode == OPCODE_CALL)
        {
            nap_call(vm);
        }
        else
        if(vm->current_opcode == OPCODE_PEEK)
        {
            nap_peek(vm);
        }
        else
        if(vm->current_opcode == OPCODE_POP)
        {
            nap_pop(vm);
        }
        else
        if(vm->current_opcode == OPCODE_RETURN)
        {
            nap_return(vm);
        }
        else
        if(vm->current_opcode == OPCODE_EXIT)
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
        if(vm->current_opcode == OPCODE_INC) /* increment */
        {
            nap_inc(vm);
        }
        else
        if(vm->current_opcode == OPCODE_DEC) /* decrement */
        {
            nap_dec(vm);
        }
        else
        if(vm->current_opcode == OPCODE_CLIDX)
        {
            nap_clidx(vm);
        }
        else
        if(vm->current_opcode == OPCODE_LEAVE)
        {
            vm->cc = vm->call_frames[-- vm->cfsize];
        }
        else
        if(vm->current_opcode == OPCODE_POPALL)
        {
            nap_restore_registers(vm);
        }
        else
        if(vm->current_opcode == OPCODE_PUSHALL)
        {
            nap_save_registers(vm);
        }
        else
        if(vm->current_opcode == OPCODE_ADD
                || vm->current_opcode == OPCODE_MUL
                || vm->current_opcode == OPCODE_SUB
                || vm->current_opcode == OPCODE_DIV
                || vm->current_opcode == OPCODE_MOD )
        {
            nap_operation(vm);
        }
        else
        if(vm->current_opcode == OPCODE_CLBF)
        {
            vm->lbf = UNDECIDED;
        }
        else
        if(vm->current_opcode == OPCODE_INTR) /* call an interrupt */
        {
            nap_handle_interrupt(vm);
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
