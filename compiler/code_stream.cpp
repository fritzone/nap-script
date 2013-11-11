#include "code_stream.h"
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
static uint8_t max_reg_count = 0;

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

class file_abstraction
{
public:

    file_abstraction() : fp(0), last_file_pos(0)
    {
    }

    file_abstraction(const char* f, const char* mode) : last_file_pos(0)
    {
        fp = fopen(f, mode);
    }

    void open(const char* f, const char* mode)
    {
        fp = fopen(f, mode);
    }

    ~file_abstraction()
    {
        fclose(fp);
    }

    template <class T> NUMBER_INTEGER_TYPE write_stuff_to_file(T stuff, int cnt)
    {
        fwrite(&stuff, sizeof(T), cnt, fp);
        return last_file_pos = ftell();
    }

    NUMBER_INTEGER_TYPE write_string_to_file(const char* s, int cnt)
    {
        fwrite(s, cnt, 1, fp);
        return last_file_pos = ftell();
    }

    void seek_end()
    {
        fseek(fp, 0, SEEK_END);
    }

    long ftell() const
    {
        return ::ftell(fp);
    }

    NUMBER_INTEGER_TYPE get_last_file_pos() const
    {
        return last_file_pos;
    }

private:

    FILE *fp;
    NUMBER_INTEGER_TYPE last_file_pos;
    long currentIndex;
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
    file_abstraction f(fname.c_str(), "ab+");

    meta_location = f.ftell();
    // write out the variables
    static const char METATABLE[] = ".meta";
    f.write_string_to_file(METATABLE, 5);
    uint32_t meta_count = variables.size();
    f.write_stuff_to_file(meta_count, 1);
    for(unsigned int i=0; i<meta_count; i++)
    {
        f.write_stuff_to_file(variables[i]->meta_location, 1);
        uint16_t var_name_length = variables[i]->name.length();
        f.write_stuff_to_file(var_name_length, 1);
        const char* vname = variables[i]->name.c_str();
        f.write_string_to_file(vname, var_name_length);
    }

