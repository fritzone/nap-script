#include "compiler.h"

#include <vector>
#include <string>
#include <algorithm>

#include "consts.h"
#include "utils.h"
#include "method.h"
#include "call_ctx.h"
#include "parser.h"
#include "code_stream.h"

void nap_compiler::load_file(const char* file_name)
{
    int level = -1; /* currently we are in a global context */
    expression_with_location* expwloc = NULL;
    char delim;
    pf = parsed_file::open_file(file_name);
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
                load_file(file_to_load);
                expwloc = pf->parser_next_phrase(&delim);
            }
        }
        else
        {
            pf->load_next_single_phrase(expwloc, cur_method, cur_cc, &delim, level);
            expwloc = pf->parser_next_phrase(&delim);
        }
    }
}

void nap_compiler::compile()
{
    global_cc->compile(this);
}

void nap_compiler::write_bytecode(const char* file_name)
{
    code_finalizer(this).finalize();

    FILE* fp = fopen(file_name, "wb+");
    void* t = bytecode.data();
    fwrite(t, 1, bytecode.size(), fp);
    fclose(fp);
}
unsigned char nap_compiler::getLastOpcode() const
{
    return last_opcode;
}

void nap_compiler::setLastOpcode(unsigned char value)
{
    last_opcode = value;
}


nap_compiler::nap_compiler() : opcode_counter(0),
    mfirst_entry(true), mvar_counter(0), minterpreter(this)
{
    cur_cc = new call_context(this, 0, "global", NULL, NULL) ;
    global_cc = cur_cc;
    cur_method = 0;
}

nap_compiler::~nap_compiler()
{
    delete global_cc;
    delete pf;
}

void nap_compiler::place_bytes(int pos, const void *addr, int count)
{
    if(pos < (int)bytecode.size() && pos != -1)
    {
        for(int i=0; i<count; i++)
        {
            bytecode[i + pos] = *((char*)addr + i) ;
        }
    }
    else
    for(int i=0; i<count; i++)
    {
        bytecode.push_back( *((char*)addr + i) );
    }
}
