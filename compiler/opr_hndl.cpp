#include "opr_hndl.h"
#include "consts.h"
#include "type.h"
#include "utils.h"
#include "compiler.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Look for a level 0 operator which can be a character long. Useful for cases like
 * checking the validity of ++ and +.
 * Returns -1 in case nothing was found or the location of the operator which comes first
 * between op1, op2 in the list
 */
int level_0_char_operator(const nap_compiler* _compiler, const std::string &expr, char op1, char op2, int need_first, bool& psuccess)
{
    signed int i=0, len = expr.length();
    if(len == 1 && expr[0] != op1 && expr[0] != op2)
    {
        return -1;
    }

    // special case: a unary operator found at pos 0:
    if(expr[0] == op1 || expr[0] == op2)
    {
        if(len > 1 && (expr[1] != op1 && expr[1] != op2) )
        {
            return 0;
        }
    }

    int level = 0, found_idx = -1;
    while(i<len)
    {
        bool already_inced = false;
        if( expr[i] == C_PAR_OP ) level++;
        if( expr[i] == C_PAR_CL )
        {
            level--;
            if(level == -1)
            {
                _compiler->throw_error(E0012_SYNTAXERR, expr, NULL);
                psuccess = false;
                return -1;

            }
        }

        if(expr[i] == C_SQPAR_OP) /* now check if we are currently placed on a [ character. if yes, just skip till we find its closing */
        {
        int sq_level = 0;
        int can_stop = 0;
            i++;
            while(!can_stop && i<len)
            {
                if(expr[i] == C_SQPAR_CL)
                {
                    if( --sq_level == -1)
                    {
                        can_stop = 1;
                    }
                }
                if(expr[i] == C_SQPAR_OP)
                {
                    sq_level ++;
                }

                if(!can_stop)
                {
                    i++;
                }
            }

            if(i == len)
            {
                _compiler->throw_error(E0009_PARAMISM, expr, NULL);
                psuccess = false;
                return -1;
            }
        }

        /* try to skip if any quotes were found */
        if(expr[i] == '\"')
        {
        int can_stop = 0;
            i++;
            while(i<len && !can_stop)
            {
                if(expr[i] == '\"' && expr[i-1] != '\\')
                {
                    can_stop = 1;
                }
                else
                {
                    i++;
                }
            }
            if(i == len)
            {
                _compiler->throw_error(E0025_QMISM, expr, NULL);
                psuccess = false;
                return -1;
            }

            i++; /* to skip the closing quote, without affecting  */
            already_inced = true;
        }

        //if(i < len && i+1 < len && i+2 < len) /* This if will check the post/pre increments so it needs to be aware of the length of the string */
        if( !is_whitespace(expr[i]) && ( (
             ( (i < len && expr[i]==op1) || expr[i]==op2) && (level==0)
                && ( (i+1 < len && i-1>=0 && expr[i+1] != expr[i] && expr[i-1] != expr[i]) )
            )/* check if not pre-post increment */
                ||
                (
                    (i+1 < len && i+2 < len && i-2 >=0 && i-1 >=0
                    &&
                    ( (i+1 <len && expr[i+1] == expr[i])
                      &&
                      (i-1 >= 0 && expr[i-1] == expr[i])
                      &&
                      (i+2 < len && expr[i+2] == expr[i])
                      &&
                      (i-2 >= 0 && expr[i-2] == expr[i])
                    )) /* check if pre/post increment followed by a +/- sign again: x+++++y */
                    ||
                    (
                      (i+1 < len && expr[i+1] != expr[i])
                      &&
                      (i-1 >= 0 && expr[i-1] == expr[i])
                      &&
                      (i+2 < len && expr[i+2] != expr[i])
                      &&
                      (i-2 >= 0 && expr[i-2] == expr[i])
                      &&
                      (i+2 < len && expr[i+2] == expr[i+1])
                    ) /*  check if pre/post increment followed by a +/- sign again: x+++--y */
                )
              ) // closes the one after the &&
            )
        {
            if(need_first)
            {
                if(!isalnum(expr[i]))
                {
                    return i;
                }
            }
            else
            {
                if(!isalnum(expr[i]))
                {
                    found_idx = i;
                }
            }

        }
        if(!already_inced) i++;
    }
    return found_idx;
}

