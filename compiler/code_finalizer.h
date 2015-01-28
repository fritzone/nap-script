#ifndef CODE_FINALIZER_H
#define CODE_FINALIZER_H

#include <vector>
#include <string>
#include <stdint.h>

class nap_compiler;
class file_abstraction;

/**
 * Helper class: its destructor is executed on the application exit and this
 * finalizes the bytecode file
 **/
class code_finalizer
{
public:

    static code_finalizer* instance();
    static void destroy();
    void finalize(nap_compiler* _compiler);
    void add_assembly_command(const std::string& cmd);

private:
    code_finalizer() {}

    void finalize_metatable();
    void finalize_strtable();
    void finalize_jumptable();
    void finalize_funtable();
    void finalize_locations();
    void finalize_classtable();
    bool is_asm_command_word(const std::string& expr);

    nap_compiler* mcompiler;

    uint32_t meta_location ;
    uint32_t strtable_location ;
    uint32_t jumptable_location ;
    uint32_t funtable_location ;
    uint32_t classtable_location;
    file_abstraction* f;

    uint32_t jumptable_count;

    std::vector<std::string> assembly_commands;
    static code_finalizer* minstance;
};


#endif // CODE_FINALIZER_H
