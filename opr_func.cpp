#include "opr_func.h"
#include "utils.h"

#include <string.h>

/*
 * this checks if an expression is a function or not (ex: sin(x) is s function)
 */
 method* is_function_call(char *s,  call_context* cc)
{
    unsigned int i=0;
    int sc2=0;
    char *s2=new_string(strlen(s));
    method* get_method = NULL;

	while(i<strlen(s) && s[i]!='(')
	{
		s2[sc2++]=s[i++];
	}
	s2[sc2]=0;

	s2 = trim(s2);
	get_method = call_context_get_method(cc, s2);
	if(get_method) return get_method;

	return NULL;
}
