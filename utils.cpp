#include "consts.h"
#include "number.h"
#include "variable.h"
#include "type.h"
#include "utils.h"
#include "notimpl.h"
#include "throw_error.h"
#include "bt_string.h"

#include "sys_brkp.h"

#include <string.h>

#include "utils.h"
#include "is.h"
#include "consts.h"
#include "throw_error.h"
#include "call_ctx.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sstream>

/**
 * Transforms a character to a string
 */
char *c2str(char c)
{
char *s = alloc_mem(char,2);
	s[0] = c;
	s[1] = 0;
	return s;
}

/**
 * return the substring of src before pos
 */
char* before(int pos, const char *src)
{
char *befs = alloc_mem(char,pos+1);
	strncpy(befs, src, pos);
	return befs;
}

/**
 * return the substring of src after pos
 */
char* after(int pos, const char *src)
{
int len = strlen(src);
char *afts= alloc_mem(char,len - pos + 1);
	strncpy(afts, src + pos + 1, len - pos + 1);
	return afts;
}


/**
 * Trims the trailing spaces, tabs, newlines from input, returns the trimmed string
 */
char *rtrim(const char* src)
{
char *res = duplicate_string(src);
int endPos = strlen(res)-1;
	if(endPos == -1)
	{
        return EMPTY;
	}
	while(is_whitespace(res[endPos]))
	{
		res[endPos] = 0;
		endPos -- ;
	}
	return res;
}

/**
* Trims the leading spaces, tabs, newlines from input, returns the trimmed string. User must free the returned string
*/
char *ltrim(const char* src)
{
	if(!src)
	{
        return EMPTY;
	}

    int endPos = strlen(src);
    int i = 0;
	
	while(i<endPos && is_whitespace(src[i])) i++;
	
	return duplicate_string(src + i);
}

/**
 * Removes leading, trailing spaces
 */
char* trim(const char* src)
{
char* trimd1 = ltrim(src);
	return rtrim(trimd1);
}

/**
 * Duplicates src
 */
char* duplicate_string(const char* src)
{
	return strdup(src);
}

/**
 * Creates a new string
 */
char* new_string(int size)
{
char* tmp = alloc_mem(char, size + 1);
	return tmp;
}

/**
 * Returns the maximum between the two numbers
 */
int max_int(int a, int b)
{
	return a>b?a:b;
}

/**
 * Reverses the string
 */
void reverse(char* s, int len)
{
	if(!s || len<1) return;
	while(len > 1)
	{
		*s ^= *(s + len - 1);
		*(s + len - 1) ^= *s;
		*s ^= *(s + len - 1);
		len -= 2;
		s ++;
	}
}

char* extract(const char* from, const char* to)
{
    char* res = new_string(to - from + 1);
	strncpy(res, from + 1, to-from-1);
	return res;
}

void skip_pars(const char* expr, int expr_len, int* i)
{
	(*i)++;	/* */
int can_stop = 0;
int level = 1;
	while(!can_stop && *i < expr_len)
	{
		if(expr[*i] == C_PAR_OP) level ++;
		if(expr[*i] == C_PAR_CL) level --;
		if(level == 0) can_stop = 1;
		if(!can_stop) (*i) ++;
	}
	(*i)++;
}

void skip_sq_pars(const char* expr, int expr_len, int* i)
{
	(*i)++;	/* */
	int can_stop = 0;
	int level = 1;
	while(!can_stop && *i < expr_len)
	{
		if(expr[*i] == C_SQPAR_OP) level ++;
		if(expr[*i] == C_SQPAR_CL) level --;
		if(level == 0) can_stop = 1;
		if(!can_stop) (*i) ++;
	}
	(*i)++;
}


void skip_whitespace(const char* expr, int expr_len, int* i)
{
	while(*i < expr_len && is_whitespace(expr[*i])) (*i) ++;
}

char* extract_next_enclosed_phrase(char* input, char c_starter, char c_ender, char* o_result)
{
int level = 0;
char* p = input, *result = o_result;
int can_stop = 0;
	while(*p && !can_stop)
	{
		if(*p == c_starter) level ++;
		if(*p == c_ender) level --;
		if(level == -1) can_stop = 1;
		if(*p == C_QUOTE || *p == C_BACKQUOTE || *p == C_SQUOTE)
		{
		char m_ender = *p;
			*result ++ = *p ++;
			while(*p && *p != m_ender)
			{
				*result ++ = *p ++;
			}
		}
		if(!can_stop) *result ++ = *p ++;
	}
	p++;
	return p;
}


long mem_alloc_count = 0;

int mem_loc_compare(const void* m1, const void* m2)
{
	return  (long)m2 - (long)m1;
}

void** mem_liberated = NULL;
long liberc = 0;

void* create(int obj_size, int count)
{
    void* tmp = calloc(count , obj_size);
	return tmp;
}

void* reallocate(void* addr, int new_size)
{
		void* tmp =  realloc(addr, new_size);
	return tmp;
}

char other_par(char c)
{
	if(c == '(') return ')';
	if(c == '[') return ']';
	if(c == '{') return '}';

	return c;
}

bool valid_variable_name(const char* nm)
{
    int l = strlen(nm);
	if(l < 1 || l > 255)
    {
        printf("variable name too long: %s [%d]\n", nm, l);
        return false;
    }
	if(!isalpha(nm[0]) && nm[0] !='$') 
    {
        printf("variable name does not start with allowed symbol %s [%c]\n", nm, nm[0]);
        return false;
    }
	if(nm[0] == '$')
    {
        return true;
    }
    
	for(int i=(nm[0]=='$'?1:0); i<l ;i++)
	{
		if(!isalnum(nm[i]) && nm[i] !='_')
        {
            printf("variable contains invalid character: %s [%c] at position [%d]\n", nm, nm[i], i);
            return false;
        }
	}
	return true;
}


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


std::string fully_qualified_varname(call_context* cc, variable* v)
{
    if(v->cc)
    {
        return std::string(v->cc->name) + STR_DOT + v->name;
    }
    else
    {
        return std::string(cc->name) + STR_DOT + v->name;
    }
}
std::string fully_qualified_varname(call_context* cc, const char* v)
{
    return std::string(cc->name) + STR_DOT + v;
}

std::string fully_qualified_label(const char* l)
{
    std::stringstream ss;
    ss << STR_COLON << l << STR_COLON;
    std::string s = ss.str();
    return s;
}

std::string fully_qualified_label(std::string& l)
{
    std::stringstream ss;
    ss << STR_COLON << l << STR_COLON;
    std::string s = ss.str();
    return s;
}