/**
 * Look for a level 0 operator, specifies if we need the last occurence or the first
 * The operator can be longer than a single character
 */
int level_0_longer_operator(const nap_compiler* _compiler, const std::string& expr, const char* op, int need_last, bool& psuccess)
{
signed int len = expr.length(), lenopc = strlen(op), i = 0;
int last_idx = -1, level = 0;

    while(i<len)
    {
        if(expr[i]==C_PAR_OP)level++;
        if(expr[i]==C_PAR_CL)
        {
            level--;
            if(level == -1)
            {
                _compiler->throw_error(E0009_PARAMISM, expr, NULL);
                psuccess = false;
                return -1;
            }
        }
        if(expr[i] == C_SQPAR_OP)
        {
        int sq_level = 0;    /* square bracket level for square bracket */
        int can_stop = 0;
            i++;
            while(!can_stop && i<len)
            {
                if(expr[i] == C_SQPAR_CL)
                {
                    sq_level --;
                    if(sq_level == -1)
                    {
                        can_stop = 1;
                    }
                }
                if(expr[i] == C_SQPAR_OP)
                {
                    sq_level ++;
                }

                if(!can_stop)
                {
                    i++;
                }

            }

            if(i == len)
            {
                _compiler->throw_error(E0009_PARAMISM, expr, NULL);
                psuccess = false;
                return -1;
            }
        }

        /* try to skip if any quotes were found */
        if(expr[i] == '\"')
        {
            int can_stop = 0;
            i++;
            while(i<len && !can_stop)
            {
                if(expr[i] == '\"' && expr[i-1] != '\\')
                {
                    can_stop = 1;
                }
                else
                {
                    i++;
                }
            }
            if(i == len)
            {
                _compiler->throw_error(E0025_QMISM, expr, NULL);
                psuccess = false;
                return -1;
            }

            i++; /*skipping the closing quote*/
        }


        if((i<len && expr[i]==op[0] && level==0))
        {
        int found = 1;
        signed int j=i;
        signed int opc = 0;

            while(j<len && opc < lenopc)
            {
                if(expr[j] != op[opc])
                {
                    found = 0;
                    break;
                }
                if(j == len - 1 && opc != lenopc - 1) /* might not work ...*/
                {
                    found = 0;
                    break;
                }
                j++; opc++;
            }

            /* now check for the specific case of '==' and '=' */

            if(found && !strcmp(op, STR_EQUAL))
            {
                if(expr[i] == C_EQ && (i+1)<len && expr[i+1] == C_EQ) found = 0;
                if(expr[i] == C_EQ && i>0 && expr[i-1] == C_EQ) found = 0;
            }

            if(found)
            {
                if(!need_last)    return i;
                else last_idx = i;
            }
        }
        i++;
    }

    return last_idx;
}


/**
 * Looks for a level 0 additive operator, such as + -
 */
int level_0_add_operator(const nap_compiler* _compiler, const std::string& expr, bool& psuccess)
{
    /* TODO: Check: if the operator was found, but :
        1. it is + or -
        2. There is an operator before it
        3. it is strictly attached to the following term,
        4. which is a number

        do not consider since it might be part of a number: 3  * -2
*/
    return level_0_char_operator(_compiler, expr, C_ADD, C_SUB, 0, psuccess);
}

/**
 * Looks for level 0 bitwise operators:
 * && || !
 */
