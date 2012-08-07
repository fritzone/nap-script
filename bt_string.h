#ifndef _BT_STRING_H_
#define _BT_STRING_H_

/**
 * The String Basic Type
 */
struct bt_string
{
	/* this is the actual string */
	char* the_string;

	/* the length of the string */
	int len;
};

/**
 * Creates a new bt_string structure and populates it with the given string
 * @param src - the string to create
 */
bt_string* bt_string_create(const char* src);

/**
 * Creates a new struct bt_string struct and populates it with the given strings
 */
bt_string* bt_string_create_empty();

/**
 * Checks whether the parameter expr_trim is something enclosed in quotes (meaning: a String)
 */
int is_string(const char* expr_trim, int expr_len);

/**
 * Inserts in the given bt_string object from the given idx position the string in what.
 * Reallocates the mmeory if necessarry
 */
void bt_string_insert(struct bt_string* str, int idx, char* what);

/**
 * Checks if the given string is a statement string (ie: command, that will be executed, placed in ``)
 */
int is_statement_string(const char* expr_trim, int expr_len);

#endif
