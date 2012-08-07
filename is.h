#ifndef _IS_H_
#define _IS_H_

/*
 * This file contains the prototype of different functions that determine whether the passed in parameter is
 * something recognizeable.
 */


#define BT_IS_NUMERIC(bt) (BASIC_TYPE_INT == bt || BASIC_TYPE_REAL == bt)

#define BT_IS_STRING(bt) (BASIC_TYPE_STRING == bt)

/**
 * Checks if the given character is an operator or not
 */
int isoperator(char c);

int isparanthesis(char c);

int isnumber(const char *s);

int is_identifier_char(char c);

int is_phrase_delimiter(char c);

int is_string_delimiter(char c);

int is_whitespace(char c);

int is_valid_variable_name(const char* name);
#endif
