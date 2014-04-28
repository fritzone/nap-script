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

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)
        {
            vm->rvi = vm->regi[register_index];
        }
        else
        if(register_type == OPCODE_BYTE)
        {
            vm->rvb = nap_regb(vm, register_index);
        }
        else
        if(register_type == OPCODE_STRING)
        {
            /* we are supposed to recreate the RVS, clear it before*/
            NAP_MEM_FREE(vm->rvs);
            vm->rvl = vm->regslens[register_index];
            NAP_STRING_ALLOC(vm, vm->rvs, vm->rvl);
            NAP_STRING_COPY(vm->rvs, vm->regs[register_index], vm->rvl);

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
