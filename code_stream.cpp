#include "code_stream.h"
#include "is.h"
#include "utils.h"
#include "type.h"
#include "opcodes.h"
#include "throw_error.h"

#include <string>
#include <vector>
#include <stdlib.h>
#include <limits.h>

static const char BITSIZE = 0x32;
const char* NEWLINE = "\n";

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
    NUMBER_INTEGER_TYPE meta_location;
    std::string name;    
};

/**
 * The named marks
 */
struct bc_named_marks
{
    uint32_t marker_code;
    std::string marker_name;
};

struct label_entry
{
    // where this is in the bytecode stream
    NUMBER_INTEGER_TYPE bytecode_location;
    
    // and the name of it
    std::string name;
};

struct bc_string_table_entry
{
    NUMBER_INTEGER_TYPE index;
    uint32_t length;
    std::string the_string;
};

unsigned char code_stream::last_opcode = 0;

// a list of variables that will be added to the "meta" section of the bytecode file
static std::vector<bc_variable_entry*> variables;

// the string table of the aplication
static std::vector<bc_string_table_entry*> stringtable;

// the table holding all the jump location in the code
static std::vector<label_entry*> jumptable;

static std::vector<bc_named_marks> namedmarks;

// this counts the variables
static NUMBER_INTEGER_TYPE var_counter = 0;

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
        fseek(f, 0, SEEK_END);

        uint32_t meta_location = ftell(f);
        // write out the variables
        static const char METATABLE[] = ".meta";
        fwrite(METATABLE, 5, 1, f);
        uint32_t meta_count = variables.size();
        fwrite(&meta_count, sizeof(uint32_t), 1, f);
        for(unsigned int i=0; i<meta_count; i++)
        {
            fwrite(&variables[i]->meta_location, sizeof(NUMBER_INTEGER_TYPE), 1, f);
            uint16_t var_name_length = variables[i]->name.length();
            fwrite(&var_name_length, sizeof(uint16_t), 1, f);
            const char* vname = variables[i]->name.c_str();
            fwrite(vname, sizeof(uint8_t), var_name_length, f);
        }

        uint32_t strtable_location = ftell(f);
        // write out the stringtable
        static const char STRINGTABLE[] = ".str";
        fwrite(STRINGTABLE, 4, 1, f);
        uint32_t strtable_count = stringtable.size();
        fwrite(&strtable_count, sizeof(uint32_t), 1, f);
        for(unsigned int i=0; i<strtable_count; i++)
        {
            fwrite(&stringtable[i]->index, sizeof(NUMBER_INTEGER_TYPE), 1, f);
            fwrite(&stringtable[i]->length, sizeof(uint32_t), 1, f);
            const char* str = stringtable[i]->the_string.c_str();
            fwrite(str, sizeof(uint8_t), stringtable[i]->length, f);
        }
        
        uint32_t jumptable_location = ftell(f);
        // write out the jumptable
        static const char JUMPTABLE[] = ".jmp";
        fwrite(JUMPTABLE, 4, 1, f);
        uint32_t jumptable_count = jumptable.size();
        fwrite(&jumptable_count, sizeof(uint32_t), 1, f);
        for(unsigned int i=0; i<jumptable_count; i++)
        {
            fwrite(&jumptable[i]->bytecode_location, sizeof(NUMBER_INTEGER_TYPE), 1, f);
        }
        
        
        fclose(f);
        
        f = fopen(fname.c_str(), "r+");

        uint8_t type = BITSIZE;
        fwrite(&type, sizeof(type), 1, f);  // the type of the file: 32 bit
        fwrite(&meta_location, sizeof(meta_location), 1, f); // the meta location for variables
        fwrite(&strtable_location, sizeof(strtable_location), 1, f); // the stringtable location         
        fwrite(&jumptable_location, sizeof(jumptable_location), 1, f); // the jumptable location

        fclose(f);
        
    }
};

// dummy object, we need its destructor
static code_finalizer code_f;

static NUMBER_INTEGER_TYPE last_file_pos = 0;

template <class T> NUMBER_INTEGER_TYPE write_stuff_to_file(FILE* fp, T stuff, int cnt)
{
    fwrite(&stuff, sizeof(T), cnt, fp);
    return last_file_pos = ftell(fp);
}

