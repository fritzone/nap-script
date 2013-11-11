#include "nap_runtime.h"

#include <stdlib.h>
#include <stdio.h>

struct nap_runtime
{
};

struct nap_bytecode_chunk
{
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

    return result;
}

struct nap_bytecode_chunk* nap_runtime_compile(struct nap_runtime* /*runtime*/,
                                               const char* /*commands*/)
{
    return 0;
}


int nap_runtime_execute(struct nap_runtime* /*runtime*/,
                        struct nap_bytecode_chunk* /*bytecode*/)
{
    return 0;
}


nap_number_t nap_runtime_get_int(struct nap_runtime* /*runtime*/,
                                 const char* /*variable_name*/)
{
    return 0;
}

