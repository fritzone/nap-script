#include "call_ctx.h"
#include "compiler.h"
#include "garbage_bin.h"

int main(int argc, char* argv[])
{
    const char* file_name = argc > 1?argv[1]:"test.nap";
    const char* bc_file_name = argc > 2?argv[2]:"test.ncb";

    nap_compiler c;
    c.load_file(file_name);
    c.compile();
    c.write_bytecode(bc_file_name);

    garbage_bin_bin::shutdown();

    return 1;


}
