#ifndef _TREE_H_
#define _TREE_H_

#include "hash.h"
#include "common_structs.h"

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
