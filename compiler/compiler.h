#ifndef _COMPILER_H_
#define _COMPILER_H_

struct call_context;
struct method;
struct parsed_file;
class garbage_bin_bin;
struct nap_vm;

#include "interpreter.h"

#include <string>
#include <vector>
#include <memory>

#include "type.h"

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
    enum VariableType
    {
        VT_INTERNAL = 0,
        VT_EXTERN = 1
    };

    bc_variable_entry(NUMBER_INTEGER_TYPE ploc, const std::string& pname, VariableType pvt = VT_INTERNAL) :
        meta_location(ploc), name(pname), type(pvt)
    {}

    NUMBER_INTEGER_TYPE meta_location;
    std::string name;
    VariableType type; // 0 - internal, 1 - external (from the parent VM)
};


struct bc_string_table_entry
{
    bc_string_table_entry(NUMBER_INTEGER_TYPE pindex, uint32_t plen, const std::string& ps) :
        index(pindex), length(plen), the_string(ps)
    {}

    NUMBER_INTEGER_TYPE index;
    uint32_t length;
    std::string the_string;
};

struct label_entry
{
    enum LabelEntryType
    {
        LE_GENERAL = 0,
        LE_CALL = 1,
        LE_MEHOD_CALL = 2,
        LE_PARENT_CALL = 3,
    };

    label_entry() : bytecode_location(0), type(LE_GENERAL), name() {}

    // where this is in the bytecode stream
    NUMBER_INTEGER_TYPE bytecode_location;

    // if this is a function (1) or not (0) or method call (2)
    LabelEntryType type;

    // and the name of it
    std::string name;
};

/**
 * @brief The nap_compiler class represents a compiler.
 */
class nap_compiler
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

    std::vector<bc_variable_entry>& variables()
    {
        return mvariables;
    }

    void add_variable(bc_variable_entry new_var)
    {
        mvariables.push_back(new_var);
    }

    std::vector<bc_string_table_entry>& stringtable()
    {
        return mstringtable;
    }

    void add_stringtable_entry(bc_string_table_entry entry)
    {
        mstringtable.push_back(entry);
    }

    std::vector<label_entry>& jumptable()
    {
        return mjumptable;
    }

    void add_jumptable_entry(label_entry le)
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

    std::vector<unsigned char>& opcodes()
    {
        return mopcodes;
    }

    void add_opcode(unsigned char opc)
    {
        mopcodes.push_back(opc);
    }

    void modify_last_opcode(unsigned char opc)
    {
        bytecode[bytecode.size() - 1] = opc;
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

    void deliver_bytecode(uint8_t*& exp_w_location, size_t& label_entry);

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

    void print_assemblies()
    {
        print_assembly = true;
    }

    call_context *getGlobalCc() const;
    void setGlobal_cc(call_context *value);

    struct nap_vm *mvm_chain;

    int file_index_for_name(const std::string& n) const;
    std::string filename(size_t) const;

    void add_external_method(method*);

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
    std::vector<bc_variable_entry> mvariables;

    // the string table of the aplication
    std::vector<bc_string_table_entry> mstringtable;

    // the table holding all the jump location in the code
    std::vector<label_entry> mjumptable;

    // this counts the variables
    NUMBER_INTEGER_TYPE mvar_counter;

    std::vector<bc_named_marks> mnamedmarks;

    // the written opcodes are stored in here for later reference
    std::vector<unsigned char> mopcodes;

    interpreter minterpreter;

    garbage_bin_bin& mgbb;

    const expression_with_location* exp_w_location;

    mutable std::string mfinalError;
    mutable int mErrorCode;
    bool mEmbedded;

    // the last command for the bytecode generation. Used for the debugging feature
    std::string mlast_cmd_for_bytecode;
    bool print_assembly;

    // the external methods that are used by this compiler
    std::vector<method*> mexternal_methods;

    friend class code_stream;
};


#endif

