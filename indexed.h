#ifndef _INDEXED_H_
#define _INDEXED_H_

#include "common_structs.h"

#define INDEXED_GET_INDEX(idxd) (idxd->idx)

#define INDEXED_GET_ENV(idxd) (idxd->base)

/**
 * Creates a new struct indexed object for the struct variable (which must have been declared as being
 * with a dimension > 1) and the specified index.
 */
struct indexed* new_indexed(struct envelope* base_variable, int index_value);

#endif
