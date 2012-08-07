#include "indexed.h"
#include "utils.h"

/**
 * Create a new indexed object for the given variable
 */
indexed* new_indexed(envelope* base_variable, int index_value)
{
indexed* tmp = alloc_mem(indexed,1);
	tmp->base = base_variable;
	tmp->idx = index_value;
	return tmp;
}
