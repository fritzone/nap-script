#ifndef _COMPILER_H_
#define _COMPILER_H_

struct call_context;
struct method;
struct parsed_file;
class garbage_bin_bin;

#include "interpreter.h"

#include <string>
#include <vector>
#include <memory>

#include "type.h"

#ifdef _WINDOWS
	#ifdef NAP_CPL_BUILT_AS_SHARED
		#include "nap_cpl_exp.h"
	#elif defined NAP_CPL_BUILT_AS_STATIC
		#include "nap_cpl_s_exp.h"
	#else
		#define NAP_LIB_API
	#endif
#endif

/**
 * The named marks
 */
struct bc_named_marks
{
    uint32_t marker_code;
    std::string marker_name;
};

/**
 * the description of a variable from the binary file
 */
struct bc_variable_entry
{
    NUMBER_INTEGER_TYPE meta_location;
    std::string name;
    uint8_t type; // 0 - internal, 1 - external (from the parent VM)
};


struct bc_string_table_entry
{
    NUMBER_INTEGER_TYPE index;
    uint32_t length;
    std::string the_string;
};

struct label_entry
{
    label_entry() : bytecode_location(0), type(0), name() {}

    // where this is in the bytecode stream
    NUMBER_INTEGER_TYPE bytecode_location;

    // if this is a function (1) or not (0) or method call (2)
    uint8_t type;

    // and the name of it
    std::string name;
};

struct nap_vm ;

/**
 * @brief The nap_compiler class represents a compiler.
 */
class NAP_LIB_API nap_compiler
{

    nap_compiler();
    ~nap_compiler();
    nap_compiler& operator = (const nap_compiler&);

public:

    static std::auto_ptr<nap_compiler> create_compiler();
    friend class std::auto_ptr<nap_compiler>;

    static void release_compiler(std::auto_ptr<nap_compiler>& compiler);

    void load_file(const std::string &file_name, bool& psuccess);
    bool compile();
    void write_bytecode(const char* file_name);

    unsigned char getLastOpcode() const;
    void setLastOpcode(unsigned char value);

    std::vector<bc_variable_entry*>& variables()
    {
        return mvariables;
    }

    void add_variable(bc_variable_entry* new_var)
    {
        mvariables.push_back(new_var);
    }

    const std::vector<bc_string_table_entry*>& stringtable() const
    {
        return mstringtable;
    }

    void add_stringtable_entry(bc_string_table_entry* entry)
    {
        mstringtable.push_back(entry);
    }

    const std::vector<label_entry*>& jumptable() const
    {
        return mjumptable;
    }

    void add_jumptable_entry(label_entry* le)
    {
        mjumptable.push_back(le);
    }

    NUMBER_INTEGER_TYPE var_counter() const
    {
        return mvar_counter;
    }

    void inc_var_counter()
    {
        mvar_counter ++;
    }

    const std::vector<bc_named_marks>& namedmarks() const
    {
        return mnamedmarks;
    }

    void add_mark(bc_named_marks mark)
    {
        mnamedmarks.push_back(mark);
    }

    const std::vector<unsigned char>& opcodes() const
    {
        return mopcodes;
    }

    void add_opcode(unsigned char opc)
    {
        mopcodes.push_back(opc);
    }

    // counts the opcodes
    int opcode_counter;
    // if the very first byte writte then also create the file
    bool mfirst_entry;

    /**
     * @brief place_bytes places bytes in the bytecode vector
     * @param addr
     * @param count
     */
    void place_bytes(int pos, const void *addr, int count);

    NUMBER_INTEGER_TYPE last_bc_pos()
    {
        return bytecode.size();
    }

    interpreter& get_interpreter()
    {
        return minterpreter;
    }

    bool set_source(const char* src, bool &psuccess);

    void parse(bool &psuccess);

    void deliver_bytecode(uint8_t*& location, size_t& label_entry);

    /**
     * Duplicates src
     */
    char* duplicate_string(const char* s) const;

    /**
     * Creates a new string
     */
    char* new_string(int size) const;

    void throw_error(const char* error) const;

    void throw_error(const char* error, const std::string& par) const ;

    void throw_error(const char* error, int id, const char* par) const ;

    void throw_index_out_of_range(const char* variable_name, int maximum_allowed, int got) const ;

    void throw_error(const char* error, const std::string &par1, const std::string &par2) const;

    void set_location(const expression_with_location* loc);

    std::string prepare_location() const;

    int getErrorCode() const
    {
        return mErrorCode;
    }

    std::string get_error() const
    {
        return mfinalError;
    }

    const struct nap_vm* vm_chain() const
    {
        return mvm_chain;
    }

    void set_vmchain(struct nap_vm* vm)
    {
        mvm_chain = vm;
    }

private:

    call_context* cur_cc;
    method* cur_method;
    parsed_file* mpf;
    call_context* global_cc;
    std::vector<std::string> loaded_files;
    std::vector<unsigned char> bytecode;

    // taken over from the code_stream.cpp, static stuff
    unsigned char last_opcode;

    // a list of variables that will be added to the "meta" section of the bytecode file
    std::vector<bc_variable_entry*> mvariables;

    // the string table of the aplication
    std::vector<bc_string_table_entry*> mstringtable;

    // the table holding all the jump location in the code
    std::vector<label_entry*> mjumptable;

    // this counts the variables
    NUMBER_INTEGER_TYPE mvar_counter;

    std::vector<bc_named_marks> mnamedmarks;

    // the written opcodes are stored in here for later reference
    std::vector<unsigned char> mopcodes;

    interpreter minterpreter;

    garbage_bin_bin& mgbb;

    const expression_with_location* location;

    mutable std::string mfinalError;
    mutable int mErrorCode;
    bool mEmbedded;

    const struct nap_vm *mvm_chain;

    // the last command for the bytecode generation. Used for the debugging feature
    std::string mlast_cmd_for_bytecode;

    friend class code_stream;
};


#endif

