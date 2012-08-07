#include "res_wrds.h"

#include "utils.h"

/**
 * Creates a new resw_if structure
 */
resw_if* new_resw_if()
{
	return alloc_mem(resw_if,1);
}

/**
 * Creates a new resw_while structure
 */
resw_while* new_resw_while()
{
	return alloc_mem(resw_while,1);
}