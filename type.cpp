#include <string.h>

#include "type.h"
#include "consts.h"
#include "number.h"

/**
 * Returns the size of the default types, later it will include the sizes of different user objects
 * The following basic types are allowed right now:
 * 1. int translates to 'long'
 * 2. real translates to 'double'
 * In case of user defined types the sum of the members will form the type
 */
int get_size(const char* type)
{
	if(!strcmp(type, STR_INT)) return sizeof(long);
	if(!strcmp(type, STR_REAL)) return sizeof(double);

	return 0;
}

/**
 * Returns the type ID of the given text type...
 */
int get_typeid(const char* type)
{
	if(!strcmp(type, STR_INT)) return BASIC_TYPE_INT;
	if(!strcmp(type, STR_REAL)) return BASIC_TYPE_REAL;
	if(!strcmp(type, STR_STRING)) return BASIC_TYPE_STRING;
	if(!strcmp(type,"void")) return BASIC_TYPE_VOID;

	return BASIC_TYPE_DONTCARE;
}

/**
 * Returns the string representation of the given type_id
 */
const char* get_desc(int type_id)
{
	switch(type_id) 
	{
	case BASIC_TYPE_INT:	return STR_INT;
	case BASIC_TYPE_REAL:	return STR_REAL;
	case BASIC_TYPE_STRING:	return STR_STRING;
	default: return STR_UNKNOWN;
	}
}
