#include "call_ctx.h"
#include "compiler.h"

int main(int argc, char* argv[])
{
    const char* file_name = argc > 1?argv[1]:"test.nap";

    nap_compiler c;
    c.load_file(c.cur_cc, file_name, c.cur_method);
    c.compile();
    return 1;
}
