#include "number.h"
#include "type.h"
#include "is.h"
#include "consts.h"
#include "throw_error.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Returns the type of the given string as a  number... or at least tries to guess
 */
int number_get_type(const char* src)
{
        int i = 0;
        int len = strlen(src);
	if(!isnumber(src))
	{
		return 0;
	}
	while(i<len)
	{
		if(src[i] == '.')
		{
			return BASIC_TYPE_REAL;
		}
		i++;
	}

	return BASIC_TYPE_INT;
}

/**
 * Creates a new number from the given string
 */
 number* new_number_str(const char* src)
{
int type = number_get_type(src);
	if(BASIC_TYPE_INT == type)
	{
		return new_number_int(atol(src));
	}
	else
	if(BASIC_TYPE_REAL == type)
	{
		return new_number_real(atof(src));
	}

	return NULL;
}

/**
 * Creates a new integer
 */
number* new_number_int(long src)
{
number* nr = alloc_mem(number,1);
long* new_long = alloc_mem(long,1);
	*new_long = src;
	nr->type = BASIC_TYPE_INT;
	nr->location = new_long;
	return nr;
}

/**
 * Creates a new double
 */
number* new_number_real(double src)
{
number* nr = alloc_mem(number,1);
double* new_double = alloc_mem(double,1);
	*new_double = src;
	nr->type = BASIC_TYPE_REAL;
	nr->location = new_double;
	return nr;
}
