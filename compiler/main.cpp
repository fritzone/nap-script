#include "call_ctx.h"
#include "compiler.h"
#include "garbage_bin.h"

#include "nap_types.h"

int main(int argc, char* argv[])
{
    const char* file_name = "test.nap";
    const char* bc_file_name = "test.ncb";

    bool print_asm = true;
    for(int i=0; i<argc; i++)
    {
        if(!strcmp(argv[i], "-a"))
        {
            print_asm = true;
        }
        if(!strcmp(argv[i], "-i"))
        {
            if(i + 1 < argc)
            {
                file_name = argv[i + 1];
            }
        }
        if(!strcmp(argv[i], "-o"))
        {
            if(i + 1 < argc)
            {
                bc_file_name = argv[i + 1];
            }
        }
    }
    std::auto_ptr<nap_compiler> c = nap_compiler::create_compiler();
    if(print_asm)
    {
        c->print_assemblies();
    }
    bool success = true;
    c->load_file(file_name, success);
    if(!success)
    {
        nap_compiler::release_compiler(c);
        garbage_bin_bin::shutdown();
        return 1;
    }

    c->compile();
    if(c->get_error() == "OK")
    {
        c->write_bytecode(bc_file_name);
    }
    nap_compiler::release_compiler(c);

    garbage_bin_bin::shutdown();

    return 0;
}
