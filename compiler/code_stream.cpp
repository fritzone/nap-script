#include "code_stream.h"
#include "utils.h"
#include "type.h"
#include "code_finalizer.h"
#include "opcodes.h"
#include "compiler.h"
#include "charconverter.h"
#include "parameter.h"
#include "file_abstraction.h"

extern "C"
{
#include "byte_order.h"
}

#include <string>
#include <vector>
#include <stdlib.h>
#include <limits.h>


const char* NEWLINE = "\n";
static uint8_t max_reg_count = 0;
int code_stream::max_mark_index = 0;

// the name of the bytecode file
static const std::string fname = "test.ncb";

void code_stream::output_bytecode(const char* s)
{

    std::string expr = s;

    if(expr == "," || expr == " " || expr == "(" || expr == ")" ||expr == NEWLINE)
    {
        return;
    }

    code_finalizer::instance()->add_assembly_command(expr);

    if(mcompiler->mlast_cmd_for_bytecode.empty())
    {
        if(mcompiler->print_assembly && mcompiler->exp_w_location)
        {
//            fprintf(stderr, "\n--XX %s @ (%s:%d->%d)\n", mcompiler->exp_w_location->expression.c_str(),
//                    mcompiler->filename(mcompiler->exp_w_location->location.mfile_index).c_str(),
//                    mcompiler->exp_w_location->location.start_line_number,
//                    mcompiler->exp_w_location->location.end_line_number);
        }
        if(mcompiler->exp_w_location)
		{
            mcompiler->mlast_cmd_for_bytecode = mcompiler->exp_w_location->expression;
		}
    }
    else
    {

        if(mcompiler->exp_w_location && mcompiler->mlast_cmd_for_bytecode != mcompiler->exp_w_location->expression)
        {
            if(mcompiler->print_assembly)
            {
//                fprintf(stderr, "\n--XX %s @ (%s:%d->%d)\n", mcompiler->exp_w_location->expression.c_str(),
//                    mcompiler->filename(mcompiler->exp_w_location->location.mfile_index).c_str(),
//                    mcompiler->exp_w_location->location.start_line_number,
//                    mcompiler->exp_w_location->location.end_line_number);
            }
            mcompiler->mlast_cmd_for_bytecode = mcompiler->exp_w_location->expression;
		}
    }


    file_abstraction f(mcompiler);
    
    if(mcompiler->mfirst_entry)
    {
        mcompiler->mfirst_entry = false;
        NUMBER_INTEGER_TYPE temp = 0;
        uint8_t type = 0x32; // the type of the file. For now only 32 bits
        uint8_t bits = 255;

        f.write_stuff_to_file_8(type);  // the type of the file: 32 bit
        f.write_stuff_to_file_32(temp); // the meta location for variables
        f.write_stuff_to_file_32(temp); // the stringtable location
        f.write_stuff_to_file_32(temp); // the jumptable location
        f.write_stuff_to_file_32(temp); // the funtable location
        f.write_stuff_to_file_8(bits);  // extra for the max reg count
        f.write_stuff_to_file_8(0);     // extra for the flags
    }

    unsigned char opcode = 0;

    if(expr == "push") opcode = OPCODE_PUSH;
    if(expr == "ref")  opcode = OPCODE_REF;
    if(expr == "int")  opcode = OPCODE_INT;
    if(expr == "generic")  opcode = OPCODE_GENERIC;
    if(expr == "byte")  opcode = OPCODE_BYTE;
    if(expr == "bool")  opcode = OPCODE_INT; /*bool treated as int in the bytecode*/
    if(expr == "char")  opcode = OPCODE_STRING;
    if(expr == "real") opcode = OPCODE_REAL;
    if(expr == "string")  opcode = OPCODE_STRING;
    if(expr == "idx")  opcode = OPCODE_IDX;
    if(expr == "intr")  opcode = OPCODE_INTR;
    if(expr == "call") opcode = OPCODE_CALL;
    if(expr == "mov") opcode = OPCODE_MOV;
    if(expr == "inc") opcode = OPCODE_INC;
    if(expr == "dec") opcode = OPCODE_DEC;
    if(expr == "reg") opcode = OPCODE_REG;
    if(expr == "exit") opcode = OPCODE_EXIT;
    if(expr == "store") opcode = OPCODE_STORE;
    if(expr == "restore") opcode = OPCODE_RESTORE;
    if(expr == "add") opcode = OPCODE_ADD;
    if(expr == "sub") opcode = OPCODE_SUB;
    if(expr == "mul") opcode = OPCODE_MUL;
    if(expr == "div") opcode = OPCODE_DIV;
    if(expr == "mod") opcode = OPCODE_MOD;
    if(expr == "shl") opcode = OPCODE_SHL;
    if(expr == "shr") opcode = OPCODE_SHR;
    if(expr == "and") opcode = OPCODE_AND;
    if(expr == "or") opcode = OPCODE_OR;
    if(expr == "xor") opcode = OPCODE_XOR;
    if(expr == "not") opcode = OPCODE_NOT;
    if(expr == "bcom") opcode = OPCODE_BCOM;
    if(expr == "neg") opcode = OPCODE_NEG;
    if(expr == "pos") opcode = OPCODE_POS;
    if(expr == "eq") opcode = OPCODE_EQ;
    if(expr == "lt") opcode = OPCODE_LT;
    if(expr == "gt") opcode = OPCODE_GT;
    if(expr == "lte") opcode = OPCODE_LTE;
    if(expr == "gte") opcode = OPCODE_GTE;
    if(expr == "neq") opcode = OPCODE_NEQ;
    if(expr == "jlbf") opcode = OPCODE_JLBF;
    if(expr == "jmp") opcode = OPCODE_JMP;
    if(expr == "marksn") opcode = OPCODE_MARKS_NAME;
    if(expr == "clrsn") opcode = OPCODE_CLRS_NAME;
    if(expr == "return") opcode = OPCODE_RETURN;
    if(expr == "rv") opcode = OPCODE_RV;
    if(expr == "pop") opcode = OPCODE_POP;
    if(expr == "peek") opcode = OPCODE_PEEK;
    if(expr == "serve") opcode = OPCODE_SERVE;
    if(expr == "clidx") opcode = OPCODE_CLIDX;
    if(expr == "leave") opcode = OPCODE_LEAVE;
    if(expr == "pushall") opcode = OPCODE_PUSHALL;
    if(expr == "popall") opcode = OPCODE_POPALL;
    if(expr == "clbf") opcode = OPCODE_CLBF;
    if(expr == "poke") opcode = OPCODE_POKE;

    if(isnumber((expr.c_str())))
    {
        // the method calls just output a byte counter, no number identifier
        if(mcompiler->getLastOpcode() == OPCODE_CCIDX || mcompiler->getLastOpcode() == OPCODE_GROW)
        {
			// TODO: What is this?
            int8_t nrf = (int8_t)atoi(expr.c_str());
            f.write_stuff_to_file_8(nrf);
            return;
        }
        else
        {
            // TODO: check, if the number is a real, change the opcode
            if(number_get_type(expr) == BASIC_TYPE_INT)
            {
                opcode = OPCODE_IMMEDIATE_INT;
            }
            else
            {
                opcode = OPCODE_IMMEDIATE_REAL;
            }
        }
    }
    
    if(opcode)
    {
        if(opcode == OPCODE_IMMEDIATE_INT && mcompiler->opcode_counter > 2 &&
                (   mcompiler->opcodes()[mcompiler->opcode_counter - 2] == OPCODE_REG
                 || mcompiler->opcodes()[mcompiler->opcode_counter - 1] == OPCODE_INTR)
                )
        {
            // do not write it if it counts registers
        }
        else
        {
            f.write_stuff_to_file_8(opcode);
        }
        mcompiler->add_opcode(opcode);
        mcompiler->opcode_counter ++;
        mcompiler->setLastOpcode(opcode);

        if(opcode == OPCODE_IMMEDIATE_INT)
        {
            if(mcompiler->opcode_counter > 3 && mcompiler->opcodes()[mcompiler->opcode_counter - 3] == OPCODE_REG)
            { // if this counts a register
                uint8_t nrf = (uint8_t)atoi(expr.c_str());
                f.write_stuff_to_file_8(nrf);
                if(max_reg_count < nrf)
                {
                    max_reg_count = nrf;
                    if(max_reg_count == 255)
                    {
                        std::cerr <<"[compiler] a too complex operation was attempted. re-write your applcation :( " << std::endl;
                        abort();
                    }
                }
            }
            else
            // interrupt, does not neeed spcifier, there are max 255 of them
            if(mcompiler->opcode_counter > 3 && mcompiler->opcodes()[mcompiler->opcode_counter - 2] == OPCODE_INTR)
            {
                uint8_t nrf = (uint8_t)atoi(expr.c_str());
                f.write_stuff_to_file_8(nrf);
            }
            else
            {
#ifndef _MSC_VER
                uint64_t nr = (uint64_t)atoll(expr.c_str());
#else
				uint64_t nr = (uint64_t)_strtoi64(expr.c_str(), NULL, 10); //TODO: is this really base 10?
#endif

                // the size of the number
                uint8_t type = OPCODE_BYTE;
                if(nr > 255) type = OPCODE_SHORT;
                if(nr > 65535) type = OPCODE_LONG;
                if(nr > 4294967294u) type = OPCODE_HUGE;

                f.write_stuff_to_file_8(type);

                // and now write the number according to the size
                if(type == OPCODE_BYTE)
                {
                    int8_t nrf = (uint8_t)atoi(expr.c_str());
                    f.write_stuff_to_file_8(nrf);
                }
                if(type == OPCODE_SHORT)
                {
                    int16_t nrf = (int16_t)atoi(expr.c_str());
                    f.write_stuff_to_file_16((int16_t)nrf);
                }
                if(type == OPCODE_LONG)
                {
                    int32_t nrf = atoi(expr.c_str());
                    f.write_stuff_to_file_32((int32_t)nrf);
                }
                if(type == OPCODE_HUGE)
                {
#ifndef _MSC_VER
                uint64_t nrf = (uint64_t)atoll(expr.c_str());
#else
				uint64_t nrf = (uint64_t)_strtoi64(expr.c_str(), NULL, 10); //TODO: is this really base 10?
#endif
                    f.write_stuff_to_file_64((int64_t)nrf);
                }
            }

        }
        else
        if(opcode == OPCODE_IMMEDIATE_REAL)
        {
#ifndef _MSC_VER
            long double v = strtold(expr.c_str(), 0);
#else
			long double v = (long double) atof(expr.c_str()); /* Till the day MSVS will have strtold */
#endif
            f.write_stuff_to_file_64(pack754_64(v));
        }
    }
    else
    {
        if(s[0] == ':')   // label definition
        {
            std::string lblName = expr.substr(1, expr.length() - 2);
            int idx = -1;
            for(unsigned int i=0; i<mcompiler->jumptable().size(); i++)
            {
                if(!strcmp(mcompiler->jumptable()[i].name.c_str(), lblName.c_str()))
                {
                    idx = i;
                }
            }
            if(idx > -1) // found a label
            {
                mcompiler->jumptable()[idx].bytecode_location = f.ftell();
            }
            else /* possibly creating a label before using it */
            {
                label_entry le;
                le.bytecode_location = f.ftell();
                le.name = lblName;
                mcompiler->add_jumptable_entry(le);
            }
        }
        else
        if(s[0] == '@') // (internal) function call
        {
            uint8_t last = mcompiler->getLastOpcode() ;
            if(s[1] == '#') // builtin function
            {
                if(expr == "@#ccidx")
                {
                    f.write_stuff_to_file_8(OPCODE_CCIDX);
                    mcompiler->setLastOpcode(OPCODE_CCIDX);
                }
                else
                if(expr == "@#len")
                {
                    f.write_stuff_to_file_8(OPCODE_LEN);
                    mcompiler->setLastOpcode(OPCODE_LEN);
                }
                else
                if(expr == "@#grow")
                {
                    // grow must be preceeded by a "call" command.
                    if(last == OPCODE_CALL)
                    {
                        // fix the last opcode to be the OPCODE GROW, however the
                        // last opcode (the CALL) does not go in the bytecode stream
                        mcompiler->modify_last_opcode(OPCODE_CALL_INT);
                        f.write_stuff_to_file_8(OPCODE_GROW);
                        mcompiler->setLastOpcode(OPCODE_GROW);
                    }
                    else
                    {
                        mcompiler->throw_error("@#grow not preceeded by a \"call\" command");
                    }
                }

            }
        }
        else
        if(s[ 0 ] == '$') // method call
        {
        }
        else
        if(s[ 0 ] == '&')  // variable access of a class
        {
        }
        else
        if(s[0] == '\"') // string. Every mention of it creates a new stringtable entry
        {
            bc_string_table_entry entry (mcompiler->stringtable().size(),
                                         expr.length() - 2,
                                         expr.substr(1, expr.length() - 2) );
            mcompiler->add_stringtable_entry(entry);
            // write directly the opcode string, no immediate specifier
            f.write_stuff_to_file_8(OPCODE_STRING);
            f.write_stuff_to_file_32(entry.index);
        }
        else
        {
            if(mcompiler->getLastOpcode() == OPCODE_MARKS_NAME || mcompiler->getLastOpcode() == OPCODE_CLRS_NAME)
            {
                // this is a named mark
                int idx = -1;
                for(unsigned int i=0; i<mcompiler->namedmarks().size(); i++)
                {
                    if(mcompiler->namedmarks()[i].marker_name == expr)
                    {
                        idx = i;
                    }
                }
                if(idx > -1) // found a named mark
                {
                    NUMBER_INTEGER_TYPE index = idx;
                    f.write_stuff_to_file_32(index);
                }
                else // let's create a named mark
                {
                    bc_named_marks mark;
                    mark.marker_code = mcompiler->namedmarks().size();
                    mark.marker_name = expr;

                    mcompiler->add_mark(mark);
                    NUMBER_INTEGER_TYPE index = mcompiler->namedmarks().size() - 1; // the real idx
                    max_mark_index = index;
                    f.write_stuff_to_file_32(index);
                }

            }
            else
            if(mcompiler->getLastOpcode() != OPCODE_JLBF && mcompiler->getLastOpcode() != OPCODE_JMP && mcompiler->getLastOpcode() != OPCODE_CALL)
            { // this is a plain variable 
                if(expr == "true")
                {
                    output_bytecode("1");
                }
                else
                if(expr == "false")
                {
                    output_bytecode("0");
                }
                else
                {
                    int idx = -1;
                    for(unsigned int i=0; i<mcompiler->variables().size(); i++)
                    {
                        if(mcompiler->variables()[i].name == expr)
                        {
                            idx = i;
                        }
                    }

                    if(idx > -1)
                    {
                        f.write_stuff_to_file_8(OPCODE_VAR);
                        f.write_stuff_to_file_32(mcompiler->variables()[idx].meta_location);
                    }
                    else
                    {
                        bc_variable_entry new_var(mcompiler->var_counter(), expr);
                        mcompiler->add_variable(new_var);
                        f.write_stuff_to_file_8(OPCODE_VAR);
                        f.write_stuff_to_file_32(mcompiler->var_counter());
                        mcompiler->inc_var_counter();
                    }
                }
            }
            else
            { // this is a label but also a function call (which is treated as a label)
                int idx = -1;
                for(unsigned int i=0; i<mcompiler->jumptable().size(); i++)
                {
                    if(mcompiler->jumptable()[i].name == expr)
                    {
                        idx = i;
                    }
                }
                if(idx > -1) // found a label
                {
                    NUMBER_INTEGER_TYPE index = idx;
                    f.write_stuff_to_file_32(index);
                }
                else // let's create a label
                {
                    label_entry le;

                    le.bytecode_location = 0;
                    le.name = expr;

                    if(mcompiler->getLastOpcode() == OPCODE_CALL)
                    {
                        le.type = label_entry::LE_CALL;
                    }

                    mcompiler->add_jumptable_entry(le);
                    NUMBER_INTEGER_TYPE index = mcompiler->jumptable().size() - 1; // the real idx
                    f.write_stuff_to_file_32(index);
                }                
            }
        }
    }    
}

