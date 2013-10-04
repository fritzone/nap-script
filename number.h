#ifndef _NUMBER_H_
#define _NUMBER_H_

#include "common_structs.h"

/**
 * Returns the type of the struct number represented as string
 */
int number_get_type(const char* src);

/**
 * Creates a new struct number from the string
 */
struct number* new_number_str(const char* src);

/**
 * Creates a new struct number from the src
 */
struct number* new_number_int(long src);

/**
 * Creates a new struct number from the src
 */
struct number* new_number_real(double src);

/**
 * Renders the given number to a string
 */
char* number_render_to_string(const number* val);

#endif
