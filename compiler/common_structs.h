#ifndef COMMON_STRUCTS_H
#define COMMON_STRUCTS_H

#include "type.h"

#include <stdio.h>
#include <vector>
#include <string>
#include <list>

#define SUCCES_OR_RETURN  if(!psuccess) return



struct call_context;
struct call_context_list;
struct method;
struct class_declaration;
struct expression_tree;
struct expression_tree_list;
struct parameter;
struct variable;

/**
 * This structure holds the information about the location of a particular expression.
 * The parser creates this structure when parsing the file, and passes it in the functions that
 * store the 'compiled' expressions.
 */
struct file_location
{

    file_location() : location(-1), start_line_number(-1),
        end_line_number(-1), mfile_index(-1)
    {}

    file_location(long plocation, long pstart_line, long pend_line, int file_index)
        : location(plocation), start_line_number(pstart_line),
          end_line_number(pend_line), mfile_index(file_index)
    {}

    /* the location in the 'content' of the parsed_file structure */
    long location;

    /* the first line number */
    int start_line_number;

    /* the last line number */
    int end_line_number;

    /* this is the index of the file name in the map of */
    int mfile_index;

};

/**
 * Holds the relationship between an expression and its physical location in the source file.
 */
struct expression_with_location
{
    expression_with_location(){}

    ~expression_with_location();

    expression_tree* new_expression();

    expression_tree* new_expression(expression_tree*);

    // Contains a list of tree expressions that were created from this
    std::vector<expression_tree*> compiled_expressions;

    /* this is an expression as read from the input file*/
    std::string expression;

    /* the location of this expression in the input file */
    file_location location;
};

/**
 * The envelope structure is holding a data tha can be worked
 *  with using the same interface by the functions.
 * It has two members:
 * 1. the type. This can denote a struct variable, a struct number, a
 *    struct indexed structure or something else
 * 2. the void* pointer, which is the actual data that needs to be interpreted
 */
struct envelope
{
    /* the type */
    int type;

    /* what is in this struct envelope */
    void *to_interpret;
};


/**
 * Represents a multi-dimensional structure: int x[12,34,67];
 * Also, it can represent some expression instead of a single number. In case expr_def is NULL
 * the dimension value must contain a valid value. In case dimension is -1 the expr_def
 * must contain an expression wchich will be evaluated run-time.
 * This structure is used in the declaration/definition of the variables
 */
struct multi_dimension_def
{
    /* the relation to the next element */
    multi_dimension_def *next;

    /* the dimension if a direct number*/
    long dimension;

    /* whether this dimension is a dynamic one or not. See how this gets initialized / evaluated */
    bool dynamic;

    /* the dimension if it's an expression. In this case the evaluator will initialize the dimension*/
    expression_tree *expr_def;
};

/**
 * Contains the definition for a multidimensional index usage, such as:
 *  x[i, i+2, 3]
 * The index_definition is the structure from above, the index_values is the
 * list of expression_tree objects.
 * This structure is used when working with multi-dimension indexes
 */
struct multi_dimension_index
{
    /* the way this dimension is defined */
    multi_dimension_def *dimension_definition;

    /* the values that are used to build this dimension list */
    std::vector<expression_tree*>* dimension_values;
};

/**
 * Contains a variable definition; to be compiled at compile time,
 * to be created at interpretation time
 */
struct variable_definition
{
    /* the variable */
    variable *the_variable;

    /* the value that will be assigned to it*/
    expression_tree *the_value;

    /*the multi dimension definition for this variable if any, NULL if none*/
    multi_dimension_def *md_def;
};

#endif
