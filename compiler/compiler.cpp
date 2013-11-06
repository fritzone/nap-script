#include "compiler.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include <vector>
#include <string>
#include <algorithm>

#include "hash.h"
#include "tree.h"
#include "consts.h"
#include "variable.h"
#include "utils.h"
#include "interpreter.h"
#include "method.h"
#include "bt_string.h"
#include "type.h"
#include "number.h"
#include "call_ctx.h"
#include "sys_brkp.h"
#include "throw_error.h"
#include "parser.h"
#include "res_wrds.h"

void nap_compiler::load_file(call_context* cc, const char* file_name, method* cur_method)
{
    int level = -1; /* currently we are in a global context */
    expression_with_location* expwloc = NULL;
    char delim;
    parsed_file* pf = parsed_file::open_file(file_name);
    if(!pf)
    {
        return;
    }
    expwloc = pf->parser_next_phrase(&delim);
    while(expwloc)
    {
        char* exp_trim = trim(duplicate_string(expwloc->expression));
        if(strstr(exp_trim, "import") == exp_trim)
        {
            exp_trim += 6;
            char* file_to_load = trim(exp_trim);
            if(std::find(loaded_files.begin(), loaded_files.end(), file_to_load) == loaded_files.end())
            {
                loaded_files.push_back(file_to_load);
                load_file(cc, file_to_load, cur_method);
                expwloc = pf->parser_next_phrase(&delim);
            }
        }
        else
        {
            pf->load_next_single_phrase(expwloc, cur_method, cc, &delim, level);
            expwloc = pf->parser_next_phrase(&delim);
        }
    }
}

void nap_compiler::compile()
{
    call_context_compile(global_cc);
}

nap_compiler::nap_compiler()
{
    cur_cc = call_context_create(0, "global", NULL, NULL) ;
    global_cc = cur_cc;
    cur_method = 0;
}
