#ifndef _THROW_ERROR_H_
#define _THROW_ERROR_H_

#include "parser.h"

/*
 * Header to 'throw' an error from the application. Basically exits the application
 */

/**
 * Throws the given error and exits to the system.
 */
void throw_error(const char* error);

void throw_error(const char* error, const char* par);

void throw_error(const char* error, int id, const char* par);

void throw_index_out_of_range(const char* variable_name, int maximum_allowed, int got);

void throw_error(const char* error, const char* par1, const char* par2);

void set_location(const expression_with_location* loc);

extern const expression_with_location* g_location;

#endif
