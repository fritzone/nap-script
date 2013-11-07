#ifndef COMMON_STRUCTS_H
#define COMMON_STRUCTS_H

#include "type.h"

#include <stdio.h>
#include <vector>
#include <string>
#include <list>

struct call_context;
struct call_context_list;
struct method;
struct class_declaration;
struct call_frame_entry;
struct expression_tree;
struct expression_tree_list;

/**
 * This structure holds the information about the location of a particular expression.
 * The parser creates this structure when parsing the file, and passes it in the functions that
 * store the 'compiled' expressions.
 */
struct file_location
{

    /* the location in the 'content' of the parsed_file structure */
    long location;

    /* the first line number */
    int start_line_number;

    /* the last line number */
    int end_line_number;

    /* this is the file name */
    const char *file_name;
};

/**
 * Holds the relationship between an expression and its physical location in the source file.
 */
struct expression_with_location
{
    /* this is an expression as read from the input file*/
    char *expression;

    /* the location of this expression in the input file */
    file_location *location;
};

/**
 * The struct envelope structure is holding a data tha can be worked
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
 * Definition for an indexed element
 **/
struct indexed
{
    /* the actual index when the index is one-dimensional */
    int idx;

    /* and what we are indexing. This usually is a struct variable */
    struct envelope *base;
};

/** holds the data that is used in a list of values, { A, B, C} for example
 * listv structures are stored in envelopes, with type LIST_VALUE
 */
struct listv
{
    listv *next;
    envelope *val;
};


/**
 * This struct represents a function's parameter. The implementation of this feature is like:
 * each parameter in its initial definition stage is a variable; When the function is called this variable
 * is initialized to the value it was called with
 */
struct parameter
{
    /* 1 if the parameter was sent as a reference */
    int modifiable;

    /* the name of the parameter (as used on te function's side). This is not used on the client side*/
    char *name;

    /* the actual implementation of the parameter. it will contain a variable */
    envelope *value;

    /* the expression of this parameter as read from the source file. Used at function calling */
    expression_tree *expr;

    /* the initial value of the parameter, like: int f (int a=10, b)*/
    expression_tree *initial_value;

    /* 1 if the value is simpl (meaning, no dimensions) 0 if the value is not so simple, meaning dimensions */
    int simple_value;

    /* the method in which this parameter belongs */
    method *the_method;

    int type;
};


/**
 * A list parameters
 */
struct parameter_list
{
    /* the link to the next */
    parameter_list *next;

    /* the atual parameter */
    parameter *param;
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
    struct multi_dimension_def *dimension_definition;

    /* the values that are used to build this dimension list */
    struct expression_tree_list *dimension_values;

    /* some id to uniquely identify this index */
    char *id;
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
    struct multi_dimension_def *next;

    /* the dimension if a direct number*/
    long dimension;

    /* whether this dimension is a dynamic one or not. See how this gets initialized / evaluated */
    char dynamic;

    /* the dimension if it's an expression. In this case the evaluator will initialize the dimension*/
    struct expression_tree *expr_def;
};


/**
 * Contains a variable definition; to be evaluated at run time, to be created at interpretation time
 */
struct variable_definition
{
    /* the variable */
    struct variable *the_variable;

    /* the value that will be assigned to it*/
    struct expression_tree *the_value;

    /*the multi dimension definition for this variable if any, NULL if none*/
    struct multi_dimension_def *md_def;
};


/**
 * A list of variable definitions, just like: int  a = 10, b = a - 1;
 */
struct variable_definition_list
{
    /* the link to the next element*/
    struct variable_definition_list *next;

    /* the definition of the variable*/
    struct variable_definition *the_definition;
};

/**
 * Holds the information for the calling of a templated variable. Populated at interpret time,
 * used at run time.
 */
struct variable_template_reference
{
    /* this is the variable we are referencing to */
    struct variable *the_variable;

    /* these are the parameters that the user passed in */
    struct parameter_list *templ_pars;
};

/*
 * List for holding a list of strings
 */
struct string_list
{
    /* connection to the next element*/
    string_list *next;

    /* the actual string */
    char *str;

    /* the length of the string, populated at run time*/
    int len;
};


/**
 * Class, describing a label, a jump location.
 */
struct bytecode_label
{
    enum label_type
    {
        LABEL_PLAIN = 0,
        LABEL_BREAK = 1,
        LABEL_CONTINUE = 2
    };

    /* the name of the label */
    char *name;

    /* the location of the label in the bytecode stream */
    long bytecode_location;

    /* the type of the location, can be 0 if this is just a plain label,
     *1 if this is a "break" location label,
     *2 if this is a "continue" location label */
    label_type type;
};

/*
 * The call context is something like a namespace and/or code-block ... See the code
 * below, to understand:
 *
 * // by default a 'global' namespace starts here, with name 'global'.
 * int myGlobalWarming;         // global variable, will be visible everywhere in the program
 * namespace my_precious { // new call context starts here with name 'my_precious'
 * int x;
 * int func()
 * {                            // another call context, which can see the x from above
 * int a;
 *     {                        // just a local nameless call context
 *     int b;           // b is a local variable, a still visible above
 *     }                        // b is not visible anymore, it was destroyed at the end of the nameless call context
 * }
 * }                            // x is destroyed here by default
 */

struct call_context;
struct class_declaration;

/**
 * This holds a list of call contexts.
 */
struct call_context_list
{
    struct call_context_list *next;
    struct call_context *ctx;
};

/**
 * The call context is something like a namespace, or a class... basically its role is to group the methods
 * that belong to the same call context, and makes visible to each other. There is always a 'global' call context.
 * Variables by default belong to a call cotnext and they are destroyed when the call context ends
 */
struct call_context
{

    /**
     * Create a new call context
     * @param type - the type of the call context
     * @param name - the name of the call context
     * @param the_method - the method in which this call context is
     * @param father - the father of this call cotnext
     **/
    call_context (int ptype,
                  const char *pname,
                  method *the_method,
                  call_context *pfather);

    /**
     * @brief call_context_add_method adds a method to the call context
     * @param the_method - the method which is to be added
     * @return the location in the list of the inserted method
     */
    void add_method(method *the_method);


    /**
     * @brief call_context_get_method returns a method from the call context
     * @param name - the name of the method
     * @return the method of found, null if not found
     */
    method *get_method(const char *name);


    /* the type of the call context: 0 - global, 1 - named*/
    int type;

    /* the name of the call context */
    std::string name;

    /* the methods of this call context */
    std::vector<method*> methods;

    /* the list of variable that have been defined in this call context */
    std::vector<variable*> variables;

    /* contains the list of expressions */
    struct expression_tree_list *expressions;

    /* the father call context of this */
    struct call_context *father;

    /* if the call context is from a method this is that method */
    struct method *ccs_method;

    /* this is a vector of label locations for this*/
    std::vector<struct bytecode_label *> labels;

    struct bytecode_label *break_label;

    /* the classes that are defined in this call context */
    std::vector<struct class_declaration *> classes;

    /* the interfaces that are defined in this call context */
    std::vector<struct class_declaration *> interfaces;
};

/**
 * Class representing a class declaration with relationships to a parent class
 * and a list of implemented interfaces
 **/
struct class_declaration : public call_context
{
    /**
     * @brief class_declaration_create creates a new class declaration
     * @param name - the name of the class
     * @param father - the call context in which this is
     * @return the newly created class declaration
     */
    class_declaration (const char *pname, call_context *pfather);

    class_declaration *parent_class;
    std::vector<class_declaration *> implemented_interfaces;
};


#endif
