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

    call_context* cur_cc;
    method* cur_method;

    void load_file(call_context* cc, const char* file_name, method* cur_method);
    void compile();

private:

    parsed_file* pf;
    call_context* global_cc;
    std::vector<std::string> loaded_files;

};


#endif

