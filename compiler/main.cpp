#include "call_ctx.h"
#include "compiler.h"
#include "garbage_bin.h"

#include "nap_types.h"
//void fun(nap_int_t a)
//{
//    printf("%d", a);
//}

//struct nap_ext_par_desc
//{
//    void* p1;
//    void* p2;
//    void* p3;
//    void* p4;
//    void* p5;
//    void *p6;
//    void *p7;
//};


////nap_ext_v__i
//typedef void (*nap_ext_v__i)(nap_int_t);
//void nap_ext_caller_v__i(void* fun, nap_ext_par_desc* pars)
//{
//    nap_ext_v__i local_fun = (nap_ext_v__i)fun;
//    local_fun( *((int*)(pars->p1)) );
//}



int main(int argc, char* argv[])
{
    const char* file_name = "test.nap";
    const char* bc_file_name = "test.ncb";

    bool print_asm = true;
    for(int i=0; i<argc; i++)
    {
        if(!strcmp(argv[i], "-a"))
        {
            print_asm = true;
        }
        if(!strcmp(argv[i], "-i"))
        {
            if(i + 1 < argc)
            {
                file_name = argv[i + 1];
            }
        }
        if(!strcmp(argv[i], "-o"))
        {
            if(i + 1 < argc)
            {
                bc_file_name = argv[i + 1];
            }
        }
    }
    std::auto_ptr<nap_compiler> c = nap_compiler::create_compiler();
    if(print_asm)
    {
        c->print_assemblies();
    }
    bool success = true;
    c->load_file(file_name, success);
    if(!success)
    {
        nap_compiler::release_compiler(c);
        garbage_bin_bin::shutdown();
        return 1;
    }

    c->compile();
    c->write_bytecode(bc_file_name);
    nap_compiler::release_compiler(c);

    garbage_bin_bin::shutdown();

    return 0;
}
