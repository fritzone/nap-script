#ifndef _TREE_H_
#define _TREE_H_

#include "hash.h"

#include "bsd_enve.h"
#include "bsd_extr.h"

/**
 * This contains the list of expressions that can be found in a struct method
 */
struct expression_tree_list
{
	/* the link to the next */
	struct expression_tree_list* next;

	/* the root of the tree */
	const expression_tree* root;

	/* the text representation of this expression that is interpreted in the root */
	char* text_expression;
};

/**
 * Checks if the expression is valid for the given node
 */
int expression_is_valid( char* expr);

/**
 * Adds a new expression to the list
 */
struct expression_tree_list* expression_tree_list_add_new_expression(const struct expression_tree* expression, struct expression_tree_list** first, const char* text_expr);

/**
 * Creates a new node, initializes all the contents to default values
 */
struct expression_tree* new_expression_tree(const expression_with_location* expwloc);

/**
 * Creates a new node, initializes all the contents to default values and sets the father to be the parameter
 */
struct expression_tree* new_expression_tree_with_father(struct expression_tree* father, const expression_with_location* expwloc);

#endif
