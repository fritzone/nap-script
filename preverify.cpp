#include "preverify.h"
#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

/**
 * This prepares the final expression that will be sent to the interpreter. This works on a one line expression.
 */
char* prepare_expression(const char* expr)
{
char* tmp = duplicate_string(expr);
	tmp = rtrim(tmp);
	return tmp;
}

