#include "return.h"
#include "nbci.h"
#include "opcodes.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>
#include <string.h>

int nap_return(struct nap_vm *vm)
{
    uint8_t return_what = vm->content[nap_step_ip(vm)];   /* return what? */
    if(return_what == OPCODE_REG) /* do we check a register? */
    {
        uint8_t register_type = vm->content[nap_step_ip(vm)]; /* int/string/float...*/
        uint8_t register_index = vm->content[nap_step_ip(vm)]; /* 0, 1, 2 ...*/

        if(register_type == OPCODE_INT) /* we are dealing with an INT register */
        {
            vm->cec->rvi = nap_regi(vm, register_index);
        }
        else
        if(register_type == OPCODE_REAL) /* we are dealing with an REAL register */
        {
            vm->cec->rvr = nap_regr(vm, register_index);
        }
        else
        if(register_type == OPCODE_BYTE)
        {
            vm->cec->rvb = nap_regb(vm, register_index);
        }
        else
        if(register_type == OPCODE_STRING)
        {
            /* we are supposed to recreate the RVS, clear it before*/
            /* TODO this is dangerous, might lose the old rvs. rewrite */
            NAP_MEM_FREE(vm->cec->rvs);
            vm->cec->rvl = nap_regs(vm, register_index)->l;
            NAP_STRING_ALLOC(vm, vm->cec->rvs, vm->cec->rvl);
            NAP_STRING_COPY(vm->cec->rvs, nap_regs(vm, register_index)->s, vm->cec->rvl);

        }
        else
        {
            NAP_NOT_IMPLEMENTED
        }
    }
    else
    {
        NAP_NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}
