#include "tree.h"
#include "utils.h"
#include "sys_brkp.h"
#include "number.h"
#include "type.h"
#include "throw_error.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * Checks if this expression can be added to the tree as an expression for a  variable.
 */
int expression_is_valid( char* expr)
{
	if(strstr(expr, "(")) return 0;
	if(strstr(expr, ")")) return 0;
	return 1;
}

/**
 * Creates a new node
 */
expression_tree* new_expression_tree(const expression_with_location* expwloc)
{
expression_tree* tmp = alloc_mem(expression_tree,1);
	tmp->v_type = BASIC_TYPE_DONTCARE;
	tmp->op_type = NO_OPERATOR;
	tmp->expwloc = expwloc;
	return tmp;
}

/**
* Creates a new node, initializes all the contents
*/
expression_tree* new_expression_tree_with_father(expression_tree* father, const expression_with_location* expwloc)
{
 expression_tree* tmp = new_expression_tree(expwloc);
	tmp->father = father;
	return tmp;
}


/**
 * Adds a new expression to the given list
 */
expression_tree_list* expression_tree_list_add_new_expression(const struct expression_tree* expression, struct expression_tree_list** first, const char* text_expr)
{
expression_tree_list *q , *tmp;
	if(NULL == first)
	{
		throw_error("Internal: an expression tree list's address is 0", NULL);
	}
	//printf("[TreeListExpr:Add] %s\n", text_expr);
	q= *first; 

	if(NULL == *first)
	{
		*first = alloc_mem(expression_tree_list,1);
		(*first)->root = expression;
		(*first)->text_expression = duplicate_string(text_expr);
		return *first;
	}
	while(q->next) {q=q->next;}
	tmp = alloc_mem(expression_tree_list,1);
	tmp->root = expression;
	tmp->text_expression = duplicate_string(text_expr);
	q->next = tmp;
	return tmp;
}
