#include "return.h"
#include "nbci.h"
#include "opcodes.h"

#include "nap_consts.h"

#include <stdlib.h>

int nap_return(struct nap_vm *vm)
{
    uint8_t return_what = vm->content[vm->cc ++];   /* return what? */
    if(return_what == OPCODE_REG) /* do we check a register? */
    {
        uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
            vm->rvi = vm->regi[register_index];
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
