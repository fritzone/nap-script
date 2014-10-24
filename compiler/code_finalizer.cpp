#include "code_finalizer.h"
#include "compiler.h"
#include "charconverter.h"
#include "parameter.h"
#include "code_stream.h"
#include "file_abstraction.h"
extern "C" {
#include "byte_order.h"
}

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>

code_finalizer* code_finalizer::minstance = 0;

/**
 * Helper class: its destructor is executed on the application exit and this
 * finalizes the bytecode file
 **/
void code_finalizer::finalize_metatable()
{

    meta_location = f->ftell();

    // write out the variables
    static const char METATABLE[] = ".meta";
    f->write_string_to_file(METATABLE, 5, 0);
    uint32_t meta_count = mcompiler->variables().size();
    f->write_stuff_to_file_32(meta_count);
    for(unsigned int i=0; i<meta_count; i++)
    {
        f->write_stuff_to_file_32(mcompiler->variables()[i].meta_location);
        std::string ex = "extern";
        if(mcompiler->variables()[i].name.compare(0, ex.length(), ex ) == 0)
        {
            // starts with extern
            mcompiler->variables()[i].type = bc_variable_entry::VT_EXTERN;
        }

        f->write_stuff_to_file_8((uint8_t)mcompiler->variables()[i].type);

        uint16_t var_name_length = (uint16_t)mcompiler->variables()[i].name.length();
        // find out if this is a global/extern variable or not: if there is only one dot
        // in the name, it is a global/extern variable
        // TODO: include a debugging flag option to write out all the variable names
        size_t n = std::count(mcompiler->variables()[i].name.begin(),
                              mcompiler->variables()[i].name.end(), '.');
        if(n == 1 || 1) // global/extern variable. Skip the "global." or "extern." // ||1 to write out all time
        {          // WARNING: this code counts on tha both global and extern have 6 characters.
            uint16_t globlen = (uint16_t)strlen("global") + 1;
            f->write_stuff_to_file_16(var_name_length - globlen);
            const char* vname = mcompiler->variables()[i].name.c_str();
            vname += globlen;
            f->write_string_to_file(vname, var_name_length - globlen, 0);
        }
        else
        {
            f->write_stuff_to_file_16(0); // write 0 to indicate name is not important
        }
    }
}

void code_finalizer::finalize_strtable()
{
    strtable_location = f->ftell();
    // write out the stringtable
    static const char STRINGTABLE[] = ".str";
    f->write_string_to_file(STRINGTABLE, 4, 0);
    uint32_t strtable_count = mcompiler->stringtable().size();
    f->write_stuff_to_file_32(strtable_count);
    for(unsigned int i=0; i<strtable_count; i++)
    {
        f->write_stuff_to_file_32( mcompiler->stringtable()[i].index);
        f->write_stuff_to_file_32( 0 );
        const char* str =  mcompiler->stringtable()[i].the_string.c_str();
        size_t used_len = f->write_string_to_file(str,  mcompiler->stringtable()[i].length, 1);
        // and update the real size
        f->write_stuff_to_file_32(used_len / 4, f->ftell() - used_len - 4);
    }

}

void code_finalizer::finalize_jumptable()
{
    jumptable_location = f->ftell();
    // write out the jumptable
    static const char JUMPTABLE[] = ".jmp";
    f->write_string_to_file(JUMPTABLE, 4, 0);
    jumptable_count = mcompiler->jumptable().size();
    f->write_stuff_to_file_32(jumptable_count);
    f->write_stuff_to_file_32(code_stream::max_mark_index);

    for(unsigned int i=0; i<jumptable_count; i++)
    {
        label_entry& je = mcompiler->jumptable()[i];
        uint16_t l = (uint16_t)je.name.length();

        f->write_stuff_to_file_32(je.bytecode_location); // the actual location in code

        // see if this is a call from the parent
        if(je.name[0] == '-') // indicates the parent
        {
            je.type = label_entry::LE_PARENT_CALL;
            je.name = je.name.substr(je.name.find('.') + 1);
            l = (uint16_t)je.name.length();
        }
        f->write_stuff_to_file_8((uint8_t)je.type);      // 0, 1, 2 .. .see there
        if(je.type == label_entry::LE_CALL
                || je.type == label_entry::LE_MEHOD_CALL
                || je.type ==  label_entry::LE_PARENT_CALL)
        {
            f->write_stuff_to_file_16(l);            // The length of the name
            f->write_string_to_file(je.name.c_str(), l, 0);
        }
    }

}

