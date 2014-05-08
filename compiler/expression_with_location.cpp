#include "common_structs.h"
#include "expression_tree.h"

expression_with_location::~expression_with_location()
{
    for(size_t i=0; i<compiled_expressions.size(); i++)
    {
        delete compiled_expressions[i];
    }
}


expression_tree *expression_with_location::new_expression()
{
    expression_tree* expr = new expression_tree(this) ;
    compiled_expressions.push_back(expr);
    return expr;
}

expression_tree *expression_with_location::new_expression(expression_tree *father)
{
    expression_tree* expr = new expression_tree(this, father) ;
    compiled_expressions.push_back(expr);
    return expr;
}

