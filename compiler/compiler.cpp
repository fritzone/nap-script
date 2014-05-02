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
#include "code_finalizer.h"


nap_compiler::nap_compiler() : opcode_counter(0),
    mfirst_entry(true), mvm_chain(0), mvar_counter(0), minterpreter(this), mgbb(garbage_bin_bin::instance()),
    mfinalError("OK"), mErrorCode(0), mEmbedded(false), print_assembly(false)
{
    cur_cc = new call_context(this, 0, "global", NULL, NULL) ;
    global_cc = cur_cc;
    cur_method = 0;
}

nap_compiler::~nap_compiler()
{
    if(garbage_bin_bin::minstance)
    {
        mgbb.empty(this);
    }
    delete global_cc;
    delete mpf;
}

bool nap_compiler::set_source(const char *src, bool& psuccess)
{
    mpf = parsed_file::set_source(src, this);
    if(!mpf)
    {
        return false;
    }

    mEmbedded = true;
    bool success = true;
    parse(success);
    if(!success)
    {
        psuccess = false;
        return 0;
    }

    return true;
}

void nap_compiler::parse(bool& psuccess)
{
    int level = -1; /* currently we are in a global context */
    char delim = 0;
    expression_with_location* expwloc = mpf->parser_next_phrase(&delim);
    while(expwloc)
    {
        if(strstr(expwloc->expression, "import") == expwloc->expression)
        {
            std::string exp_trim = expwloc->expression + 6;
            exp_trim = strim(exp_trim);

            if(std::find(loaded_files.begin(), loaded_files.end(), exp_trim) == loaded_files.end())
            {
                loaded_files.push_back(exp_trim);
                bool success = true;
                load_file(exp_trim, success);
                if(!success)
                {
                    psuccess = false;
                    return;
                }

                expwloc = mpf->parser_next_phrase(&delim);
            }
        }
        else
        {
            bool success = true;
            mpf->load_next_single_phrase(expwloc, cur_method, cur_cc, &delim, level, success);
            if(!success)
            {
                psuccess = false;
                return;
            }

            expwloc = mpf->parser_next_phrase(&delim);
        }
    }
}


void nap_compiler::load_file(const std::string & file_name, bool &psuccess)
{
    mpf = mpf->open_file(file_name, this);
    if(!mpf)
    {
        return;
    }
    bool success = true;
    parse(success);
    if(!success)
    {
        psuccess = false;
        return;
    }

}

bool nap_compiler::compile()
{
    bool success = true;
    global_cc->compile(this, success);
    if(!success)
    {
        return false;
    }

    if(mErrorCode == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
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
	if(location == NULL)
	{
		len = 0;
		return;
	}
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

    if(!location->location->file_name.empty())
    {
        ss << "file:[" << location->location->file_name
           << "] ~line:[" << location->location->start_line_number << "]";
    }
    mErrorCode = 1;
    return ss.str();
}
call_context *nap_compiler::getGlobalCc() const
{
    return global_cc;
}

void nap_compiler::setGlobal_cc(call_context *value)
{
    global_cc = value;
}


void nap_compiler::throw_error(const char* error) const
{
    std::string loc = prepare_location();
    std::stringstream ss;
    ss << "[err:compiler] " << error << " @ " << loc;
    mfinalError = ss.str();
    if(!mEmbedded)
    {
        std::cerr << mfinalError << std::endl ;
    }
}

void nap_compiler::throw_error(const char* error, const std::string& par1, const std::string& par2) const
{
    std::string loc = prepare_location();
    std::stringstream ss;
    ss << "[err:compiler] " << error << ": [" << par1 << "] - " << par2 << " @ " << loc;
    mfinalError = ss.str();
    if(!mEmbedded)
    {
        std::cerr << mfinalError << std::endl;
    }
}

void nap_compiler::throw_error(const char* error, const std::string& par) const
{
    std::string loc = prepare_location();
    std::stringstream ss;
    ss << "[err:compiler] " << error << ": [" << par << "] @ " << loc;
    mfinalError = ss.str();
    if(!mEmbedded)
    {
        std::cerr << mfinalError << std::endl;
    }
}

void nap_compiler::throw_error(const char* error, int id, const char* par) const
{
    std::string loc = prepare_location();
    std::stringstream ss;
    ss << "[err:compiler] " << error << " id[" << id << "] - [" << par << "] @ " << loc;
    mfinalError = ss.str();
    if(!mEmbedded)
    {
        std::cerr << mfinalError << std::endl;
    }
}

void nap_compiler::throw_index_out_of_range(const char* variable_name, int maximum_allowed, int got) const
{
    std::string loc = prepare_location();
    std::stringstream ss;
    ss << "[err:compiler] index ["<< got << "] out of range for ["
       << variable_name << ":"<<maximum_allowed << "]" << " @ " << loc;
    mfinalError = ss.str();
    if(!mEmbedded)
    {
        std::cerr << mfinalError << std::endl;
    }
}

void nap_compiler::set_location(const expression_with_location* loc)
{
    location = loc;
}

std::auto_ptr<nap_compiler> nap_compiler::create_compiler()
{
    return std::auto_ptr<nap_compiler>(new nap_compiler);
}

void nap_compiler::release_compiler(std::auto_ptr<nap_compiler> &c)
{
    delete c.release();
}
