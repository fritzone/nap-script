#ifndef _UTILS_H_
#define _UTILS_H_

#include <memory.h>
#include <string>

#include "garbage_bin.h"

struct call_context;
struct variable;
class nap_compiler;

#define BT_IS_NUMERIC(bt) (BASIC_TYPE_INT == bt || BASIC_TYPE_REAL == bt)

#define BT_IS_STRING(bt) (BASIC_TYPE_STRING == bt)

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

// trim from start
static inline std::string &sltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &srtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &strim(std::string &s) {
        return sltrim(srtrim(s));
}

/**
 * Checks if the given character is an operator or not
 */
int isoperator(char c);

int isparanthesis(char c);

int isnumber(const std::string &s);

int is_identifier_char(char c);

int is_phrase_delimiter(char c);

int is_string_delimiter(char c);

int is_whitespace(char c);

int is_valid_variable_name(const char* name);

/* check for an immediate byte value expressed as character: 'A' -> 65*/
int is_immediate_byte(const std::string &t);

/*
 * Utility functions
 */

/**
 * Trims the leading/trailing spaces, tabs, newlines from input, returns the trimmed string.
 * This method creates a new string, modifies and returns it.
 * This method allocates memory, the user must free.
 * @param src the string to trim
 */
char *trim(const char* src, const nap_compiler *_compiler);

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
void skip_whitespace(const std::string &expr, int expr_len, int* i);

/**
 * Extracts a phrase from the input string which is enclosed between c_starter and c_ender, the result will
 * be places in o_result, and it will return the first character after the extracted pahrase and the enclosing
 * ender in the input
 */
char* extract_next_enclosed_phrase(const char *input, char c_starter, char c_ender, char* o_result);

extern long mem_alloc_count;
extern void** mem_allocation;

template <class T> T* allocate(size_t count, const nap_compiler* _compiler)
{
    T* tmp = new T[count];
    memset(tmp, 0, count * sizeof(T));
    garbage_bin<T*>::instance(_compiler).throwIn(tmp, _compiler);

    return tmp;
}

#define alloc_mem(type,count,compiler) allocate<type>(count, compiler)

char other_par(char c);

bool valid_variable_name(const std::string &nm);
/**
 * Returns the type of the operator as a struct number for faster access
 */
int get_comp_typeid(const char* op);

std::string fully_qualified_varname(call_context* cc, const variable* v);

std::string fully_qualified_varname(call_context* cc, const char* v);

std::string fully_qualified_label(std::string& l);
std::string fully_qualified_label(const char*);

int irand(int min, int max);

std::string generate_unique_hash();

/**
 * Creates a new string list from the instr which is separated by the given separator
 * @param instr - the input string
 * @param sep - the expected separator
 */
std::vector<std::string> string_list_create_bsep(const std::string &instr, char sep, const nap_compiler *_compiler, bool &psuccess);

int is_enclosed_string(const std::string &expr_trim, int expr_len, char encls, char encle);

/**
 * @brief is_string returns true if the string is enclosed in quotes
 * @param expr_trim
 * @param expr_len
 * @return
 */
int is_string(const std::string &expr_trim, int expr_len);

/**
 * @brief is_statement_string returns true if the strign is enclosed in bacquotes
 * @param expr_trim
 * @param expr_len
 * @return
 */
int is_statement_string(const std::string &expr_trim, int expr_len);

/**
 * Returns the type identifier for the given type
 */
int get_typeid(const std::string& type);


bool starts_with(const std::string& s1, const std::string& s2) ;

#endif
