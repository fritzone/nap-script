#include "envelope.h"
#include "number.h"
#include "type.h"
#include "consts.h"
#include "utils.h"
#include "bt_string.h"

#include <stdlib.h>
#include <string.h>

envelope* new_envelope(void* data, int env_type)
{
 envelope* tmp = alloc_mem(envelope,1);
	tmp->to_interpret = data;
	tmp->type = env_type;
	return tmp;
}

/**
 * Returns the type of the  envelope that will be created based on the
 */
int envelope_get_type_from_vartype( char* src)
{
	if(!strcmp(src, STR_INT)) return BASIC_TYPE_INT;
	if(!strcmp(src, STR_REAL)) return BASIC_TYPE_REAL;
	if(!strcmp(src, "string")) return BASIC_TYPE_STRING;

	/* objects later */

	return BASIC_TYPE_DONTCARE;
}
