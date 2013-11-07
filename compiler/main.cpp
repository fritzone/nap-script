#include "call_ctx.h"
#include "compiler.h"
#include "garbage_bin.h"

int main(int argc, char* argv[])
{
    const char* file_name = argc > 1?argv[1]:"test.nap";

    nap_compiler c;
    c.load_file(c.cur_cc, file_name, c.cur_method);
    c.compile();

    garbage_bin_bin::instance().empty();
    garbage_bin_bin::shutdown();

    return 1;


}
