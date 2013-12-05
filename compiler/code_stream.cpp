#include "code_stream.h"
#include "utils.h"
#include "type.h"
#include "opcodes.h"
#include "compiler.h"

extern "C"
{
#include "byte_order.h"
}

#include <string>
#include <vector>
#include <stdlib.h>
#include <limits.h>

static const char BITSIZE = 0x32;
const char* NEWLINE = "\n";
static uint8_t max_reg_count = 0;

// the name of the bytecode file
static const std::string fname = "test.ncb";

class file_abstraction
{
public:

    file_abstraction(nap_compiler* _compiler) : mcompiler(_compiler)
    {
    }


    void write_stuff_to_file_64(uint64_t stuff, int pos = -1)
    {
        uint64_t network_order_stuff = htovm_64(stuff);
        mcompiler->place_bytes(pos, &network_order_stuff, 8);
    }

    void write_stuff_to_file_32(uint32_t stuff, int pos = -1)
    {
        uint32_t network_order = htovm_32(stuff);
        mcompiler->place_bytes(pos, &network_order, 4);
    }

    void write_stuff_to_file_16(uint16_t stuff, int pos = -1)
    {
        uint16_t network_order = htovm_16(stuff);
        mcompiler->place_bytes(pos, &network_order, 2);
    }

    void write_stuff_to_file_8(uint8_t stuff, int pos = -1)
    {
        uint8_t netw = stuff;
        mcompiler->place_bytes(pos, &netw, 1);
    }

    void write_string_to_file(const char* s, int cnt, int pos = -1)
    {
        mcompiler->place_bytes(pos, s, cnt);
    }

    uint32_t ftell()
    {
        return mcompiler->last_bc_pos();
    }

private:

    nap_compiler* mcompiler;
};

/**
 * Helper class: its destructor is executed on the application exit and this
 * finalizes the bytecode file
 **/
void code_finalizer::finalize()
{
    uint32_t meta_location ;
    uint32_t strtable_location ;
    uint32_t jumptable_location ;

    {
    file_abstraction f(mcompiler);

    meta_location = f.ftell();

    // write out the variables
    static const char METATABLE[] = ".meta";
    f.write_string_to_file(METATABLE, 5);
    uint32_t meta_count = mcompiler->variables().size();
    f.write_stuff_to_file_32(meta_count);
    for(unsigned int i=0; i<meta_count; i++)
    {
        f.write_stuff_to_file_32(mcompiler->variables()[i]->meta_location);
        f.write_stuff_to_file_8(mcompiler->variables()[i]->type);

        uint16_t var_name_length = mcompiler->variables()[i]->name.length();
        // find out if this is a global variable or not: if there is only one dot
        // in the name, it is a global variable
        // TODO: include a debugging flag option to write out all the variable names
        size_t n = std::count(mcompiler->variables()[i]->name.begin(),
                              mcompiler->variables()[i]->name.end(), '.');
        if(n == 1) // global variable. Skip the "global."
        {
            int globlen = strlen("global") + 1;
            f.write_stuff_to_file_16(var_name_length - globlen);
            const char* vname = mcompiler->variables()[i]->name.c_str();
            vname += globlen;
            f.write_string_to_file(vname, var_name_length - globlen);
        }
        else
        {
            f.write_stuff_to_file_16(0); // write 0 to indicate name is not important
        }
    }

    strtable_location = f.ftell();
    // write out the stringtable
    static const char STRINGTABLE[] = ".str";
    f.write_string_to_file(STRINGTABLE, 4);
    uint32_t strtable_count = mcompiler->stringtable().size();
    f.write_stuff_to_file_32(strtable_count);
    for(unsigned int i=0; i<strtable_count; i++)
    {
        f.write_stuff_to_file_32( mcompiler->stringtable()[i]->index);
        f.write_stuff_to_file_32( mcompiler->stringtable()[i]->length);
        const char* str =  mcompiler->stringtable()[i]->the_string.c_str();
        f.write_string_to_file(str,  mcompiler->stringtable()[i]->length);
    }
    jumptable_location = f.ftell();
    // write out the jumptable
    static const char JUMPTABLE[] = ".jmp";
    f.write_string_to_file(JUMPTABLE, 4);
    uint32_t jumptable_count = mcompiler->jumptable().size();
    f.write_stuff_to_file_32(jumptable_count);
    for(unsigned int i=0; i<jumptable_count; i++)
    {
        label_entry* je = mcompiler->jumptable()[i];
        uint16_t l = je->name.length();

        f.write_stuff_to_file_32(je->bytecode_location); // the actual location in code
        f.write_stuff_to_file_8(je->type);      // 0, 1, 2 .. .see there
        if(je->type == 1 || je->type == 2)
        {
            f.write_stuff_to_file_16(l);            // The length of the name
            f.write_string_to_file(je->name.c_str(), l);
        }
    }

    }

    {

    file_abstraction f(mcompiler);

    uint8_t type = BITSIZE;
    f.write_stuff_to_file_8(type, 0);  // the type of the file: 32 bit
    f.write_stuff_to_file_32(meta_location, 0 + 1); // the meta location for variables
    f.write_stuff_to_file_32(strtable_location, 0 + 1 + 4); // the stringtable location
    f.write_stuff_to_file_32(jumptable_location, 0 + 1 + 4 + 4); // the jumptable location
    f.write_stuff_to_file_8(max_reg_count, 0 + 1 + 4 + 4 + 4);
    f.write_stuff_to_file_8(0, 0 + 1 + 4 + 4 + 4); /* a dummy flag for now */
    }
}

