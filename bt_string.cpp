#include "bt_string.h"
#include "consts.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Creates a new string from the src
 */
bt_string* bt_string_create(const char* src)
{
int len = strlen(src);
bt_string* nw_str = alloc_mem(bt_string,1);	
char *tmp = alloc_mem(char,len + 1);
    for(int i=0, j=0; i<len; i++)
	{
		if(src[i] != '\\') tmp[j++] = src[i];
		else
		{
			if(i<len-1)
			{
				tmp[j++] = src[++i];
			}
		}
	}
int len2 = strlen(tmp);
	nw_str->the_string = alloc_mem(char,len2 + 1);	
	memset(nw_str->the_string, 0, len2 + 1);
	strncpy(nw_str->the_string, tmp, len2);
	/* now prepare the escaped characters such as \" and \\ */
	nw_str->len = len2;
	return nw_str;
}


/**
 * Creates a new empty string
 */
bt_string* bt_string_create_empty()
{
	return bt_string_create("");
}

/**
 * Returns 1 if the string is enclosed in encls and encle
 */
static int is_enclosed_string(const char* expr_trim, int expr_len, char encls, char encle)
{
	if(expr_trim[0] == encls)	/* we can check them ... */
	{
    int i=1;
		while(i<expr_len)
		{
			if(expr_trim[i] == encle && expr_trim[i-1] != '\\' && i < expr_len - 1)
			{
				return 0; /* meaning: enclosing character found before end of expression */
			}
			i++;
		}

		return 1;
	}
	else
	{
		return 0;
	}

}

/**
 * Checks whether the parameter expr_trim is something enclosed in quotes (meaning: a String)
 */
int is_string(const char* expr_trim, int expr_len)
{
	return is_enclosed_string(expr_trim, expr_len, C_QUOTE, C_QUOTE);
}

int is_statement_string(const char* expr_trim, int expr_len)
{
	return is_enclosed_string(expr_trim, expr_len, C_BACKQUOTE, C_BACKQUOTE);
}

/**
 * Inserts in the given bt_string object from the given idx position the string in what.
 * Reallocates the mmeory if necessarry. Overwrites the content.
 */
void bt_string_insert( bt_string* str, int idx, char* what)
{
int whlen = strlen(what);
int new_length = idx + whlen;
	if(new_length > str->len)
	{
	char* new_str = new_string(new_length);
		strncpy(new_str, str->the_string, str->len);
		/* MEM: Here don't forget to sentence the current value of str->the_string */
		str->the_string = new_str;
	}
	strncpy(str->the_string + idx, what, whlen);
}