    strtable_location = f.ftell();
    // write out the stringtable
    static const char STRINGTABLE[] = ".str";
    f.write_string_to_file(STRINGTABLE, 4);
    uint32_t strtable_count = stringtable.size();
    f.write_stuff_to_file(strtable_count, 1);
    for(unsigned int i=0; i<strtable_count; i++)
    {
        f.write_stuff_to_file(stringtable[i]->index, 1);
        f.write_stuff_to_file(stringtable[i]->length, 1);
        const char* str = stringtable[i]->the_string.c_str();
        f.write_string_to_file(str, stringtable[i]->length);
    }
    jumptable_location = f.ftell();
    // write out the jumptable
    static const char JUMPTABLE[] = ".jmp";
    f.write_string_to_file(JUMPTABLE, 4);
    uint32_t jumptable_count = jumptable.size();
    f.write_stuff_to_file(jumptable_count, 1);
    for(unsigned int i=0; i<jumptable_count; i++)
    {
        f.write_stuff_to_file(jumptable[i]->bytecode_location, 1);
    }

    }

    {

    file_abstraction f(fname.c_str(), "r+");

    uint8_t type = BITSIZE;
    f.write_stuff_to_file(type, 1);  // the type of the file: 32 bit
    f.write_stuff_to_file(meta_location, 1); // the meta location for variables
    f.write_stuff_to_file(strtable_location, 1); // the stringtable location
    f.write_stuff_to_file(jumptable_location, 1); // the jumptable location
    f.write_stuff_to_file(max_reg_count, 1);
    }
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


    file_abstraction f;
    
    if(first_entry)
    {
        f.open(fname.c_str(), "wb+");
        first_entry = false;
        NUMBER_INTEGER_TYPE temp = 0;
        uint8_t type = BITSIZE;
        uint8_t bits = 255;

        f.write_stuff_to_file(type, 1);  // the type of the file: 32 bit
        f.write_stuff_to_file(temp, 1); // the meta location for variables
        f.write_stuff_to_file(temp, 1); // the stringtable location
        f.write_stuff_to_file(temp, 1); // the jumptable location
        f.write_stuff_to_file(bits, 1);  // extra for the max reg count
    }
    else
    {
        f.open(fname.c_str(), "ab+");
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
        if(last_opcode == OPCODE_CCIDX || last_opcode == OPCODE_GROW)
        {
            int8_t nrf = atoi(expr.c_str());
            f.write_stuff_to_file(nrf, 1);
            return;
        }
        else
        {
            opcode = OPCODE_IMMEDIATE;
        }
    }
    
    if(opcode)
    {
        if(opcode == OPCODE_IMMEDIATE && opcode_counter > 2 && opcodes[opcode_counter - 2] == OPCODE_REG)
        {
            // do not write it if it counts registers
        }
        else
        {
            f.write_stuff_to_file(opcode, 1);
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
                f.write_stuff_to_file(nrf, 1);
                if(max_reg_count < nrf)
                {
                    max_reg_count = nrf;
                    if(max_reg_count == 255)
                    {
                        fprintf(stderr, "a too complex operation was attempted in [%s]", g_location->expression);
                        _exit(1);
                    }
                }
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
                f.write_stuff_to_file(type, 1);
                
                // and now write the number according to the size
                if(type == OPCODE_BYTE)
                {
                    int8_t nrf = atoi(expr.c_str());
                    f.write_stuff_to_file((uint8_t)nrf, 1);
                }
                if(type == OPCODE_SHORT)
                {
                    int16_t nrf = atoi(expr.c_str());
                    f.write_stuff_to_file((uint16_t)nrf, 1);
                }
                if(type == OPCODE_LONG)
                {
                    int32_t nrf = atoi(expr.c_str());
                    f.write_stuff_to_file((uint32_t)nrf, 1);
                }
                if(type == OPCODE_HUGE)
                {
                    int64_t nrf = atoi(expr.c_str());
                    f.write_stuff_to_file((uint64_t)nrf, 1);
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
                jumptable[idx]->bytecode_location = f.get_last_file_pos();
            }
            else /* possibly creating a label before using it */
            {
                label_entry* le = new label_entry;
                le->bytecode_location = f.get_last_file_pos();
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
                        f.write_stuff_to_file(OPCODE_CCIDX, 1);
                        last_opcode = OPCODE_CCIDX;
                    }
                    if(expr == "@#grow")
                    {
                        f.write_stuff_to_file(OPCODE_GROW, 1);
                        last_opcode = OPCODE_GROW;
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
            f.write_stuff_to_file(OPCODE_STRING, 1);
            f.write_stuff_to_file(entry->index, 1);
            garbage_bin<bc_string_table_entry*>::instance().place(entry);
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
                    f.write_stuff_to_file(index, 1);
                }
                else // let's create a named mark
                {
                    bc_named_marks mark;
                    mark.marker_code = namedmarks.size();
                    mark.marker_name = expr;

                    namedmarks.push_back(mark);
                    NUMBER_INTEGER_TYPE index = namedmarks.size() - 1; // the real idx
                    f.write_stuff_to_file(index, 1);
                }

            }
            else
            if(last_opcode != OPCODE_JLBF && last_opcode != OPCODE_JMP && last_opcode != OPCODE_CALL)
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
                    for(unsigned int i=0; i<variables.size(); i++)
                    {
                        if(variables[i]->name == expr)
                        {
                            idx = i;
                        }
                    }

                    if(idx > -1)
                    {
                        f.write_stuff_to_file(OPCODE_VAR, 1);
                        f.write_stuff_to_file(variables[idx]->meta_location, 1);
                    }
                    else
                    {
                        bc_variable_entry* new_var = new bc_variable_entry;
                        new_var->meta_location = var_counter;
                        new_var->name = expr;
                        variables.push_back(new_var);
                        f.write_stuff_to_file(OPCODE_VAR, 1);
                        f.write_stuff_to_file(var_counter, 1);
                        var_counter ++;
                        garbage_bin<bc_variable_entry*>::instance().place(new_var);
                    }
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
                    f.write_stuff_to_file(index, 1);
                }
                else // let's create a label
                {
                    label_entry* le = new label_entry;
                    le->bytecode_location = 0;
                    le->name = expr;
                    jumptable.push_back(le);
                    NUMBER_INTEGER_TYPE index = jumptable.size() - 1; // the real idx
                    f.write_stuff_to_file(index, 1);
                    garbage_bin<label_entry*>::instance().place(le);
                }                
            }
        }
    }    

}