void code_stream::output_bytecode(const char* s)
{
    std::string expr = s;
    if(expr == " " || expr == "(" || expr == ")" || expr == "\n" || expr == ",")
    {
        fprintf(stderr, "%s ", s);
        return;
    }

    fprintf(stderr, "%s ", s);


    file_abstraction f(mcompiler);
    
    if(mcompiler->mfirst_entry)
    {
        mcompiler->mfirst_entry = false;
        NUMBER_INTEGER_TYPE temp = 0;
        uint8_t type = BITSIZE;
        uint8_t bits = 255;

        f.write_stuff_to_file_8(type);  // the type of the file: 32 bit
        f.write_stuff_to_file_32(temp); // the meta location for variables
        f.write_stuff_to_file_32(temp); // the stringtable location
        f.write_stuff_to_file_32(temp); // the jumptable location
        f.write_stuff_to_file_8(bits);  // extra for the max reg count
        f.write_stuff_to_file_8(0);     // extra for the flags
    }

    unsigned char opcode = 0;
        
    if(expr == "push") opcode = OPCODE_PUSH;
    if(expr == "ref")  opcode = OPCODE_REF;
    if(expr == "int")  opcode = OPCODE_INT;
    if(expr == "bool")  opcode = OPCODE_INT; /*bool treated as int in the bytecode*/
    if(expr == "char")  opcode = OPCODE_CHAR;
    if(expr == "real") opcode = OPCODE_FLOAT;
    if(expr == "string")  opcode = OPCODE_STRING;
    if(expr == "idx")  opcode = OPCODE_IDX;
    if(expr == "intr")  opcode = OPCODE_INTR;
    if(expr == "call") opcode = OPCODE_CALL;
    if(expr == "mov") opcode = OPCODE_MOV;
    if(expr == "inc") opcode = OPCODE_INC;
    if(expr == "dec") opcode = OPCODE_DEC;
    if(expr == "reg") opcode = OPCODE_REG;
    if(expr == "exit") opcode = OPCODE_EXIT;
    if(expr == "add") opcode = OPCODE_ADD;
    if(expr == "sub") opcode = OPCODE_SUB;
    if(expr == "mul") opcode = OPCODE_MUL;
    if(expr == "div") opcode = OPCODE_DIV;
    if(expr == "mod") opcode = OPCODE_MOD;
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
    if(expr == "clidx") opcode = OPCODE_CLIDX;
    if(expr == "leave") opcode = OPCODE_LEAVE;
    if(expr == "pushall") opcode = OPCODE_PUSHALL;
    if(expr == "popall") opcode = OPCODE_POPALL;
    if(expr == "clbf") opcode = OPCODE_CLBF;

    if(isnumber((expr.c_str())))
    {
        // the method calls just output a byte counter, no number identifier
        if(mcompiler->getLastOpcode() == OPCODE_CCIDX || mcompiler->getLastOpcode() == OPCODE_GROW)
        {
            int8_t nrf = atoi(expr.c_str());
            f.write_stuff_to_file_8(nrf);
            return;
        }
        else
        {
            opcode = OPCODE_IMMEDIATE;
        }
    }
    
    if(opcode)
    {
        if(opcode == OPCODE_IMMEDIATE && mcompiler->opcode_counter > 2 &&
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

        if(opcode == OPCODE_IMMEDIATE)
        {
            if(mcompiler->opcode_counter > 3 && mcompiler->opcodes()[mcompiler->opcode_counter - 3] == OPCODE_REG)
            { // if this counts a register
                uint8_t nrf = atoi(expr.c_str());
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
                uint8_t nrf = atoi(expr.c_str());
                f.write_stuff_to_file_8(nrf);
            }
            else
            {
                NUMBER_INTEGER_TYPE nr = atoi(expr.c_str());
                
                // the size of the number
                uint8_t type = OPCODE_BYTE;
                if(nr > 255) type = OPCODE_SHORT;
                if(nr > 65535) type = OPCODE_LONG;
                if(nr > 4294967294) type = OPCODE_HUGE;
                
                // if yes, switch on the first bit in the type
                if(expr[0] == '-') type |= 0x80;
                f.write_stuff_to_file_8(type);
                
                // and now write the number according to the size
                if(type == OPCODE_BYTE)
                {
                    int8_t nrf = atoi(expr.c_str());
                    f.write_stuff_to_file_8((uint8_t)nrf);
                }
                if(type == OPCODE_SHORT)
                {
                    int16_t nrf = atoi(expr.c_str());
                    f.write_stuff_to_file_16((uint16_t)nrf);
                }
                if(type == OPCODE_LONG)
                {
                    int32_t nrf = atoi(expr.c_str());
                    f.write_stuff_to_file_32((uint32_t)nrf);
                }
                if(type == OPCODE_HUGE)
                {
                    int64_t nrf = atoi(expr.c_str());
                    f.write_stuff_to_file_64((uint64_t)nrf);
                }
            }

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
                if(!strcmp(mcompiler->jumptable()[i]->name.c_str(), lblName.c_str()))
                {
                    idx = i;
                }
            }
            if(idx > -1) // found a label
            {
                mcompiler->jumptable()[idx]->bytecode_location = f.ftell();
            }
            else /* possibly creating a label before using it */
            {
                label_entry* le = new label_entry;
                le->bytecode_location = f.ftell();
                le->name = lblName;
                mcompiler->add_jumptable_entry(le);
            }
        }
        else
        if(s[ 0 ] == '@') // function call
        {
            if(mcompiler->getLastOpcode() == OPCODE_MOV)
            { // possibly a @#ccidx, @#grow
                if(s[1] == '#') // builtin function
                {
                    if(expr == "@#ccidx")
                    {
                        f.write_stuff_to_file_8(OPCODE_CCIDX);
                        mcompiler->setLastOpcode(OPCODE_CCIDX);
                    }
                    if(expr == "@#grow")
                    {
                        f.write_stuff_to_file_8(OPCODE_GROW);
                        mcompiler->setLastOpcode(OPCODE_GROW);
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
            bc_string_table_entry* entry = new bc_string_table_entry;
            entry->index =  mcompiler->stringtable().size();
            entry->the_string = expr.substr(1, expr.length() - 2);
            entry->length = expr.length() - 2;
            mcompiler->add_stringtable_entry(entry);
            f.write_stuff_to_file_8(OPCODE_STRING);
            f.write_stuff_to_file_32(entry->index);
            garbage_bin<bc_string_table_entry*>::instance().place(entry, mcompiler);
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
                        if(mcompiler->variables()[i]->name == expr)
                        {
                            idx = i;
                        }
                    }

                    if(idx > -1)
                    {
                        f.write_stuff_to_file_8(OPCODE_VAR);
                        f.write_stuff_to_file_32(mcompiler->variables()[idx]->meta_location);
                    }
                    else
                    {
                        bc_variable_entry* new_var = new bc_variable_entry;
                        new_var->meta_location = mcompiler->var_counter();
                        new_var->name = expr;
                        mcompiler->add_variable(new_var);
                        f.write_stuff_to_file_8(OPCODE_VAR);
                        f.write_stuff_to_file_32(mcompiler->var_counter());
                        mcompiler->inc_var_counter();
                        garbage_bin<bc_variable_entry*>::instance().place(new_var, mcompiler);
                    }
                }
            }
            else
            { // this is a label but also a function call (which is treated as a label)
                int idx = -1;
                for(unsigned int i=0; i<mcompiler->jumptable().size(); i++)
                {
                    if(mcompiler->jumptable()[i]->name == expr)
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
                    label_entry* le = new label_entry;
                    garbage_bin<label_entry*>::instance().place(le, mcompiler);

                    le->bytecode_location = 0;
                    le->name = expr;
                    mcompiler->add_jumptable_entry(le);
                    if(mcompiler->getLastOpcode() == OPCODE_CALL)
                    {
                        le->type = 1;
                    }
                    NUMBER_INTEGER_TYPE index = mcompiler->jumptable().size() - 1; // the real idx
                    f.write_stuff_to_file_32(index);
                }                
            }
        }
    }    
}

