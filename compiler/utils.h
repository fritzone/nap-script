#ifndef _UTILS_H_
#define _UTILS_H_

#include <malloc.h>
#include <memory.h>
#include <string>

struct call_context;
struct variable;

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

/*
 * Utility functions
 */

/**
 * Transforms a character to a string
 * @param c the character to transform
 * Caller is responsible for deleting the result using `free`
 */
char *c2str(char c);

/**
 * Returns the substring of src before pos. The character at pos is not included.
 * @param pos the position
 * @param src the source string
 * Returns a new string, user must `free` it.
 */
char* before(int pos, const char *src);

/**
 * Returns the substring of src after pos. The character at pos is not included.
 * @param pos the position
 * @param src the source string
 * Returns a new string, user must `free` it.
 */
char* after(int pos, const char *src);

/**
 * Trims the trailing spaces, tabs, newlines from input, returns the trimmed string.
 * This method Modifies the input, returns the modified version.
 * This method does not allocate any memory.
 * @param src the string to trim
 */
char *rtrim(const char* src);

/**
 * Trims the leading spaces, tabs, newlines from input, returns the trimmed string.
 * This method creates a new string, modifies and returns it.
 * This method allocates memory, the user must free.
 * @param src the string to trim
 */
char *ltrim(const char* src);

/**
 * Trims the leading/trailing spaces, tabs, newlines from input, returns the trimmed string.
 * This method creates a new string, modifies and returns it.
 * This method allocates memory, the user must free.
 * @param src the string to trim
 */
char *trim(const char* src);

/**
 * Duplicates the given string, the user is responsible for deleting the result.
 * @param src the string to duplicate
 */
char* duplicate_string(const char* src);

/**
 * Prepares a string, allcoates and zeroes the memory for it
 * @param size the size ofthe string
 */
char* new_string(int size);

/**
 * Returns the maximum between the two integers
 * @param a - the first number
 * @param b - the second
 */
int max_int(int a, int b);

/**
 * Reverses the given string. Works only for malloced-strings since it reverses on place. Don't use for constants!
 * @param s - the string to reverse
 * @param len - the lenth of the string
 */
void reverse(char* s, int len);

/**
 * Extracts from s the seqction between from and to. Not including * from and * to
 */
char* extract(const char* from, const char* to);

/**
 * Skips the parantheses in the given expression, updates i
 * @param expr - the expression to work on
 * @param expr_len - the length of the expression
 * @param i - the index, points exactly to the position of the parantheses
 */
void skip_pars(const char* expr, int expr_len, int* i);
void skip_sq_pars(const char* expr, int expr_len, int* i);

/**
 * Skips the whitespace in the given string
 */
void skip_whitespace(const char* expr, int expr_len, int* i);

/**
 * Extracts a phrase from the input string which is enclosed between c_starter and c_ender, the result will
 * be places in o_result, and it will return the first character after the extracted pahrase and the enclosing
 * ender in the input
 */
char* extract_next_enclosed_phrase(char* input, char c_starter, char c_ender, char* o_result);

extern long mem_alloc_count;
extern void** mem_allocation;

#define alloc_mem(type,count) (type*)create(sizeof(type), count)

/**
 * Creates count objects of the given type
 */
void* create(int obj_size, int count);


char other_par(char c);

bool valid_variable_name(const char* nm);
/**
 * Returns the type of the operator as a struct number for faster access
 */
int get_comp_typeid(const char* op);

std::string fully_qualified_varname(call_context* cc, variable* v);

std::string fully_qualified_varname(call_context* cc, const char* v);

std::string fully_qualified_label(std::string& l);
std::string fully_qualified_label(const char*);

int irand(int min, int max);

std::string generate_unique_hash();

#endif
