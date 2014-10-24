#ifndef _MOV_H_
#define _MOV_H_

#include "nbci.h"

/**
 * Execute a MOV operation on the given virtual machine
 * @param vm - the VM on which this MOV is executed
 */
int nap_mov(struct nap_vm* vm);

int mov_into_register(struct nap_vm* vm);
int mov_into_variable(struct nap_vm* vm);
int mov_into_indexed(struct nap_vm* vm);
int mov_into_peek_target(struct nap_vm* vm);

#endif
