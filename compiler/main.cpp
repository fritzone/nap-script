#include "call_ctx.h"
#include "compiler.h"

int main(int argc, char* argv[])
{
    call_context* cur_cc = call_context_create(0, "global", NULL, NULL) ;
    global_cc = cur_cc;

    method* cur_method = NULL;
    const char* file_name = argc > 1?argv[1]:"test.nap";

    load_file(cur_cc, file_name, cur_method);
    call_context_compile(global_cc);
    return 1;
}
