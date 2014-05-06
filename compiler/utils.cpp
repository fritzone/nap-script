#include "consts.h"
#include "number.h"
#include "variable.h"
#include "type.h"
#include "utils.h"
#include "bt_string.h"
#include "garbage_bin.h"
#include "sys_brkp.h"
#include "compiler.h"

#include <string.h>

#include "utils.h"
#include "consts.h"
#include "call_ctx.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <set>

/**
 * Removes leading, trailing spaces
 */
char* trim(const char* src, const nap_compiler *_compiler)
{
    const char *start = src;
    while(*start && is_whitespace(*start)) start ++;
    if(!*start)
    {
        return (char*)"";
    }
    const char* end = src + strlen(src)- 1;
    while(end > src && is_whitespace(*end)) end --;
    char* trimd2 = alloc_mem(char, end + 1 - start + 1, _compiler);
    strncpy(trimd2, start, end - start + 1);
    return trimd2;
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

void skip_pars(const char* expr, int expr_len, int* i)
{
    (*i)++;    /* */
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
    (*i)++;    /* */
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


void skip_whitespace(const std::string& expr, int expr_len, int* i)
{
    while(*i < expr_len && is_whitespace(expr[*i])) (*i) ++;
}

char* extract_next_enclosed_phrase(const char* input, char c_starter, char c_ender, char* o_result)
{
    int level = 0;
    const char* p = input;
    char *result = o_result;
    int can_stop = 0;
    while(*p && !can_stop)
    {
        if(*p == c_starter) level ++;
        if(*p == c_ender) level --;
        if(level == -1) can_stop = 1;
        if(*p == C_QUOTE || *p == C_BACKQUOTE || *p == C_SQUOTE)
        {
            char m_ender = *p;
            *result ++ = *p;
            p ++;
            while(*p && *p != m_ender)
            {
                *result ++ = *p;
                p++;
            }
        }
        if(!can_stop) *result ++ = *p ++;
    }
    p++;
    return const_cast<char*>(p);
}

char other_par(char c)
{
    if(c == '(') return ')';
    if(c == '[') return ']';
    if(c == '{') return '}';

    return c;
}

bool valid_variable_name(const std::string& nm)
{
    int l = nm.length();
    if(l < 1 || l > 255)
    {
        return false;
    }
    if(!isalpha(nm[0]) && nm[0] !='$') 
    {
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


std::string fully_qualified_varname(call_context* cc, const variable* v)
{
    std::string result;
    if(v->cc)
    {
        result = std::string(v->cc->name) + STR_DOT + v->name;
    }
    else
    {
        result = std::string(cc->name) + STR_DOT + v->name;
    }

    if(v->c_type == "extern")
    {
        result = "extern." + v->name;
    }
    return result;
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

int irand(int min, int max)
{
    return (int)(((double)rand() / ((double)RAND_MAX + 1.0)) * (max - min + 1) + min);
}

static std::string generate_hash_tag()
{
    char str[5];
    for (int i = 0; i < 4; ++i)
    str[i] = (char)irand('a', 'z');
    str[4] = '\0';
    return std::string(str);
}


std::string generate_unique_hash()
{
    static std::set<std::string> hashes;

    std::string hash = generate_hash_tag();
    while(hashes.count(hash))
    {
        hash = generate_hash_tag();
    }
    hashes.insert(hash);
    return hash;
}

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
int isnumber(const std::string& s)
{
    unsigned int i;
    for(i=0; s[i]; i++)
    {
        if(!isdigit(s[i]) && !(s[i]=='.') && !(s[i] == '-') && !(s[i] == '+'))
        {
            return 0;
        }
    }
    if(s[0] && (!isdigit(s[0]) && s[0] != '-' && s[0] != '+'))
    {
        return 0;
    }
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

inline int is_whitespace(char c)
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

/**
 * Creates a new string list from the instr which is separated by the given separator
 */
std::vector<std::string> string_list_create_bsep(const std::string& instr, char sep, const nap_compiler* _compiler, bool& psuccess)
{
    std::vector<std::string>  head;
    const char* p = instr.c_str(), *frst = instr.c_str();
    char* cur_elem = NULL;
    while(*p)
    {
        bool already_increased = false; /* when we load the expressions in parentheses we do an extra increment. This signals that this extra increment was done */
        if(C_SQPAR_OP == *p)    /* for now skip the things that are between index indicators*/
        {
        int can_stop = 0;
        int level = 0;
            p++;
            while(*p && !can_stop)
            {
                if(*p == C_SQPAR_CL && --level == -1) can_stop = 1;
                if(*p == C_SQPAR_OP) level ++;
                if(!can_stop) p++;
            }
            if(!*p)
            {
                _compiler->throw_error(E0009_PARAMISM, instr, NULL);
                psuccess = false;
                return std::vector<std::string>();
            }
            p++;    /* to skip the last closing brace*/
            already_increased = true;
        }

        if(*p == C_PAR_OP)    /* for now skip the things that are between parantheses too ...*/
        {
            int can_stop = 0;
            int level = 0;
            p++;
            while(*p && !can_stop)
            {
                if(*p == C_PAR_CL && --level == -1) can_stop = 1;
                if(*p == C_PAR_OP) level ++;
                if(!can_stop) p++;
            }
            if(!*p)
            {
                _compiler->throw_error(E0009_PARAMISM, instr, NULL);
                psuccess = false;
                return std::vector<std::string>();
            }
            p++;    /* to skip the last closing brace*/
            already_increased = true;
        }

        if(*p == '{' )    /* for now skip the things that are between curly braces too ...*/
        {
            int can_stop = 0;
            int level = 0;
            p++;
            while(*p && !can_stop)
            {
                if(*p == '}' && --level == -1) can_stop = 1;
                if(*p == '{' ) level ++;
                if(!can_stop) p++;
            }
            if(!*p)
            {
                _compiler->throw_error(E0009_PARAMISM, instr, NULL);
            }
            p++;    /* to skip the last closing brace*/
            already_increased = true;
        }


        if(*p == C_QUOTE)    /* skip stuff between quotes*/
        {
        int can_stop = 0;
            p++;
            while(*p && !can_stop)
            {
                if(*p == C_QUOTE && *(p-1) != C_BACKSLASH) can_stop = 1;
                already_increased = 1;
                p++;
            }
        }

        if(*p == sep)
        {
            cur_elem = (char*)calloc(p - frst + 1, 1);
            strncpy(cur_elem, frst, p - frst);
            std::string t = cur_elem;
            strim(t);
            head.push_back(t);
            frst = p + 1;
            already_increased = 1;
            p++; // to skip the separator

            free(cur_elem);
        }
        if(! already_increased)
        {
            p++;
        }
    }
    /* and now the last element */
    cur_elem = (char*)calloc(p - frst + 1, 1);
    strncpy(cur_elem, frst, p - frst);
    std::string t = cur_elem;
    strim(t);
    head.push_back(t);
    free(cur_elem);
    return head;
}


int is_immediate_byte(const std::string& t)
{
    if(t.length() != 3) return 0;
    if(t[0] != t[2]) return 0;
    if(t[0] != '\'') return 0;
    return 1;
}


int is_enclosed_string(const std::string& expr_trim, int expr_len, char encls, char encle)
{
    if(expr_trim[0] == encls)    /* we can check them ... */
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
 * @brief is_string returns true if the string is enclosed in quotes
 * @param expr_trim
 * @param expr_len
 * @return
 */
int is_string(const std::string& expr_trim, int expr_len)
{
    return is_enclosed_string(expr_trim, expr_len, C_QUOTE, C_QUOTE);
}

/**
 * @brief is_statement_string returns true if the strign is enclosed in bacquotes
 * @param expr_trim
 * @param expr_len
 * @return
 */
int is_statement_string(const std::string& expr_trim, int expr_len)
{
    return is_enclosed_string(expr_trim, expr_len, C_BACKQUOTE, C_BACKQUOTE);
}

/**
 * Returns the type ID of the given text type...
 */
int get_typeid(const std::string &type)
{
    if(type == STR_INT) return BASIC_TYPE_INT;
    if(type == STR_BYTE) return BASIC_TYPE_BYTE;
    if(type == STR_REAL) return BASIC_TYPE_REAL;
    if(type == STR_CHAR) return BASIC_TYPE_CHAR;
    if(type == STR_BOOL) return BASIC_TYPE_BOOL;
    if(type == STR_STRING) return BASIC_TYPE_STRING;
    if(type == STR_VOID) return BASIC_TYPE_VOID;

    return BASIC_TYPE_DONTCARE;
}

bool starts_with(const std::string& s1, const std::string& s2)
{
    return s2.size() <= s1.size() && s1.compare(0, s2.size(), s2) == 0;
}
