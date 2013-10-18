#ifndef _OPR_HNDL_H_
#define _OPR_HNDL_H_

/*
 * This file contains everything that has anything in common with the operator handling
 */

/**
 * The function looks for two level 0 operators. It always returns the first found operator.
 * The function is used when looking for one character long operators, such as +, -, *, /
 * @param expr - the expression in which we are checking the operators
 * @param op1 - the first operator to look for
 * @param op2 - the second operator
 * @param needs_first - tells the method if we need the first (1) or the last (0) found operator
 */
int level_0_char_operator(const char *expr, char op1, char op2, int needs_first);

/**
 * This function looks for a level 0 operator which can be logner than a character, for example NotEqual.
 * @param expr - the expression in which we are searching the operator
 * @param op - the operator we are looking for
 * @param need_last - tells us if we need the last occurence (1) or the first occurence (default) of the op.
 */
int level_0_longer_operator(const char *expr, const char* op, int need_last);

/**
 * Looks for a level 0 bitwise operator (&, |, ~, !) in the given expression
 * Returns the index, if any found.
 * Returns -1 if nothing is found
 * @param expr - the expression we are working on
 */
int level_0_add_operator(const char* expr);

/**
 * Looks for a lvel 0 additive operator (+, -) in the given expression
 * Returns the index, if any found.
 * Returns -1 if nothing is found
 * @param expr - the expression we are working on
 */
int level_0_add_operator(const char* expr);

/** 
 * Looks for a level 0 multiplicative operator (*, /, %) in the given expression
 * Returns the index, if any found.
 * Returns -1 if nothing is found
 * @param expr - the expression we are working on
 */
int level_0_multiply_operator(const char *expr);

/**
 * Looks for a level 0 shifting operator.
 * Returns the index, if any found.
 * Returns -1 if nothing is found
 * @param expr - the expression we are working on
 */
int level_0_shift(const char* expr);

/**
 * Looks for a level 0 assignment (=) operator in the given expression
 * Returns the index, if any found.
 * Returns -1 if nothing is found
 * @param expr - the expression we are working on
 */
int level_0_assignment_operator(const char *expr);

/**
 * Looks for a level 0 dot (.) operator in the given expression
 * Returns the index, if any found.
 * Returns -1 if nothing is found
 * @param expr - the expression we are working on
 */
int level_0_dot_operator(const char *expr);

/**
 * Looks for a level 0 comparison operator.
 * Returns the index, if any found.
 * Returns -1 if nothing is found
 * @param expr - the expression we are working on
 * @param found_operator - will be populated with the operator that was retrieved, since this method can
 *        identify more than one kind of operators
 */
int level_0_comparison_operator(const char *expr, const char** found_operator);

/**
 * Looks for a level 0 bitwise (~, &, |, ^) operator.
 * Returns the index, if any found.
 * Returns -1 if nothing is found
 * @param expr - the expression we are working on
 */
int level_0_bitwise_operator(const char* expr);

/**
 * Looks for a level 0 logical (&&, ||, !) operator.
 * Returns the index, if any found and the found_operator is populated with the found op.
 * Returns -1 if nothing is found
 * @param expr - the expression we are working on
 */
int level_0_logical_operator(const char* expr);

/**
 * Finds the first occurence of a level 0 x= operator, such as +=, -=, *=, etc ...
 * Returns the index, if any found.
 * Returns -1 if nothing is found
 * @param expr - the expression we are working on
 * @param found_operator - will be populated with the operator that was retrieved, since this method can
 *        identify more than one kind of operators
 */
int level_0_sg_eq_operator(const char *expr, const char** found_operator, int* found_op_type);

#endif