int level_0_logical_operator(const nap_compiler* _compiler, const std::string& expr, bool& psuccess)
{
    int opr = level_0_longer_operator(_compiler, expr, STR_LOGIC_OR, 0, psuccess);
    if(opr > -1) return opr;
    SUCCES_OR_RETURN -1;

    opr = level_0_longer_operator(_compiler, expr, STR_LOGIC_AND, 0, psuccess);
    if(opr > -1) return opr;
    SUCCES_OR_RETURN -1;

    opr = level_0_longer_operator(_compiler, expr, STR_LOGIC_NOT, 0, psuccess);
    if(opr > -1) return opr;
    SUCCES_OR_RETURN -1;

    return -1;
}

/**
 * Looks for level 0 bitwise operators:
 * & | ~ ^
 */
int level_0_bitwise_operator(const nap_compiler* _compiler, const std::string& expr, bool &psuccess)
{
    int opr = level_0_longer_operator(_compiler, expr, STR_BIT_OR, 0, psuccess);
    SUCCES_OR_RETURN -1;
    if(opr > -1) return opr;

    opr = level_0_longer_operator(_compiler, expr, STR_BIT_XOR, 0, psuccess);
    SUCCES_OR_RETURN -1;
    if(opr > -1) return opr;

    opr = level_0_longer_operator(_compiler, expr, STR_BIT_AND, 0, psuccess);
    SUCCES_OR_RETURN -1;
    if(opr > -1) return opr;

    opr = level_0_longer_operator(_compiler, expr, STR_BIT_COMP, 0, psuccess);
    SUCCES_OR_RETURN -1;
    if(opr > -1) return opr;

    return -1;
}

/**
 * Looks for a level 0 multiplicative operator
 */
int level_0_multiply_operator(const nap_compiler* _compiler, const std::string& expr, bool& psuccess)
{
    int posMply = level_0_longer_operator(_compiler, expr, STR_MUL, 1, psuccess);
    SUCCES_OR_RETURN -1;
    int posDiv = level_0_longer_operator(_compiler, expr, STR_DIV, 1, psuccess);
    SUCCES_OR_RETURN -1;
    int modDiv = level_0_longer_operator(_compiler, expr, STR_MOD, 1, psuccess);
    SUCCES_OR_RETURN -1;
    return (max_int (posMply, max_int(posDiv, modDiv)));
}

/**
 * Looks for a level 0 assignment operator
 */
