#ifndef _JUMP_H_
#define _JUMP_H_

struct nap_vm;

/**
 * Execute a jump operation
 * @param vm - the VM on which this runs
 */
int nap_jump(struct nap_vm* vm);

#endif
