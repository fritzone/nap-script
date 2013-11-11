#ifndef _COMPARISON_H_
#define _COMPARISON_H_

struct nap_vm;

/**
 * Executes a comparison operation, and updates the LBF
 * @param vm - the VM on which this runs
 */
void nap_comparison(struct nap_vm* vm);

#endif
