#ifndef _BSD_EXTR_H_
#define _BSD_EXTR_H_

#include <string>

/**
 * Adds a new expression to the list
 */
expression_tree_list* expression_tree_list_add_new_expression(const struct expression_tree* expression, struct expression_tree_list** first, const char* text_expr);

/**
 * Creates a new node, initializes all the contents to default values
 */
expression_tree* new_expression_tree(const expression_with_location* expwloc);

/**
 * Creates a new node, initializes all the contents to default values and sets the father to be the parameter
 */
expression_tree* new_expression_tree_with_father(struct expression_tree* father, const expression_with_location* expwloc);


#endif
