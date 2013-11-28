#ifndef _RES_WRDS_H_
#define _RES_WRDS_H_

#include "common_structs.h"
#include "call_ctx.h"
#include "expression_tree.h"

/* 
 * This header file holds the structures that are associated with the reserved words (keyword).
 */

/**
 * The structure for the 'if' keyword
 */
struct resw_if
{
    /* holds the logical expression that will be evaluated when the 'if' is evaluated */
    expression_tree* logical_expr;

    /* the call context of the 'if' branch*/
    call_context* if_branch;

    /* the call context of the 'else' branch */
    call_context* else_branch;
};


/**
 * Structure associated with the while statements
 */
struct resw_while
{
    /* the logical expression that will be evaluated when checking if the while's body must be xecuted once more*/
    expression_tree* logical_expr;

    /* these are the operations that will be executed */
    call_context* operations;

    bytecode_label* break_label;
};

/**
 * This structure holds the internal data for a 'for' keyword.
 */
struct resw_for
{
    resw_for():init_stmt(0), condition(0), expr(0), tree_init_stmt(0),
        tree_condition(0), tree_expr(0), operations(0), unique_hash() {}

    char* init_stmt;
    char* condition;
    char* expr;

    expression_tree* tree_init_stmt;
    expression_tree* tree_condition;
    expression_tree* tree_expr;

    call_context* operations;

    std::string unique_hash;
};

#endif
