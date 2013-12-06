#include "nap_runtime.h"

extern "C"
{
#include "nbci.h"
}

#include "garbage_bin.h"
#include "compiler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* The bytecode chunk which is created as a result of the compile command */
struct nap_bytecode_chunk
{
    /* the bytecode */
    uint8_t* code;

    /* the length of the bytecode */
    size_t length;
};

/* The nap runtime environment. There can be multiple runtimes */
struct nap_runtime
{
    std::auto_ptr<nap_compiler> compiler;
    nap_vm* vm;
    std::vector<nap_bytecode_chunk*>* chunks;
};

nap_runtime* nap_runtime_create(const char* /*name*/)
{
    struct nap_runtime* result = (struct nap_runtime*)
                                      calloc(sizeof(struct nap_runtime), 1);
    if(result == NULL)
    {
        fprintf(stderr, "cannot create a new runtime\n");
        return result;
    }

    result->compiler = nap_compiler::create_compiler();
    result->chunks = new std::vector<nap_bytecode_chunk*>();

    return result;
}

nap_bytecode_chunk* nap_runtime_compile(struct nap_runtime* runtime,
                                               const char* commands)
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
                runtime->chunks->push_back(chunk);
                return chunk;
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            fprintf(stderr, "cannot compile the given source: %s\n", runtime->compiler->get_error().c_str());
            return NULL;
        }
    }
    return NULL;
}


int nap_runtime_execute(struct nap_runtime* runtime,
                        struct nap_bytecode_chunk* bytecode)
{
    runtime->vm = nap_vm_inject(bytecode->code, bytecode->length);
    nap_vm_run(runtime->vm);

    return 1;
}


nap_int_t nap_runtime_get_int(struct nap_runtime* runtime,
                                 const char* variable_name)
{
    int found = 1;
    char* t = (char*)calloc(strlen(variable_name) + 1, 1);
    strcpy(t, variable_name);
    nap_int_t temp = nap_vm_get_int(runtime->vm, t, &found);
    free(t);
    return temp;
}



nap_real_t nap_runtime_get_real(nap_runtime * /*runtime*/,
                                const char * /*variable_name*/)
{
    return 0;
}


nap_string_t nap_runtime_get_string(nap_runtime* /*runtime*/,
                                    const char* /*variable_name*/)
{
    return 0;
}


void nap_runtime_shutdown(nap_runtime **runtime)
{
    nap_compiler::release_compiler((*runtime)->compiler);
    if((*runtime)->vm)
    {
        nap_vm_cleanup((*runtime)->vm);
    }
    for(size_t i=0; i<(*runtime)->chunks->size(); i++)
    {
        free( (*runtime)->chunks->at(i)->code);
        free( (*runtime)->chunks->at(i));
    }
    delete( (*runtime)->chunks);
    free(*runtime);
    *runtime = 0;
    garbage_bin_bin::shutdown();
}
