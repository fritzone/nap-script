#ifndef COMMON_STRUCTS_H
#define COMMON_STRUCTS_H

#include "type.h"

#include <stdio.h>
#include <vector>

struct call_context;
struct call_context_list;
struct method;
struct class_declaration;

/**
 * The String Basic Type
 */
struct bt_string
{
        /* this is the actual string */
        char* the_string;

        /* the length of the string */
        int len;
};

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
        const char* file_name;
};

/**
 * Holds the relationship between an expression and its physical location in the source file.
 */
struct expression_with_location
{
        /* this is an expression as read from the input file*/
        char* expression;

        /* the location of this expression in the input file */
        file_location* location;
};


/*
 * this tree holds the interpreted form of the formula
 */
struct expression_tree
{
        /* the left branch of the expression */
        struct expression_tree* left;

        /* the right branch of the expression */
        struct expression_tree* right;

        /* the info that can be found in the expression */
        char *info;

        /* the reference ofthe node ... can be a struct number, a struct variable, etc ... */
        struct envelope* reference;

        /* the type of the struct variable that can be found in this node (ie: real, integer)... used for type correct calculations */
        int v_type;

        /* the type of the operator if any */
        int op_type;

        /* the father of this node */
        struct expression_tree* father;

        /* this is the physical location of the expression (file, line, etc)*/
        const expression_with_location* expwloc;
};

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
        void* to_interpret;
};

/**
 * Definition for an indexed element
 **/
struct indexed
{
        /* the actual index when the index is one-dimensional */
        int idx;

        /* and what we are indexing. This usually is a struct variable */
        struct envelope* base;
};

/**
 * Represents a struct number.
 */
struct number
{
        /* type of the struct number */
        m_typeid type;

        /* the location of the number */
        void* location;
};

/** holds the data that is used in a list of values, { A, B, C} for example
 * listv structures are stored in envelopes, with type LIST_VALUE
 */
struct listv
{
        listv* next;
        envelope* val;
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
        char* name;

        /* the actual implementation of the parameter. it will contain a variable */
        envelope* value;

        /* the expression of this parameter as read from the source file. Used at function calling */
        expression_tree* expr;

        /* the initial value of the parameter, like: int f (int a=10, b)*/
        expression_tree* initial_value;

        /* 1 if the value is simpl (meaning, no dimensions) 0 if the value is not so simple, meaning dimensions */
        int simple_value;

    int type;
};


/** 
 * A list parameters  
 */
struct parameter_list
{
        /* the link to the next */
        parameter_list* next;

        /* the atual parameter */
        parameter* param;
};


/**
 * The structure of a parsed file
 */
struct parsed_file
{
        /* name of the file */
        const char* name;

        /* the file pointer*/
        FILE* fp;

        /* this is the content of the file*/
        char* content;

        /* the current position of the reader */
        long position;

        /* the size of the file ... */
        long content_size;

        /* the current line number */
        long current_line;

        /* the previous position where the parser was before reading the current expression */
        long previous_position;
};

/**
 * This structure defines a struct variable. The following things are characterizing a struct variable:
 * . their name
 * . their type as a string
 * . their dimension, meaning, they can be a vector, or a single struct variable
 * . their value(s). The values are struct envelope objects
 * . their type as an int, from the famous BASIC_TYPE... list
 * A variable can be bound together with a C/C++ variable of type long, double and with any dimensions
 * suitable. When binding with multi dimensions, each variable's number* to_interpret's location will
 * point to the specific index in the array passed in.
 */
struct variable
{
        /* the name of the variable */
        char* name;

        /* the size of the variable (ie. dimension) */
        int dimension;

        /* the value of the variable. It is an struct envelope, with all the nice properties of an struct envelope ...
           actually a list of envelopes, for more than one dimension */
        struct envelope** value;

        /* the type of the variable */
        char* c_type;

        /* the type of the variable */
        int i_type;

        /* the dimension definition of this variable */
        struct multi_dimension_def* mult_dim_def;

        /* the number of indexes if this is a multi-dim variable */
        int multi_dim_count;

        /* populated with NULL if this is not a function parameter, or the actual function parameter if it is */
        struct parameter* func_par;

        /* the template parameters of the variable if any */
        struct parameter_list* templ_parameters;

        /* used by the templ_parameters above */
        struct variable_list* templ_variables;

        /* whether this variable is static or not*/
        char static_var;

        /* whether this variable represents an environment variable or not */
        char environment_variable;

        /* 1 if this variable has dynamic dimensions, 0 if not*/
        char dynamic_dimension;
    
        /* the call context in which this variable is to be found */
        call_context* cc;
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
        struct multi_dimension_def* dimension_definition;

        /* the values that are used to build this dimension list */
        struct expression_tree_list* dimension_values;

        /* some id to uniquely identify this index */
        char* id;
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
        struct multi_dimension_def* next;

        /* the dimension if a direct number*/
        long dimension;

        /* whether this dimension is a dynamic one or not. See how this gets initialized / evaluated */
        char dynamic;

        /* the dimension if it's an expression. In this case the evaluator will initialize the dimension*/
        struct expression_tree* expr_def;
};


