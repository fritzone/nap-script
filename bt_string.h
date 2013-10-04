#ifndef _BT_STRING_H_
#define _BT_STRING_H_

#include "common_structs.h"

/**
 * Creates a new bt_string structure and populates it with the given string
 * @param src - the string to create
 */
bt_string* bt_string_create(const char* src);

/**
 * Checks whether the parameter expr_trim is something enclosed in quotes (meaning: a String)
 */
int is_string(const char* expr_trim, int expr_len);

/**
 * Checks if the given string is a statement string (ie: command, that will be executed, placed in ``)
 */
int is_statement_string(const char* expr_trim, int expr_len);

#endif
