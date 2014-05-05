#ifndef _BSD_EXTR_H_
#define _BSD_EXTR_H_

#include <string>
#include "number.h"

struct expression_with_location;

/*
 * this tree holds the interpreted form of the formula
 */
struct expression_tree
{
    expression_tree(const expression_with_location* pexpwloc, expression_tree* pfather = 0)
    {
        v_type = BASIC_TYPE_DONTCARE;
        op_type = NO_OPERATOR;
        expwloc = pexpwloc;
        father = pfather;
        left = 0;
        right = 0;
    }

    /* the left branch of the expression */
    struct expression_tree *left;

    /* the right branch of the expression */
    struct expression_tree *right;

    /* the info that can be found in the expression */
    std::string info;

    /* the reference ofthe node ... can be a struct number, a struct variable, etc ... */
    struct envelope *reference;

    /* the type of the struct variable that can be found in this node (ie: real, integer)... used for type correct calculations */
    int v_type;

    /* the type of the operator if any */
    int op_type;

    /* the father of this node */
    struct expression_tree *father;

    /* this is the physical location of the expression (file, line, etc)*/
    const expression_with_location *expwloc;
};

/**
 * Creates a new node, initializes all the contents to default values
 */
expression_tree* new_expression_tree(const expression_with_location* expwloc);

/**
 * Creates a new node, initializes all the contents to default values and sets the father to be the parameter
 */
expression_tree* new_expression_tree_with_father(struct expression_tree* father, const expression_with_location* expwloc);


#endif
