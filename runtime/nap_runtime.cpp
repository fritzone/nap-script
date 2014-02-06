#include "nap_runtime.h"

extern "C"
{
#include "nbci.h"
#include "nap_structs.h"
}

#include "garbage_bin.h"
#include "compiler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* The nap runtime environment. There can be multiple runtimes */
struct nap_runtime
{
    nap_runtime() : compiler(nap_compiler::create_compiler()), vm(0), chunks(),
        last_error("[NAP_OK] no error")
    {}

    // the compiler of the runtime
    std::auto_ptr<nap_compiler> compiler;

    // the virtual machine
    nap_vm* vm;

    // the list of chunks that were compiled
    std::vector<nap_bytecode_chunk*> chunks;

    // the last error
    std::string last_error;
};

NAP_LIB_API nap_runtime* nap_runtime_create(const char* /*name*/)
{
    struct nap_runtime* result = new nap_runtime;

    return result;
}

NAP_LIB_API nap_bytecode_chunk* nap_runtime_compile(struct nap_runtime* runtime,
                                                    const char* commands,
                                                    const char *name)
{
    if(runtime != NULL)
    {
        bool success = true;
        bool source_set = runtime->compiler->set_source(commands, success);

        if(source_set)
        {
            if(runtime->compiler->compile())
            {
                nap_bytecode_chunk* chunk = (struct nap_bytecode_chunk*)
                                       calloc(sizeof(struct nap_bytecode_chunk), 1);
                runtime->compiler->deliver_bytecode(chunk->code, chunk->length);
                runtime->chunks.push_back(chunk);
                if(name != NULL)
                {
                    size_t len = strlen(name);
                    chunk->name = (char*)(calloc(len, sizeof(char)));
                    strncpy(chunk->name, name, len);
                }
                return chunk;
            }
            else
            {
                runtime->last_error = runtime->compiler->get_error();
                return NULL;
            }
        }
        else
        {
            runtime->last_error = runtime->compiler->get_error();
            return NULL;
        }
    }
    return NULL;
}

NAP_LIB_API int nap_runtime_execute(struct nap_runtime* runtime,
                        struct nap_bytecode_chunk* bytecode)
{
    runtime->vm = nap_vm_inject(bytecode->code, bytecode->length, EMBEDDED);
    nap_vm_run(runtime->vm);

    return 1;
}

NAP_LIB_API nap_byte_t nap_runtime_get_byte(struct nap_runtime* runtime,
                                 const char* variable_name, int *found)
{
    char* t = (char*)calloc(strlen(variable_name) + 1, 1);
    strcpy(t, variable_name);
    nap_byte_t temp = nap_vm_get_byte(runtime->vm, t, found);
    free(t);
    return temp;
}

NAP_LIB_API nap_int_t nap_runtime_get_int(struct nap_runtime* runtime,
                                 const char* variable_name, int *found)
{
    char* t = (char*)calloc(strlen(variable_name) + 1, 1);
    strcpy(t, variable_name);
    nap_int_t temp = nap_vm_get_int(runtime->vm, t, found);
    free(t);
    return temp;
}



NAP_LIB_API nap_real_t nap_runtime_get_real(nap_runtime * /*runtime*/,
                                const char * /*variable_name*/)
{
    return 0;
}


NAP_LIB_API nap_string_t nap_runtime_get_string(nap_runtime* runtime,
                                    const char* variable_name, int *found)
{
    char* t = (char*)calloc(strlen(variable_name) + 1, 1);
    strcpy(t, variable_name);
    char* temp = nap_vm_get_string(runtime->vm, t, found);
    free(t);
    return temp;
}


NAP_LIB_API void nap_runtime_shutdown(nap_runtime **runtime)
{
    nap_compiler::release_compiler((*runtime)->compiler);
    if((*runtime)->vm)
    {
        nap_vm_cleanup((*runtime)->vm);
    }
    for(size_t i=0; i<(*runtime)->chunks.size(); i++)
    {
        free( (*runtime)->chunks.at(i)->code);
        if((*runtime)->chunks.at(i)->name)
        {
            free((*runtime)->chunks.at(i)->name);
        }

        free( (*runtime)->chunks.at(i));
    }
    delete (*runtime);
    *runtime = 0;
    garbage_bin_bin::shutdown();
}
