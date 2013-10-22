#include "comparison.h"

void nap_comparison(struct nap_vm* vm)
{
    uint8_t mov_target = content[cc ++];   /* what to check (reg only)*/

    if(mov_target == OPCODE_REG) /* do we move in a register? */
    {
        uint8_t register_type = content[cc ++]; /* int/string/float...*/

        /* we are dealing with an INT type register */
        if(register_type == OPCODE_INT)
        {
            uint8_t register_index = content[cc ++]; /* 0, 1, 2 ...*/
            uint8_t move_source = content[cc ++]; /* what are we checking against*/

            if(move_source == OPCODE_IMMEDIATE) /* immediate value (1,..) */
            {
                uint8_t imm_size = content[cc ++];
                if(imm_size == OPCODE_BYTE)
                {
                    int8_t* immediate = (int8_t*)(content + cc);
                    set_lbf_to_op_result(regi[register_index],
                                            *immediate,
                                            current_opcode);
                    cc ++;
                }
                else
                if(imm_size == OPCODE_SHORT)
                {
                    int16_t* immediate = (int16_t*)(content + cc);
                    set_lbf_to_op_result(regi[register_index],
                                            *immediate,
                                            current_opcode);
                    cc += 2;
                }
                else
                if(imm_size == OPCODE_LONG)
                {
                    int32_t* immediate = (int32_t*)(content + cc);
                    set_lbf_to_op_result(regi[register_index],
                                            *immediate,
                                            current_opcode);
                    cc += 4;
                }
                else
                if(imm_size == OPCODE_HUGE)
                {
                    int64_t* immediate = (int64_t*)(content + cc);
                    set_lbf_to_op_result(regi[register_index],
                                            *immediate,
                                            current_opcode);
                    cc += 8;
                }
                else
                {
                    printf("invalid immediate size [cmp]: 0x%x", imm_size);
                    exit(12);
                }
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
