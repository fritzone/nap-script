#ifndef _BSD_INDX_H_
#define _BSD_INDX_H_

#include "bsd_enve.h"
#include "tree.h"

/**
 * Contains the definition of an struct indexed value
 */
struct indexed
{
	/* the actual index when the index is one-dimensional */
	int idx;

	/* and what we are indexing. This usually is a struct variable */
	struct envelope* base;
};

#endif
