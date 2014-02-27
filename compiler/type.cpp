#include <string.h>

#include "type.h"
#include "consts.h"
#include "number.h"

/**
 * Returns the type ID of the given text type...
 */
int get_typeid(const std::string &type)
{
    if(type == STR_INT) return BASIC_TYPE_INT;
    if(type == STR_BYTE) return BASIC_TYPE_BYTE;
    if(type == STR_REAL) return BASIC_TYPE_REAL;
    if(type == STR_CHAR) return BASIC_TYPE_CHAR;
    if(type == STR_BOOL) return BASIC_TYPE_BOOL;
    if(type == STR_STRING) return BASIC_TYPE_STRING;
    if(type == STR_VOID) return BASIC_TYPE_VOID;

    return BASIC_TYPE_DONTCARE;
}
