#ifndef _NBCI_IMPL_H_
#define _NBCI_IMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "nap_types.h"

struct nap_vm;

/**
 * Returns 1 if the vm has a variable with the given name and populate the
 * *type with the type of the variable. *type will be -1 if no variable
 * is found and zero is returned in this case
 **/
int nap_vmi_has_variable(const struct nap_vm *vm, const char* name, int *type);


/**
 * Fetches the address at the current location in the bytecode stream
 * @param vm
 * @return
 */
nap_addr_t nap_fetch_address(struct nap_vm *vm);

/**
 * Fetch a marker from the bytecode stream
 * @param vm
 * @return
 */
nap_mark_t nap_fetch_mark(struct nap_vm* vm);

/**
 * Fetch an index from the bytecode stream
 * @param vm
 * @return
 */
nap_index_t nap_fetch_index(struct nap_vm* vm);

/**
 * Read an immediate value from the bytecode stream and return it
 * @param vm
 * @return
 */
nap_int_t nap_read_immediate(struct nap_vm* vm);

/**
 * Saves the registers. Happens automatically on a "call"
 */
void nap_save_registers(struct nap_vm*);

/**
 * Restores the registers. Happens automatically on a "leave"
 */
void nap_restore_registers(struct nap_vm*);

/**
 * Handles the interrupt which is next in the "content" of the VM
 */
void nap_handle_interrupt(struct nap_vm*);

#ifdef __cplusplus
}
#endif

#endif

