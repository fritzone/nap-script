#ifndef _JUMP_H_
#define _JUMP_H_

struct nap_vm;

/**
 * Execute a jump operation
 * @param vm - the VM on which this runs
 */
void nap_jump(struct nap_vm* vm);

#endif
