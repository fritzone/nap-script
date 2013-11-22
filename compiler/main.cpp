#include "call_ctx.h"
#include "compiler.h"
#include "garbage_bin.h"

int main(int argc, char* argv[])
{
    const char* file_name = argc > 1?argv[1]:"test.nap";
    const char* bc_file_name = argc > 2?argv[2]:"test.ncb";

    std::auto_ptr<nap_compiler> c = nap_compiler::create_compiler();
    c->load_file(file_name);
    c->compile();
    c->write_bytecode(bc_file_name);
    nap_compiler::release_compiler(c);

    garbage_bin_bin::shutdown();

    return 1;


}