int level_0_assignment_operator(const nap_compiler* _compiler, const std::string& expr, bool& psuccess)
{
    int t = level_0_longer_operator(_compiler, expr, STR_EQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;
    return t;
}

/**
 * Looks for a level 0 dot operator
 */
int level_0_dot_operator(const nap_compiler* _compiler, const std::string &expr, bool& psuccess)
{
    int t = level_0_longer_operator(_compiler, expr, STR_DOT, 0, psuccess);
    SUCCES_OR_RETURN -1;
    return t;
}

/**
 * Finds the shift operations on the 0th level. Left shift has priority
 */
int level_0_shift(const nap_compiler* _compiler, const std::string &expr, bool& psuccess)
{
    int posleft = level_0_longer_operator(_compiler, expr, STR_SHLEFT, 0, psuccess);
    SUCCES_OR_RETURN -1;

    int posrigh = level_0_longer_operator(_compiler, expr, STR_SHRIGHT, 0, psuccess);
    SUCCES_OR_RETURN -1;

    return posleft==-1? posrigh : posleft;
}

/**
* Looks for a level 0 comparison operator.
* Returns the index, if any found and the found_operator is populated with the found op
*/
int level_0_comparison_operator(const nap_compiler* _compiler, const std::string &expr, const char** found_operator, bool& psuccess)
{
    int l0_cmp_op = level_0_longer_operator(_compiler, expr, STR_EQUALEQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_cmp_op > -1)
    {
        *found_operator = STR_EQUALEQUAL;
        return l0_cmp_op;
    }
    l0_cmp_op = level_0_longer_operator(_compiler, expr, STR_NEQ, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_cmp_op > -1)
    {
        *found_operator = STR_NEQ;
        return l0_cmp_op;
    }
    /* the order is important, LTE should be checked before LT since other way it might find the < op
       and ignore the = after it, leading to errors */
    l0_cmp_op = level_0_longer_operator(_compiler, expr, STR_LTE, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_cmp_op > -1)
    {
        *found_operator = STR_LTE;
        return l0_cmp_op;
    }
    l0_cmp_op = level_0_longer_operator(_compiler, expr, STR_GTE, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_cmp_op > -1)
    {
        *found_operator = STR_GTE;
        return l0_cmp_op;
    }
    l0_cmp_op = level_0_char_operator(_compiler, expr, '<', '<', 1, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_cmp_op > -1)
    {
        *found_operator = STR_LT;
        return l0_cmp_op;
    }
    l0_cmp_op = level_0_char_operator(_compiler, expr, '>', '>', 1, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_cmp_op > -1)
    {
        *found_operator = STR_GT;
        return l0_cmp_op;
    }
    return -1;
}

/**
 * Finds the first ocurence of a level 0 x= operator.
 */
int level_0_sg_eq_operator(const nap_compiler* _compiler, const std::string &expr, const char** found_operator, int* found_type, bool&psuccess)
{
    int l0_seq_op = level_0_longer_operator(_compiler, expr, STR_PLUS_EQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_seq_op > -1)
    {
        *found_operator = STR_PLUS_EQUAL;
        *found_type = OPERATOR_PLUS_EQUAL;
        return l0_seq_op;
    }

    l0_seq_op = level_0_longer_operator(_compiler, expr, STR_MINUS_EQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_seq_op > -1)
    {
        *found_operator = STR_MINUS_EQUAL;
        *found_type = OPERATOR_MINUS_EQUAL;
        return l0_seq_op;
    }

    l0_seq_op = level_0_longer_operator(_compiler, expr, STR_MUL_EQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_seq_op > -1)
    {
        *found_operator = STR_MUL_EQUAL;
        *found_type = OPERATOR_MUL_EQUAL;
        return l0_seq_op;
    }

    l0_seq_op = level_0_longer_operator(_compiler, expr, STR_DIV_EQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_seq_op > -1)
    {
        *found_operator = STR_DIV_EQUAL;
        *found_type = OPERATOR_DIV_EQUAL;
        return l0_seq_op;
    }

    l0_seq_op = level_0_longer_operator(_compiler, expr, STR_AND_EQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_seq_op > -1)
    {
        *found_operator = STR_AND_EQUAL;
        *found_type = OPERATOR_AND_EQUAL;
        return l0_seq_op;
    }

    l0_seq_op = level_0_longer_operator(_compiler, expr, STR_OR_EQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_seq_op > -1)
    {
        *found_operator = STR_OR_EQUAL;
        *found_type = OPERATOR_OR_EQUAL;
        return l0_seq_op;
    }

    l0_seq_op = level_0_longer_operator(_compiler, expr, STR_XOR_EQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_seq_op > -1)
    {
        *found_operator = STR_XOR_EQUAL;
        *found_type = OPERATOR_XOR_EQUAL;
        return l0_seq_op;
    }

    l0_seq_op = level_0_longer_operator(_compiler, expr, STR_SHL_EQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_seq_op > -1)
    {
        *found_operator = STR_SHL_EQUAL;
        *found_type = OPERATOR_SHL_EQUAL;
        return l0_seq_op;
    }

    l0_seq_op = level_0_longer_operator(_compiler, expr, STR_SHR_EQUAL, 0, psuccess);
    SUCCES_OR_RETURN -1;

    if(l0_seq_op > -1)
    {
        *found_operator = STR_SHR_EQUAL;
        *found_type = OPERATOR_SHR_EQUAL;
        return l0_seq_op;
    }

    return -1;
}
