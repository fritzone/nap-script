#ifndef _COMPILER_H_
#define _COMPILER_H_

struct call_context;
struct method;
struct parsed_file;

#include <string>
#include <vector>

class nap_compiler
{
public:

    nap_compiler();
    ~nap_compiler();

    void load_file(const char* file_name);
    void compile();
    void write_bytecode(const char* file_name);

private:

    call_context* cur_cc;
    method* cur_method;
    parsed_file* pf;
    call_context* global_cc;
    std::vector<std::string> loaded_files;
    std::vector<unsigned char> bytecode;

};


#endif

