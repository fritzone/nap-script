#ifndef _BSD_NUMB_H_
#define _BSD_NUMB_H_

#include "type.h"
#include "typedefs.h"

/**
 * Represents a struct number.
 */
struct number
{
	/* type of the struct number */
	m_typeid type;

	/* the location of the number */
	void* location;
};

#endif

