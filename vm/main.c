#include "nbci.h"
#include <stdlib.h>

/*
 * Main entry point
 */
int main()
{
    struct nap_vm* vm = nap_vm_load("test.ncb");
    if(!vm)
    {
        exit(1);
    }

    vm->environment = STANDALONE;
    nap_vm_run(vm);
    nap_vm_cleanup(vm);

    return 0;
}
