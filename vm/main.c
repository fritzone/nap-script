#include "nbci.h"
#include <stdlib.h>
#include <string.h>
/*
 * Main entry point
 */
int main(int argc, char* argv[])
{
    struct nap_vm* vm = NULL;
    const char* bc_file_name = "test.ncb";
    int dump = 0;
    int i = 0;
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
    }

    vm = nap_vm_load(bc_file_name);
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
