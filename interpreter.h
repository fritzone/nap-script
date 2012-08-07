#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "call_ctx.h"
#include "tree.h"
#include "method.h"
#include "envelope.h"
#include "parser.h"

/*
 * This file contains all the functions that handle the interpretation task.
 */

/**
 * Builds an expression from the given string. Stores it in the tree found at node
 */
void* build_expr_tree(const char *expr, expression_tree* node, method* the_method, const char* orig_expr, call_context* cc, int* result, const expression_with_location* location);

/**
 * Validates the given node
 */
void validate(struct expression_tree* node);

#endif
