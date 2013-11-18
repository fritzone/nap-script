#include "nap_runtime.h"
extern "C"
{
#include "nbci.h"
}

#include "compiler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct nap_runtime
{
    nap_compiler* compiler;
    nap_vm* vm;
};

struct nap_bytecode_chunk
{
    uint8_t* code;
    size_t length;
};

struct nap_runtime* nap_runtime_create(const char* /*name*/)
{
    struct nap_runtime* result = (struct nap_runtime*)
                                      calloc(sizeof(struct nap_runtime), 1);
    if(result == NULL)
    {
        fprintf(stderr, "cannot create a new runtime\n");
        return result;
    }

    result->compiler = new nap_compiler();

    return result;
}

struct nap_bytecode_chunk* nap_runtime_compile(struct nap_runtime* runtime,
                                               const char* commands)
{
    if(runtime != NULL)
    {
        bool source_set = runtime->compiler->set_source(commands);
        if(source_set)
        {
            runtime->compiler->compile();
            nap_bytecode_chunk* chunk = (struct nap_bytecode_chunk*)
                                   calloc(sizeof(struct nap_bytecode_chunk), 1);
            runtime->compiler->deliver_bytecode(chunk->code, chunk->length);
            return chunk;
        }
        else
        {
            fprintf(stderr, "cannot compile the given source\n");
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
    return nap_vm_get_int(runtime->vm, t, &found);
}



nap_real_t nap_runtime_get_real(nap_runtime */*runtime*/,
                                const char */*variable_name*/)
{
    return 0;
}


nap_string_t nap_runtime_get_string(nap_runtime */*runtime*/,
                                    const char */*variable_name*/)
{
    return 0;
}


void nap_runtime_shutdown(nap_runtime **runtime)
{
    delete (*runtime)->compiler;
    nap_vm_cleanup((*runtime)->vm);
    *runtime = 0;
}
