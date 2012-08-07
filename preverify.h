#ifndef _PREVERIFY_H_
#define _PREVERIFY_H_

/**
 * 1. eliminates all space from the string, 
 * 2. puts (0-1)* instead of a - sign
 */
char *preverify(char *expr);

/**
 * Fixes the postfix increment/decrement
 * This transforms the string in the following way:
 * var++/-- will be transformed int (var++)
 */
char* preverify2(char* expr);

/**
 * Preverifies the pre-increment/decrement
 * It will transform the input as:
 * --x -> (--x)
 */
char* preverify3(char* expr);

/**
 * Prepares the expression to be in a much more interpreter friendly like format.
 */
char* prepare_expression(const char* expr);

/**
 * Validates the parantheses
 */
int validate_parantheses( char* expr);


#endif