/** 
 * Contains a variable definition; to be evaluated at run time, to be created at interpretation time
 */
struct variable_definition
{
        /* the variable */
        struct variable* the_variable;

        /* the value that will be assigned to it*/
        struct expression_tree* the_value;

        /*the multi dimension definition for this variable if any, NULL if none*/
        struct multi_dimension_def* md_def;
};


/**
 * A list of variable definitions, just like: int  a = 10, b = a - 1;
 */
struct variable_definition_list
{
        /* the link to the next element*/
        struct variable_definition_list* next;

        /* the definition of the variable*/
        struct variable_definition* the_definition;
};

/**
 * Holds the information for the calling of a templated variable. Populated at interpret time,
 * used at run time.
 */
struct variable_template_reference
{
        /* this is the variable we are referencing to */
        struct variable* the_variable;

        /* these are the parameters that the user passed in */
        struct parameter_list* templ_pars;
};


/**
 * The call frame structure represents the "stack" way the functions are being called.
 * Each time a function is called it puts it "signature" (the address of the_method)
 * and the parameters onto a global call_frame variable. There is always required to be
 * at least one element on the global call_frame stack (the main application).
 * Also this structure holds the variable of the method being called at the current stage.
 * If the main application exits then the call_frame will be empty.
 * (The best of all: This can be used at a (very) later stage when developing threading. Each
 * thread like object will have its own call frame, and they'll be free to run independently) 
 */
struct call_frame_entry
{
        /* this is the method that was called */
        method* the_method;

        /* this are the parameters passed to the method */
        parameter_list* parameters;

        /* holds the previous call frame of the method... just in case*/
        call_frame_entry* previous_cf;
};

/**
 * Contains the definition of a method
 */
struct method
{
        /* the name of the method */
        char* name;

        /* the return type of the function */
        char* return_type;

        /* the list of struct variables that are defined in the method. This list is used in two situations:
         * 1. in the interpret phase, when the code is built up, this is used as references to find the variables.
         * 2. in the running phase, but only for static variables.
         */
        variable_list* variables;

        /* the parameters of the method */
        parameter_list* parameters;

        /* the call contexts this method is taking part from */
        call_context_list* contained_ins;

        /* this is the call context of the method, created when the method is created */
        call_context* main_cc;

        /* the current call frame. stores all the variables of the method which are not static. At each calling of a method
         * this variable gets populated with the call frame that has been constructed for the current stage
         */
        call_frame_entry* cur_cf;

        /* the definition location 0 - normal, 1 - extern*/
        int def_loc;

    int ret_type;
};

/**
 * A constructor call structure for the "new" keyword
 **/
struct constructor_call : public method
{
    class_declaration* the_class;
};

/**
 * Contains a list of methods accessible from a given context
 */
struct method_list
{
        /* link to the next element */
        method_list* next;

        /* the actual method*/
        method* the_method;
};



/**
 * Inserts in the given method list a new method
 * @param list - the address of the list we're inserting into
 * @param meth - the method
 */
method_list* method_list_insert(method_list** list, method* meth);


/**
 * Represents the list of active call frames (Call Stack). For easier access this is a double linked list
 */
struct call_frame_list
{
        struct call_frame_list* next;
        struct call_frame_list* prev;
        struct call_frame_entry* entry;
};

/*
 * List for holding the variables of some method
 */
struct variable_list
{
        /* link to the next element */
        variable_list *next;

        /* the actual variable */
        variable *var;
};

/*
 * List for holding a list of strings
 */
struct string_list
{
        /* connection to the next element*/
        string_list* next;

        /* the actual string */
        char* str;

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
    char* name;

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
    struct call_context_list* next;
    struct call_context* ctx;
};

/**
 * The call context is something like a namespace, or a class... basically its role is to group the methods
 * that belong to the same call context, and makes visible to each other. There is always a 'global' call context.
 * Variables by default belong to a call cotnext and they are destroyed when the call context ends
 */
struct call_context
{
    /* the type of the call context: 0 - global, 1 - named*/
    int type;

    /* the name of the call context */
    char* name;

    /* the methods of this call context */
    struct method_list* methods;

    /* the list of variable that have been defined in this call context */
    struct variable_list* variables;

    /* contains the list of expressions */
    struct expression_tree_list* expressions;

    /* these are the child call contexts */
    struct call_context_list* child_call_contexts;

    /* the father call context of this */
    struct call_context* father;

    /* if the call context is from a method this is that method */
    struct method* ccs_method;

    /* this is a vector of label locations for this*/
    std::vector<struct bytecode_label*>* labels;

    struct bytecode_label* break_label;
    
    /* the classes that are defined in this call context */
    std::vector<struct class_declaration*>* classes;
    
    /* the interfaces that are defined in this call context */
    std::vector<struct class_declaration*>* interfaces;
};

/**
 * Class representing a class declaration with relationships to a parent class
 * and a list of implemented interfaces
 **/
struct class_declaration : public call_context
{
    class_declaration* parent_class;
    std::vector<class_declaration*> implemented_interfaces;
};


#endif
