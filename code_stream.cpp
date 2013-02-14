#include "code_stream.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <stdlib.h>

#include "is.h"
#include "utils.h"


static const unsigned char OPCODE_INT           = 0x01;
static const unsigned char OPCODE_FLOAT         = 0x02;
static const unsigned char OPCODE_STRING        = 0x03;
static const unsigned char OPCODE_SUB           = 0x8B;
static const unsigned char OPCODE_IMMEDIATE     = 0x11;
static const unsigned char OPCODE_INC           = 0x1C;
static const unsigned char OPCODE_MUL           = 0xA8;
static const unsigned char OPCODE_MOD           = 0xA9;
static const unsigned char OPCODE_ADD           = 0xAD;
static const unsigned char OPCODE_MOV           = 0xB8;
static const unsigned char OPCODE_JMP           = 0xBA;
static const unsigned char OPCODE_MARKS         = 0xBB;
static const unsigned char OPCODE_PUSH          = 0xBC;
static const unsigned char OPCODE_JLBF          = 0xBF;
static const unsigned char OPCODE_CALL          = 0xCA;
static const unsigned char OPCODE_DIV           = 0xD8;
static const unsigned char OPCODE_EQ            = 0xE0;
static const unsigned char OPCODE_EXIT          = 0xE1;
static const unsigned char OPCODE_REG           = 0xEE;
static const unsigned char OPCODE_REF           = 0xEF;
static const unsigned char OPCODE_VAR           = 0xFE;

/**
 * the description of a method from the binary opcode file
 */
struct bc_method_entry
{
    /* the length of the name of the method */
    uint32_t name_len;
    
    /* the name of the method*/
    std::string name;
    
    /* the byte offset location of the method in the binary file*/
    uint16_t location;
    
    /* if the location is 0 the builtin_id tells us the id of the internal 
     * function to be called */
    uint16_t builtin_id;
};

/**
 * the description of a variable from the binary file
 */
struct bc_variable_entry
{
    uint64_t stack_location;
    std::string name;    
};

unsigned char code_stream::last_opcode = 0;

// a list of variables that will be added to the "meta" section of the bytecode file
static std::vector<bc_variable_entry*> variables;

// this counts the variables
static uint64_t var_counter = 0;

// if the very first byte writte then also create the file
static bool first_entry = true;

// the name of the bytecode file
static const std::string fname = "test.ncb";

// used to build a variable name between various calls to the output_bytede mth.
static std::string built_upvar_name = "";

// the written opcodes are stored in here for later reference
static std::vector<unsigned char> opcodes;

// counts the opcodes 
static int opcode_counter = 0;

/**
 * Helper class: its destructor is executed on the application exit and this
 * finalizes the bytecode file
 **/
class code_finalizer
{
public:
    ~code_finalizer()
    {
        FILE* f = fopen(fname.c_str(), "ab+");
        // write out the variables
        static const char FINALIZER[] = ".meta";
        fwrite(FINALIZER, 5, 1, f);
        for(unsigned int i=0; i<variables.size(); i++)
        {
            fwrite(&variables[i]->stack_location, sizeof(uint64_t), 1, f);
            uint16_t var_name_length = variables[i]->name.length();
            fwrite(&var_name_length, sizeof(uint16_t), 1, f);
            const char* vname = variables[i]->name.c_str();
            fwrite(vname, sizeof(uint8_t), var_name_length, f);
        }
        
        fclose(f);
    }
};

// dummy object, we need its destructor
static code_finalizer code_f;

static uint64_t last_file_pos = 0;

template <class T> uint64_t write_stuff_to_file(FILE* fp, T stuff, int cnt)
{
    fwrite(&stuff, sizeof(T), cnt, fp);
    return last_file_pos = ftell(fp);
}

void code_stream::output_bytecode(const char* s)
{
    std::string expr = s;
    if(expr == " ") return;
    if(expr == "(") return;
    if(expr == ")") return;
    if(expr == "\n") return;
    if(expr == ",") return;
    
    printf("%s\n", s);
    FILE* f = NULL;
    
    if(first_entry)
    {
        f = fopen(fname.c_str(), "wb+");
        first_entry = false;
        uint64_t meta_offset = 0;
        uint64_t method_offset = 0;
        //fwrite(&meta_offset, sizeof(meta_offset), 1, f);
        //fwrite(&method_offset, sizeof(meta_offset), 1, f);
    }
    else
    {
        f = fopen(fname.c_str(), "ab+");
    }
    
    unsigned char opcode = 0;
        
    if(expr == "push") opcode = OPCODE_PUSH;
    if(expr == "ref")  opcode = OPCODE_REF;
    if(expr == "int")  opcode = OPCODE_INT;
    if(expr == "i")  opcode = OPCODE_INT;
    if(expr == "f")  opcode = OPCODE_FLOAT;
    if(expr == "real")  opcode = OPCODE_FLOAT;
    if(expr == "string")  opcode = OPCODE_STRING;
    if(expr == "call") opcode = OPCODE_CALL;
    if(expr == "mov") opcode = OPCODE_MOV;
    if(expr == "inc") opcode = OPCODE_INC;
    if(expr == "reg") opcode = OPCODE_REG;
    if(expr == "exit") opcode = OPCODE_EXIT;
    if(expr == "add") opcode = OPCODE_ADD;
    if(expr == "sub") opcode = OPCODE_SUB;
    if(expr == "mul") opcode = OPCODE_MUL;
    if(expr == "div") opcode = OPCODE_DIV;
    if(expr == "mod") opcode = OPCODE_MOD;
    if(expr == "eq") opcode = OPCODE_EQ;
    if(expr == "jlbf") opcode = OPCODE_JLBF;
    if(expr == "jmp") opcode = OPCODE_JMP;
    if(expr == "marks") opcode = OPCODE_MARKS;
    
    if(isnumber((expr.c_str())))
    {
        opcode = OPCODE_IMMEDIATE;
    }
    
    if(opcode)
    {
        if(opcode == OPCODE_IMMEDIATE && opcode_counter > 2 && opcodes[opcode_counter - 2] == OPCODE_REG)
        {
            // do not write it if it counts registers
        }
        else
        {
            write_stuff_to_file(f, opcode, 1);
        }
        opcodes.push_back(opcode);
        opcode_counter ++;
        last_opcode = opcode;
        built_upvar_name = "";
        if(opcode == OPCODE_IMMEDIATE)
        {
            uint64_t nr = atoi(expr.c_str());
            write_stuff_to_file(f, nr, 1);
        }
    }
    else
    {
        if(s[ 0 ] == '@') // function call
        {
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
        {
            if(last_opcode != OPCODE_JLBF && last_opcode != OPCODE_JMP)
            { // this is a plain variable 
                int idx = -1;
                for(unsigned int i=0; i<variables.size(); i++)
                {
                    if(variables[i]->name == expr)
                    {
                        idx = i;
                    }
                }
                
                if(idx > -1)
                {
                    write_stuff_to_file(f, OPCODE_VAR, 1);
                    write_stuff_to_file(f, variables[idx]->stack_location, 1);
                }
                else
                {
                    bc_variable_entry* new_var = new bc_variable_entry;
                    new_var->stack_location = var_counter;
                    new_var->name = expr;
                    variables.push_back(new_var);
                    write_stuff_to_file(f, var_counter, 1);
                    var_counter ++;
                }
            }
            else
            { // this is a label
                
            }
        }
    }    
    fclose(f);
}