void code_stream::output_bytecode(const char* s)
{
    std::string expr = s;
    if(expr == " " || expr == "(" || expr == ")" || expr == "\n" || expr == ",")
    {
        printf("%s ", s);
        return;
    }

    printf("%s ", s);

    FILE* f = NULL;
    
    if(first_entry)
    {
        f = fopen(fname.c_str(), "wb+");
        first_entry = false;
        NUMBER_INTEGER_TYPE temp = 0;
        uint8_t type = BITSIZE;

        fwrite(&type, sizeof(type), 1, f);  // the type of the file: 32 bit
        fwrite(&temp, sizeof(temp), 1, f); // the meta location for variables
        fwrite(&temp, sizeof(temp), 1, f); // the stringtable location         
        fwrite(&temp, sizeof(temp), 1, f); // the jumptable location
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
    if(expr == "c")  opcode = OPCODE_CHAR;
    if(expr == "char")  opcode = OPCODE_CHAR;
    if(expr == "f")
    {
        if(last_opcode != OPCODE_JLBF && last_opcode != OPCODE_JMP && last_opcode != OPCODE_CALL)
        {
            opcode = OPCODE_FLOAT;
        }
    }
    if(expr == "real")  opcode = OPCODE_FLOAT;
    if(expr == "string")  opcode = OPCODE_STRING;
    if(expr == "s")  opcode = OPCODE_STRING;
    if(expr == "idx")  opcode = OPCODE_IDX;
    if(expr == "call")
    {
        opcode = OPCODE_CALL;
    }
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
    if(expr == "marksn")
    {
        opcode = OPCODE_MARKS_NAME;
    }
    if(expr == "clrsn") opcode = OPCODE_CLRS_NAME;
    if(expr == "return") opcode = OPCODE_RETURN;
    if(expr == "pop") opcode = OPCODE_POP;
    if(expr == "peek") opcode = OPCODE_PEEK;
    if(expr == "clidx")  opcode = OPCODE_CLIDX;

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
            if(opcode_counter > 3 && opcodes[opcode_counter - 3] == OPCODE_REG)
            { // if this counts a register
                uint8_t nrf = atoi(expr.c_str());
                write_stuff_to_file(f,nrf, 1);
            }
            else
            {
                NUMBER_INTEGER_TYPE nr = atoi(expr.c_str());
                
                // the size of the number
                uint8_t type = OPCODE_BYTE;
                if(nr > 255) type = OPCODE_SHORT;
                if(nr > 65535) type = OPCODE_LONG;
                if(nr > 4294967294) type = OPCODE_HUGE;
                
                // if the value is signed or not
                uint8_t sign = 0;
                
                sign = sign; // TODO: work this
                
                // if yes, switch on the first bit in the type
                if(expr[0] == '-') type |= 0x80;
                write_stuff_to_file(f, type, 1);
                
                // and now write the number according to the size
                if(type == OPCODE_BYTE)
                {
                    int8_t nrf = atoi(expr.c_str());
                    write_stuff_to_file(f, (uint8_t)nrf, 1);
                }
                if(type == OPCODE_SHORT)
                {
                    int16_t nrf = atoi(expr.c_str());
                    write_stuff_to_file(f, (uint16_t)nrf, 1);
                }
                if(type == OPCODE_LONG)
                {
                    int32_t nrf = atoi(expr.c_str());
                    write_stuff_to_file(f, (uint32_t)nrf, 1);
                }
                if(type == OPCODE_HUGE)
                {
                    int64_t nrf = atoi(expr.c_str());
                    write_stuff_to_file(f, (uint64_t)nrf, 1);
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
            for(unsigned int i=0; i<jumptable.size(); i++)
            {
                if(!strcmp(jumptable[i]->name.c_str(), lblName.c_str()))
                {
                    idx = i;
                }
            }
            if(idx > -1) // found a label
            {
                jumptable[idx]->bytecode_location = last_file_pos;
            }
            else /* possibly creating a label before using it */
            {
                label_entry* le = new label_entry;
                le->bytecode_location = last_file_pos;
                le->name = lblName;
                jumptable.push_back(le);
            }
        }
        else
        if(s[ 0 ] == '@') // function call
        {
            if(last_opcode == OPCODE_MOV)
            { // possibly a @#ccidx, @#grow
                if(s[1] == '#') // builtin function
                {
                    if(expr == "@#ccidx")
                    {
                        write_stuff_to_file(f, OPCODE_CCIDX, 1);
                    }
                    if(expr == "@#grow")
                    {
                        write_stuff_to_file(f, OPCODE_GROW, 1);
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
            entry->index = stringtable.size();
            entry->the_string = expr.substr(1, expr.length() - 2);
            entry->length = expr.length() - 2;
            stringtable.push_back(entry);
            write_stuff_to_file(f, OPCODE_STRING, 1);
            write_stuff_to_file(f, entry->index, 1);            
        }
        else
        {
            if(last_opcode == OPCODE_MARKS_NAME || last_opcode == OPCODE_CLRS_NAME)
            {
                // this is a named mark
                int idx = -1;
                for(unsigned int i=0; i<namedmarks.size(); i++)
                {
                    if(namedmarks[i].marker_name == expr)
                    {
                        idx = i;
                    }
                }
                if(idx > -1) // found a named mark
                {
                    NUMBER_INTEGER_TYPE index = idx;
                    write_stuff_to_file(f, index, 1);
                }
                else // let's create a named mark
                {
                    bc_named_marks mark;
                    mark.marker_code = namedmarks.size();
                    mark.marker_name = expr;

                    namedmarks.push_back(mark);
                    NUMBER_INTEGER_TYPE index = namedmarks.size() - 1; // the real idx
                    write_stuff_to_file(f, index, 1);
                }

            }
            else
            if(last_opcode != OPCODE_JLBF && last_opcode != OPCODE_JMP && last_opcode != OPCODE_CALL)
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
                    write_stuff_to_file(f, variables[idx]->meta_location, 1);
                }
                else
                {
                    bc_variable_entry* new_var = new bc_variable_entry;
                    new_var->meta_location = var_counter;
                    new_var->name = expr;
                    variables.push_back(new_var);
                    write_stuff_to_file(f, OPCODE_VAR, 1);
                    write_stuff_to_file(f, var_counter, 1);
                    var_counter ++;
                }
            }
            else
            { // this is a label but also a function call (which is treated as a label)
                int idx = -1;
                for(unsigned int i=0; i<jumptable.size(); i++)
                {
                    if(jumptable[i]->name == expr)
                    {
                        idx = i;
                    }
                }
                if(idx > -1) // found a label
                {
                    NUMBER_INTEGER_TYPE index = idx;
                    write_stuff_to_file(f, index, 1);
                }
                else // let's create a label
                {
                    label_entry* le = new label_entry;
                    le->bytecode_location = 0;
                    le->name = expr;
                    jumptable.push_back(le);
                    NUMBER_INTEGER_TYPE index = jumptable.size() - 1; // the real idx
                    write_stuff_to_file(f, index, 1);
                }                
            }
        }
    }    
    fclose(f);
}

