#include "return.h"
#include "nbci.h"
#include "opcodes.h"
#include "nbci_impl.h"
#include "nap_consts.h"

#include <stdlib.h>
#include <string.h>

int nap_return(struct nap_vm *vm)
{
    uint8_t return_what = vm->content[vm->cc ++];   /* return what? */
    if(return_what == OPCODE_REG) /* do we check a register? */
    {
        uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/
        uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)
        {
            vm->rvi = vm->regi[register_index];
        }
        else
        if(register_type == OPCODE_BYTE)
        {
            vm->rvb = vm->regb[register_index];
        }
        else
        if(register_type == OPCODE_STRING)
        {
            /* we are supposed to recreate the RVS, clear it before*/
            size_t len = 0;
            MEM_FREE(vm->rvs);
            vm->rvl = vm->regslens[register_index];
            len = vm->rvl* CC_MUL;
            vm->rvs = (char*)calloc(len, sizeof(char));
            memcpy(vm->rvs, vm->regs[register_index], len);
        }
        else
        {
            _NOT_IMPLEMENTED
        }
    }
    else
    {
        _NOT_IMPLEMENTED
    }
    return NAP_SUCCESS;
}
