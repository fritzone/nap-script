#include "comparison.h"
#include "consts.h"
#include "number.h"
#include "variable.h"
#include "indexed.h"
#include "type.h"
#include "utils.h"
#include "notimpl.h"
#include "throw_error.h"
#include "bt_string.h"

#include "typedefs.h"

#include "sys_brkp.h"

#include <string.h>

/**
* Returns the comparison function for the given input comparison string
*/
int get_comp_typeid(const char* input)
{
	if(!strcmp(input, STR_EQUALEQUAL)) return COMP_EQUALEQUAL;
	if(!strcmp(input, STR_LT)) return COMP_LT;
	if(!strcmp(input, STR_GT)) return COMP_GT;
	if(!strcmp(input, STR_LTE)) return COMP_LTE;
	if(!strcmp(input, STR_GTE)) return COMP_GTE;
	if(!strcmp(input, STR_NEQ)) return COMP_NEQ;

	return NO_OPERATOR;
}

