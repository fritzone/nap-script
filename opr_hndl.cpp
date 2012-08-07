#include "opr_hndl.h"
#include "consts.h"
#include "typedefs.h"
#include "type.h"
#include "throw_error.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Look for a level 0 operator which can be a character long. Useful for cases like
 * checking the validity of ++ and +.
 * Returns -1 in case nothing was found or the location of the operator which comes first
 * between op1, op2 in the list
 */
int level_0_char_operator(const char *expr, char op1, char op2, int need_first)
{
signed int i=0, len = strlen(expr);
	if(len == 1 && expr[0] != op1 && expr[0] != op2) return -1;
int level = 0, found_idx = -1;
	while(i<len)
	{
		if( expr[i] == C_PAR_OP ) level++;
		if( expr[i] == C_PAR_CL )
		{
			level--;
			if(level == -1)
			{
				throw_error(E0012_SYNTAXERR, expr, NULL);
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
				throw_error(E0009_PARAMISM, expr, NULL);
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
				throw_error(E0025_QMISM, expr, NULL);
			}

			i++; /* to skip the closing quote, without affecting  */
		}

		//if(i < len && i+1 < len && i+2 < len) /* This if will check the post/pre increments so it needs to be aware of the length of the string */
        if( (
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
		i++;
	}
	return found_idx;
}

/**
 * Look for a level 0 operator, specifies if we need the last occurence or the first
 * The operator can be longer than a single character
 */
int level_0_longer_operator(const char *expr, const char* op, int need_last)
{
signed int len = strlen(expr), lenopc = strlen(op), i = 0;
int last_idx = -1, level = 0;

	while(i<len)
	{
		if(expr[i]==C_PAR_OP)level++;
		if(expr[i]==C_PAR_CL)
		{
			level--;
			if(level == -1)
			{
				throw_error(E0009_PARAMISM, expr, NULL);
			}
		}
		if(expr[i] == C_SQPAR_OP)
		{
		int sq_level = 0;	/* square bracket level for square bracket */
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
				throw_error(E0009_PARAMISM, expr, NULL);
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
				throw_error(E0025_QMISM, expr, NULL);
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
				if(!need_last)	return i;
				else last_idx = i;
			}
		}
		i++;
	}

	return last_idx;
}


/**
 * Looks for a lvel 0 additive operator, such as + -
 */
int level_0_add_operator(const char* expr)
{
	return level_0_char_operator(expr, C_ADD, C_SUB, 0);
}

/**
 * Looks for level 0 bitwise operators:
 * && || !
 */
int level_0_logical_operator(const char* expr)
{
int opr = level_0_longer_operator(expr, STR_LOGIC_OR, 0);
	if(opr > -1) return opr;

	opr = level_0_longer_operator(expr, STR_LOGIC_AND, 0);
	if(opr > -1) return opr;

	opr = level_0_longer_operator(expr, STR_LOGIC_NOT, 0);
	if(opr > -1) return opr;

	return -1;
}

/**
 * Looks for level 0 bitwise operators:
 * & | ~ ^
 */
int level_0_bitwise_operator(const char* expr)
{
int opr = level_0_longer_operator(expr, STR_BIT_OR, 0);
	if(opr > -1) return opr;

	opr = level_0_longer_operator(expr, STR_BIT_XOR, 0);
	if(opr > -1) return opr;

	opr = level_0_longer_operator(expr, STR_BIT_AND, 0);
	if(opr > -1) return opr;

	opr = level_0_longer_operator(expr, STR_BIT_COMP, 0);
	if(opr > -1) return opr;

	return -1;
}

/**
 * Looks for a level 0 multiplicative operator
 */
int level_0_multiply_operator(const char *expr)
{
int posMply = level_0_longer_operator(expr, STR_MUL, 1);
int posDiv = level_0_longer_operator(expr, STR_DIV, 1);
int modDiv = level_0_longer_operator(expr, STR_MOD, 1);
	return (max_int (posMply, max_int(posDiv, modDiv)));
}

/**
 * Looks for a level 0 multiplicative operator
 */
int level_0_assignment_operator(const char *expr)
{
	return level_0_longer_operator(expr, STR_EQUAL, 0);
}

/**
 * Finds the shift operations on the 0th level. Left shift has priority
 */
int level_0_shift(const char* expr)
{
int posleft = level_0_longer_operator(expr, STR_SHLEFT, 0);
int posrigh = level_0_longer_operator(expr, STR_SHRIGHT, 0);
	return posleft==-1? posrigh : posleft;
}

/**
* Looks for a level 0 comparison operator.
* Returns the index, if any found and the found_operator is populated with the found op
*/
int level_0_comparison_operator(const char *expr, const char** found_operator)
{
	int l0_cmp_op = level_0_longer_operator(expr, STR_EQUALEQUAL, 0);
	if(l0_cmp_op > -1)
	{
		*found_operator = STR_EQUALEQUAL;
		return l0_cmp_op;
	}
	l0_cmp_op = level_0_longer_operator(expr, STR_NEQ, 0);
	if(l0_cmp_op > -1)
	{
		*found_operator = STR_NEQ;
		return l0_cmp_op;
	}
	/* the order is important, LTE should be checked before LT since other way it might find the < op
	   and ignore the = after it, leading to errors */
	l0_cmp_op = level_0_longer_operator(expr, STR_LTE, 0);
	if(l0_cmp_op > -1)
	{
		*found_operator = STR_LTE;
		return l0_cmp_op;
	}
	l0_cmp_op = level_0_longer_operator(expr, STR_GTE, 0);
	if(l0_cmp_op > -1)
	{
		*found_operator = STR_GTE;
		return l0_cmp_op;
	}
	l0_cmp_op = level_0_char_operator(expr, '<', '<', 1);
	if(l0_cmp_op > -1)
	{
		*found_operator = STR_LT;
		return l0_cmp_op;
	}
	l0_cmp_op = level_0_char_operator(expr, '>', '>', 1 );
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
int level_0_sg_eq_operator(const char *expr, const char** found_operator, int* found_type)
{
int l0_seq_op = level_0_longer_operator(expr, STR_PLUS_EQUAL, 0);
	if(l0_seq_op > -1)
	{
		*found_operator = STR_PLUS_EQUAL;
		*found_type = OPERATOR_PLUS_EQUAL;
		return l0_seq_op;
	}

	l0_seq_op = level_0_longer_operator(expr, STR_MINUS_EQUAL, 0);
	if(l0_seq_op > -1)
	{
		*found_operator = STR_MINUS_EQUAL;
		*found_type = OPERATOR_MINUS_EQUAL;
		return l0_seq_op;
	}

	l0_seq_op = level_0_longer_operator(expr, STR_MUL_EQUAL, 0);
	if(l0_seq_op > -1)
	{
		*found_operator = STR_MUL_EQUAL;
		*found_type = OPERATOR_MUL_EQUAL;
		return l0_seq_op;
	}

	l0_seq_op = level_0_longer_operator(expr, STR_DIV_EQUAL, 0);
	if(l0_seq_op > -1)
	{
		*found_operator = STR_DIV_EQUAL;
		*found_type = OPERATOR_DIV_EQUAL;
		return l0_seq_op;
	}

	l0_seq_op = level_0_longer_operator(expr, STR_AND_EQUAL, 0);
	if(l0_seq_op > -1)
	{
		*found_operator = STR_AND_EQUAL;
		*found_type = OPERATOR_AND_EQUAL;
		return l0_seq_op;
	}

	l0_seq_op = level_0_longer_operator(expr, STR_OR_EQUAL, 0);
	if(l0_seq_op > -1)
	{
		*found_operator = STR_OR_EQUAL;
		*found_type = OPERATOR_OR_EQUAL;
		return l0_seq_op;
	}

	l0_seq_op = level_0_longer_operator(expr, STR_XOR_EQUAL, 0);
	if(l0_seq_op > -1)
	{
		*found_operator = STR_XOR_EQUAL;
		*found_type = OPERATOR_XOR_EQUAL;
		return l0_seq_op;
	}

	l0_seq_op = level_0_longer_operator(expr, STR_SHL_EQUAL, 0);
	if(l0_seq_op > -1)
	{
		*found_operator = STR_SHL_EQUAL;
		*found_type = OPERATOR_SHL_EQUAL;
		return l0_seq_op;
	}

	l0_seq_op = level_0_longer_operator(expr, STR_SHR_EQUAL, 0);
	if(l0_seq_op > -1)
	{
		*found_operator = STR_SHR_EQUAL;
		*found_type = OPERATOR_SHR_EQUAL;
		return l0_seq_op;
	}

	return -1;
}
