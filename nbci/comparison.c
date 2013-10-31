#include "comparison.h"
#include "opcodes.h"
#include "nbci.h"

#include <stdlib.h>

void nap_comparison(struct nap_vm* vm)
{
    uint8_t mov_target = vm->content[vm->cc ++];   /* what to check (reg only)*/

    if(mov_target == OPCODE_REG) /* do we move in a register? */
    {
        uint8_t register_type = vm->content[vm->cc ++]; /* int/string/float...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)
        {
            uint8_t register_index = vm->content[vm->cc ++]; /* 0, 1, 2 ...*/
            uint8_t move_source = vm->content[vm->cc ++]; /* what are we checking against*/

            if(move_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
            {
                nap_number_t immediate = nap_read_immediate(vm);
                nap_vm_set_lbf_to_op_result(vm, vm->regi[register_index], immediate, vm->current_opcode);
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
    }
    else
    {
        fprintf(stderr, "eq works only on registers\n");
        exit(8);
    }

}
