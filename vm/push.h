#ifndef _PUSH_H_
#define _PUSH_H_

#include "nbci.h"

/**
 * Execute a push operation on the given virtual machine, update
 * the internal structures and advance the instruction pointer
 * @param vm - the VM on which this operation runs
 */
void nap_push(struct nap_vm* vm);

#endif
