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

        if(vm->current_opcode == OPCODE_PUSH) /* push something onto the stack */
        {
            TRY_CALL(nap_push, ERR_VM_0015)
        }
        else
        if(vm->current_opcode == OPCODE_EQ                /* compare two things*/
                || vm->current_opcode == OPCODE_LT
                || vm->current_opcode == OPCODE_GT
                || vm->current_opcode == OPCODE_NEQ
                || vm->current_opcode == OPCODE_LTE
                || vm->current_opcode == OPCODE_GTE)
        {
            TRY_CALL(nap_comparison,ERR_VM_0005)
        }
        else
        if(vm->current_opcode == OPCODE_MOV) /* move something into something else */
        {
            TRY_CALL(nap_mov, ERR_VM_0011)
        }
        else
        if(vm->current_opcode == OPCODE_JLBF || vm->current_opcode == OPCODE_JMP)
        {
            nap_jump(vm); /* jump somewhere (alsoconditional jump) */
        }
        else
        if(vm->current_opcode == OPCODE_MARKS_NAME) /* place a marker */
        {
            TRY_CALL(nap_marks, ERR_VM_0016);
        }
        else
        if(vm->current_opcode == OPCODE_CLRS_NAME) /* clear the stack till the marker*/
        {
            TRY_CALL(nap_clrs, ERR_VM_0002)
        }
        else
        if(vm->current_opcode == OPCODE_CALL) /* call a function */
        {
            nap_call(vm);
        }
        else
        if(vm->current_opcode == OPCODE_PEEK) /* peek the stack */
        {
            TRY_CALL(nap_peek, ERR_VM_0013)
        }
        else
        if(vm->current_opcode == OPCODE_POP) /* pop from the stack */
        {
            TRY_CALL(nap_pop, ERR_VM_0014)
        }
        else
        if(vm->current_opcode == OPCODE_RETURN) /* return a value */
        {
            nap_return(vm);
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
        if(vm->current_opcode == OPCODE_INC) /* increment */
        {
            TRY_CALL(nap_inc, ERR_VM_0010)
        }
        else
        if(vm->current_opcode == OPCODE_DEC) /* decrement */
        {
            TRY_CALL(nap_dec, ERR_VM_0009)
        }
        else
        if(vm->current_opcode == OPCODE_CLIDX) /* clear indices */
        {
            nap_clidx(vm);
        }
        else
        if(vm->current_opcode == OPCODE_LEAVE) /* leave the current call frame*/
        {
            vm->cc = vm->call_frames[-- vm->cfsize];
        }
        else
        if(vm->current_opcode == OPCODE_POPALL) /* pop all from internal stack*/
        {
            TRY_CALL(nap_restore_registers, ERR_VM_0007)
        }
        else
        if(vm->current_opcode == OPCODE_PUSHALL) /* psuh all to internal stack */
        {
            TRY_CALL(nap_save_registers, ERR_VM_0006)
        }
        else
        if(vm->current_opcode == OPCODE_ADD
                || vm->current_opcode == OPCODE_MUL
                || vm->current_opcode == OPCODE_SUB
                || vm->current_opcode == OPCODE_DIV
                || vm->current_opcode == OPCODE_MOD ) /* arithmetic operation */
        {
            TRY_CALL(nap_operation, ERR_VM_0012)
        }
        else
        if(vm->current_opcode == OPCODE_CLBF) /* clear last boolean flag */
        {
            vm->lbf = UNDECIDED;
        }
        else
        if(vm->current_opcode == OPCODE_INTR) /* call an interrupt */
        {
            TRY_CALL(nap_handle_interrupt, ERR_VM_0017);
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
