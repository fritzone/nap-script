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
    cur_cc = new call_context(this, call_context::CC_GLOBAL, "global", NULL, NULL) ;
    global_cc = cur_cc;
    cur_method = 0;
}

nap_compiler::~nap_compiler()
{
    if(garbage_bin_bin::minstance)
    {
        mgbb.empty(this);
    }

    for(size_t i=0; i<mexternal_methods.size(); i++)
    {
        delete mexternal_methods[i];
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
    parse(psuccess);
    return psuccess;
}

int nap_compiler::file_index_for_name(const std::string& n) const
{
    std::vector<std::string>::const_iterator it = std::find(loaded_files.begin(), loaded_files.end(), n);
    return std::distance(loaded_files.begin(), it);
}

std::string nap_compiler::filename(size_t i) const
{
    if(i < loaded_files.size())
    {
        return loaded_files.at(i);
    }
    else
    {
        return "N/A";
    }
}

void nap_compiler::add_external_method(method *m)
{
    mexternal_methods.push_back(m);
}

void nap_compiler::parse(bool& psuccess)
{
    int level = -1; /* currently we are in a global context */
    char delim = 0;
    expression_with_location* expwloc = mpf->parser_next_phrase(&delim);
    while(expwloc)
    {
        if(starts_with(expwloc->expression, "import"))
        {
            std::string exp_trim = expwloc->expression.substr(6);
            exp_trim = strim(exp_trim);

            if(std::find(loaded_files.begin(), loaded_files.end(), exp_trim) == loaded_files.end())
            {
                loaded_files.push_back(exp_trim);
                load_file(exp_trim, psuccess);
                if(!psuccess)
                {
                    return;
                }

                expwloc = mpf->parser_next_phrase(&delim);
            }
        }
        else
        {
            mpf->load_next_single_phrase(expwloc, cur_method, cur_cc, &delim, level, psuccess);
            if(!psuccess)
            {
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
    parse(psuccess);
    if(psuccess)
    {
        loaded_files.push_back(file_name);
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
    code_finalizer::instance()->finalize(this);

    FILE* fp = fopen(file_name, "wb+");
    void* t = bytecode.data();
    fwrite(t, 1, bytecode.size(), fp);
    fclose(fp);
    code_finalizer::destroy();
}


void nap_compiler::deliver_bytecode(uint8_t *&location, size_t &len)
{
    code_finalizer::instance()->finalize(this);
    len = bytecode.size();
    location = (uint8_t*)calloc(len, sizeof(char));
	if(location == NULL)
	{
		len = 0;
		return;
	}
    memcpy(const_cast<uint8_t*>(location), bytecode.data(), len);
    code_finalizer::destroy();
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

std::string nap_compiler::prepare_location() const
{
    std::stringstream ss;
    ss << "expr:[" << exp_w_location->expression
       << "]";

    if(exp_w_location->location.mfile_index != -1)
    {
        ss << "file:[" << exp_w_location->location.mfile_index
           << "] ~line:[" << exp_w_location->location.start_line_number << "]";
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
    exp_w_location = loc;
}

std::auto_ptr<nap_compiler> nap_compiler::create_compiler()
{
    return std::auto_ptr<nap_compiler>(new nap_compiler);
}

void nap_compiler::release_compiler(std::auto_ptr<nap_compiler> &c)
{
    delete c.release();
}
