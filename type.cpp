#include <string.h>

#include "type.h"
#include "consts.h"
#include "number.h"

/**
 * Returns the type ID of the given text type...
 */
int get_typeid(const char* type)
{
	if(!strcmp(type, STR_INT)) return BASIC_TYPE_INT;
	if(!strcmp(type, STR_REAL)) return BASIC_TYPE_REAL;
    if(!strcmp(type, STR_CHAR)) return BASIC_TYPE_CHAR;
    if(!strcmp(type, STR_BOOL)) return BASIC_TYPE_BOOL;
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
    case BASIC_TYPE_BOOL:   return STR_BOOL;
    case BASIC_TYPE_CHAR:   return STR_CHAR;
    
	default: return STR_UNKNOWN;
	}
}