void code_finalizer::finalize_funtable()
{
    funtable_location = f->ftell();
    // write out the function table
    call_context* cc = mcompiler->getGlobalCc();

    static const char FUNTABLE[] = ".fun";
    f->write_string_to_file(FUNTABLE, 4, 0);
    uint32_t funtable_count = cc->methods.size();
    f->write_stuff_to_file_32(funtable_count);

    for(size_t mc=0; mc<funtable_count; mc++)
    {
        method* m = cc->methods[mc];

        for(uint32_t jc=0; jc<jumptable_count; jc++)
        {
            label_entry je = mcompiler->jumptable()[jc];
            const char *n = je.name.c_str();
            if(je.name.find("global.") != std::string::npos)
            {
                n += 7;
            }

            if(m->method_name == n)
            {
                //write the jumptable index to the file
                f->write_stuff_to_file_32(jc);

                // write the function name
                uint16_t name_len = (uint16_t)strlen(n);
                f->write_stuff_to_file_16(name_len);
                f->write_string_to_file(n, name_len, 0);
                // the return type
                f->write_stuff_to_file_8((uint8_t)m->ret_type);
                // write the parameter count to file- There can be max 255 parameters
                uint8_t pars = (uint8_t)m->parameters.size();
                f->write_stuff_to_file_8(pars);
                for(uint8_t pi = 0; pi < pars; pi++)
                {
                    f->write_stuff_to_file_8((uint8_t)(m->parameters[pi]->type));
                }
            }
        }

    }

}

void code_finalizer::finalize_locations()
{
    uint8_t type = 0x32;
    f->write_stuff_to_file_8(type, 0);  // the type of the file: 32 bit
    f->write_stuff_to_file_32(meta_location, 0 + 1); // the meta location for variables
    f->write_stuff_to_file_32(strtable_location, 0 + 1 + 4); // the stringtable location
    f->write_stuff_to_file_32(jumptable_location, 0 + 1 + 4 + 4); // the jumptable location
    f->write_stuff_to_file_32(funtable_location, 0 + 1 + 4 + 4 + 4); // the funtable location
    f->write_stuff_to_file_8(255, 0 + 1 + 4 + 4 + 4 + 4); /* max reg count, always 255*/
    f->write_stuff_to_file_8(0, 0 + 1 + 4 + 4 + 4 + 4 + 1); /* a dummy flag for now */

}

bool code_finalizer::is_asm_command_word(const std::string &expr)
{
    if(expr == "push") return true;
    if(expr == "intr") return true;
    if(expr == "call") return true;
    if(expr == "mov") return true;
    if(expr == "inc") return true;
    if(expr == "dec") return true;
    if(expr == "exit") return true;
    if(expr == "add") return true;
    if(expr == "sub") return true;
    if(expr == "mul") return true;
    if(expr == "div") return true;
    if(expr == "restore") return true;
    if(expr == "store") return true;
    if(expr == "mod") return true;
    if(expr == "shl") return true;
    if(expr == "shr") return true;
    if(expr == "and") return true;
    if(expr == "or") return true;
    if(expr == "xor") return true;
    if(expr == "not") return true;
    if(expr == "bcom") return true;
    if(expr == "neg") return true;
    if(expr == "pos") return true;
    if(expr == "eq") return true;
    if(expr == "lt") return true;
    if(expr == "gt") return true;
    if(expr == "lte") return true;
    if(expr == "gte") return true;
    if(expr == "neq") return true;
    if(expr == "serve") return true;
    if(expr == "jlbf") return true;
    if(expr == "jmp") return true;
    if(expr == "marksn") return true;
    if(expr == "clrsn") return true;
    if(expr == "return") return true;
    if(expr == "pop") return true;
    if(expr == "peek") return true;
    if(expr == "clidx") return true;
    if(expr == "leave") return true;
    if(expr == "pushall") return true;
    if(expr == "popall") return true;
    if(expr == "clbf") return true;
    if(expr == "poke") return true;

    return false;
}

code_finalizer *code_finalizer::instance()
{
    static code_finalizer ins;
    if(minstance == 0)
    {
        minstance = &ins;
    }
    return minstance;
}

void code_finalizer::destroy()
{
    minstance->mcompiler = 0;
    delete minstance->f;
    minstance->f = 0;
    minstance->assembly_commands.clear();
    minstance = 0;
}

void code_finalizer::finalize(nap_compiler *_compiler)
{
    mcompiler = _compiler;
    f = new file_abstraction(mcompiler);

    finalize_metatable();
    finalize_strtable();
    finalize_jumptable();
    finalize_funtable();
    finalize_locations();

    if(mcompiler->get_print_assembly())
    {
        for(size_t i=0; i<assembly_commands.size(); i++)
        {
            if(is_asm_command_word(assembly_commands[i]) || assembly_commands[i][0] == ':')
            {
                if(assembly_commands[i] != "peek")
                {
//                    std::cout << std::endl;
                }
            }
//            std::cout << assembly_commands[i] << " ";
        }
    }
}

void code_finalizer::add_assembly_command(const std::string &cmd)
{
    if(is_asm_command_word(cmd) || cmd[0] == ':')
    {
        if(cmd != "peek")
        {
            std::cout << std::endl;
        }
    }
    std::cout << cmd << " " << std::flush;

    assembly_commands.push_back(cmd);
}

