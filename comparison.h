#ifndef _COMPARISON_H_
#define _COMPARISON_H_

#include "envelope.h"

/**
 * Returns 1 if this is a comparison
 */
int is_comparison( char* input);

/**
 * Returns the type of the operator as a struct number for faster access
 */
int get_comp_typeid(const char* op);

#endif
