#include "nbci.h"
#include <stdlib.h>
#include <string.h>
/*
 * Main entry point
 */
int main(int argc, char* argv[])
{
    struct nap_vm* vm = NULL;

    struct startup_configuration* config = calloc(1, sizeof(struct startup_configuration));

    /* bytecode file */
    const char* bc_file_name = "test.ncb";
    int dump = 0;
    int i = 0;

    /* create default configuration */
    config->stack_size = STACK_INIT;
    config->deepest_recursion = DEEPEST_RECURSION;

    for(i=0; i<argc; i++)
    {
        if(!strcmp(argv[i], "-d"))
        {
            dump = 1;
        }
        if(!strcmp(argv[i], "-i"))
        {
            if(i + 1 < argc)
            {
                bc_file_name = argv[i + 1];
            }
        }
        if(!strcmp(argv[i], "-s"))
        {
#ifdef PREFER_DYNAMIC_ALLOCATION
            if(i + 1 < argc)
            {
                config->stack_size = atoi(argv[i + 1]);
            }
#else
            fprintf(stderr, "[ERR] This VM was compiled with static memory allocation. Cannot specify dynamic stack size.\n"
                            "Recompile the VM with the \"vm-prefer-dynamic-allocation\" flag set to true.\n");
#endif
        }

        if(!strcmp(argv[i], "-r"))
        {
#ifdef PREFER_DYNAMIC_ALLOCATION
            if(i + 1 < argc)
            {
                config->deepest_recursion = atoi(argv[i + 1]);
            }
#else
            fprintf(stderr, "[ERR] This VM was compiled with static memory allocation. Cannot specify dynamic recursion size.\n"
                            "Recompile the VM with the \"vm-prefer-dynamic-allocation\" flag set to true.\n");
#endif
        }

    }

    vm = nap_vm_load(bc_file_name, config);
    if(!vm)
    {
        exit(1);
    }

    nap_vm_run(vm);
    if(dump)
    {
        nap_vm_dump(vm, stdout);
    }
    nap_vm_cleanup(vm);

    return 0;
}
