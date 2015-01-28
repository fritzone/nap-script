#include "crea.h"
#include "nbci.h"
#include "metatbl.h"
#include "nbci_impl.h"
#include "nap_consts.h"
#include "funtable.h"
#include "opcodes.h"

#include <stdlib.h>
#include <string.h>


int nap_crea(struct nap_vm *vm)
{
    nap_addr_t class_index = nap_fetch_index(vm);

    return NAP_SUCCESS;
}
