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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define __STDC_FORMAT_MACROS
#ifndef _WIN32
#include <inttypes.h>
#else
#define PRId64 "lld"
#define PRIu64 "lld"
#define PRIx64 "x"
#endif

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
                           *(int64_t*)(vm->metatable[i]->instantiation->value)
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
                           *(int64_t*)(vm->metatable[i]->instantiation->value)
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

void nap_vm_run(struct nap_vm *vm)
{
    /* and start interpreting the code */
    while(vm->cc < vm->meta_location)
    {
        vm->current_opcode = vm->content[vm->cc];
        vm->cc ++;

        /* is this a PUSH operation? */
        if(vm->current_opcode == OPCODE_PUSH)
        {
            nap_push(vm);
        }
        else
        /* is this checking for something? */
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
        /* is this a MOV operation? */
        if(vm->current_opcode == OPCODE_MOV)
        {
            nap_mov(vm) ;
        }
        else
        /* jumping somewhere ? */
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
        /* giving up our territory till a given name */
        if(vm->current_opcode == OPCODE_CLRS_NAME)
        {
            nap_clrs(vm);
        }
        else
        /* calling someone */
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
            /* free the allocated metatable */
            cleanup(vm);
            /* and finally leaving */
            exit(0);
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
        /* is this a mathematical operation? */
        if(vm->current_opcode == OPCODE_ADD
                || vm->current_opcode == OPCODE_MUL
                || vm->current_opcode == OPCODE_SUB
                || vm->current_opcode == OPCODE_DIV
                || vm->current_opcode == OPCODE_MOD )
        {
            nap_operation(vm);
        }
        else
        {
            fprintf(stderr, "invalid opcode [%x] at %"PRIu64" (%"PRIx64")\n",
                    vm->current_opcode, vm->cc - 1, vm->cc - 1);
            cleanup(vm);
            exit(5);

        }
    }

}


/*
 * Main entry point
 */
int main()
{
    struct nap_vm* vm = nap_vm_load("test.ncb");
    if(!vm)
    {
        exit(1);
    }

    nap_vm_run(vm);

    return 0;
}

