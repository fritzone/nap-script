#include "compiler.h"

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

#include "consts.h"
#include "utils.h"
#include "method.h"
#include "call_ctx.h"
#include "parser.h"
#include "code_stream.h"

nap_compiler::nap_compiler() : opcode_counter(0),
    mfirst_entry(true), mvar_counter(0), minterpreter(this), mgbb(garbage_bin_bin::instance())
{
    cur_cc = new call_context(this, 0, "global", NULL, NULL) ;
    global_cc = cur_cc;
    cur_method = 0;

}

nap_compiler::~nap_compiler()
{
    mgbb.empty(this);
    delete global_cc;
    delete mpf;
}

bool nap_compiler::set_source(const char *src)
{
    mpf = parsed_file::set_source(src, this);
    if(!mpf)
    {
        return false;
    }

    parse();
    return true;
}

void nap_compiler::parse()
{
    int level = -1; /* currently we are in a global context */
    char delim = 0;
    expression_with_location* expwloc = mpf->parser_next_phrase(&delim);
    while(expwloc)
    {
        char* exp_trim = trim(duplicate_string(expwloc->expression), this);
        if(strstr(exp_trim, "import") == exp_trim)
        {
            exp_trim += 6;
            char* file_to_load = trim(exp_trim, this);
            if(std::find(loaded_files.begin(), loaded_files.end(), file_to_load) == loaded_files.end())
            {
                loaded_files.push_back(file_to_load);
                load_file(file_to_load);
                expwloc = mpf->parser_next_phrase(&delim);
            }
        }
        else
        {
            mpf->load_next_single_phrase(expwloc, cur_method, cur_cc, &delim, level);
            expwloc = mpf->parser_next_phrase(&delim);
        }
    }
}


void nap_compiler::load_file(const char* file_name)
{
    mpf = mpf->open_file(file_name, this);
    if(!mpf)
    {
        return;
    }

    parse();
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


void nap_compiler::deliver_bytecode(uint8_t *&location, size_t &len)
{
    code_finalizer(this).finalize();
    len = bytecode.size();
    location = (uint8_t*)calloc(len, sizeof(char));
    memcpy(const_cast<uint8_t*>(location), bytecode.data(), len);
}

unsigned char nap_compiler::getLastOpcode() const
{
    return last_opcode;
}

void nap_compiler::setLastOpcode(unsigned char value)
{
    last_opcode = value;
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


/**
 * Duplicates src
 */
char* nap_compiler::duplicate_string(const char* s) const
{
    char *d = alloc_mem(char, strlen(s) + 1, this);   // Space for length plus nul
    if (d == NULL) return NULL;          // No memory
    strcpy (d,s);                        // Copy the characters
    return d;
}


/**
 * Creates a new string
 */
char* nap_compiler::new_string(int size) const
{
    char* tmp = alloc_mem(char, size + 1, this);
    return tmp;
}

std::string nap_compiler::prepare_location() const
{
    std::stringstream ss;
    ss << "expr:[" << location->expression
       << "]";

    if(location->location->file_name)
    {
        ss << "file:[" << location->location->file_name
           << "] ~line:[" << location->location->start_line_number;
    }
    return ss.str();
}

void nap_compiler::throw_error(const char* error) const
{
    std::string loc = prepare_location();
    std::stringstream ss;
    ss << "[err:compiler] " << error << " @ " << loc;
    mfinalError = ss.str();
    std::cerr << mfinalError << std::endl ;
}

void nap_compiler::throw_error(const char* error, const char* par1, const char* par2) const
{
    std::string loc = prepare_location();
    std::stringstream ss;
    ss << "[err:compiler] " << error << ": [" << par1 << "] - " << par2 << " @ " << loc;
    mfinalError = ss.str();
    std::cerr << mfinalError << std::endl;
}

void nap_compiler::throw_error(const char* error, const std::string& par) const
{
    std::string loc = prepare_location();
    std::stringstream ss;
    ss << "[err:compiler] " << error << ": [" << par << "] @ " << loc;
    mfinalError = ss.str();
    std::cerr << mfinalError << std::endl;
}

void nap_compiler::throw_error(const char* error, int id, const char* par) const
{
    std::string loc = prepare_location();
    std::stringstream ss;
    ss << "[err:compiler] " << error << " id[" << id << "] - [" << par << "] @ " << loc;
    mfinalError = ss.str();
    std::cerr << mfinalError << std::endl;
}

void nap_compiler::throw_index_out_of_range(const char* variable_name, int maximum_allowed, int got) const
{
    std::string loc = prepare_location();
    std::stringstream ss;
    ss << "[err:compiler] index ["<< got << "] out of range for ["
       << variable_name << ":"<<maximum_allowed << "]" << " @ " << loc;
    mfinalError = ss.str();
    std::cerr << mfinalError << std::endl;
}

void nap_compiler::set_location(const expression_with_location* loc)
{
    location = loc;
}
