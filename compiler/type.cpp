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
