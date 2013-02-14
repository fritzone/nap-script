#include "is.h"
#include "consts.h"

#include <ctype.h>
#include <string.h>

/*
 * Returns 1 if c is an operator (+/-*)
 */
int isoperator(char c)
{
	return c == C_MOD || c== C_ADD || c== C_SUB || c== C_DIV || c== C_MUL || c=='.' ;
}


/*
 * returns 1 if c is a paranthesis
 */
int isparanthesis(char c)
{
	return c=='(' || c==')' || c=='[' || c==']';
}

/*
 * returns 1 if s is a  number
 */
int isnumber(const char *s)
{
    unsigned int i;
    unsigned l = strlen(s);
    if(l == 0)
	{
		return 0;
	}
    for(i=0; i<l; i++)
	{
		if(!isdigit(s[i]) && !(s[i]=='.') && !(s[i] == '-'))
		{
			return 0;
		}
	}
	if(!isdigit(s[0])) return 0;
	return 1;
}

int is_identifier_char(char c)
{
	return isalnum(c) || c == '_';
}

int is_phrase_delimiter(char c)
{
	return c==';' || c == '{' || c == '}';
}

int is_string_delimiter(char c)
{
	return c == C_QUOTE || c == C_BACKQUOTE;
}

int is_whitespace(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

int is_valid_variable_name(const char* name)
{
	if(strlen(name) == 0) return 0;
	while(*name)
	{
		if(!is_identifier_char(*name)) return 0;
		name ++;
	}
	return 1;
}
