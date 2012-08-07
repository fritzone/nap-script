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
 * Returns the int value of the given number
 */
long number_get_int_value(const number*  val)
{
	return *(long*)val->location;
}

/**
 * Returns the double value of the given number
 */
double number_get_real_value(const number*  val)
{
	return *(double*)val->location;
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

/**
 * Renders this number to a string
 */
char* number_render_to_string(const number* val)
{
char* res = new_string(MAX_NR_AS_STR_LEN);
	if(BASIC_TYPE_INT == val->type)
	{
	long lv = *(long*)val->location;
        sprintf(res, "%ld", lv);
	}
	else
	if(BASIC_TYPE_REAL == val->type)
	{
	double rv = *(double*)val->location;
		sprintf(res, STR_REAL_FORMAT, rv);
	}
	return res;
}

