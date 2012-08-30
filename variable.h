#ifndef _variable_H_
#define _variable_H_

#include "envelope.h"
#include "parser.h"

struct call_context;
/*
 * Variables and methods associated with them
 */


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
 * Creates a new multi dimension index
 */
struct multi_dimension_index* new_multi_dimension_index(const char*);

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
 * Duplicates the variable definition, returns the duplicated item
 */
variable_definition* variable_definition_duplicate(variable_definition* source);

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


/*
 * Functions 
 */

struct variable_template_reference* new_variable_template_reference(variable* var, parameter_list* pars);

/**
 * Creates a new struct variable and returns it.
 * @param dimension - the dimension of the variable
 * @param type - the type of the variable
 * @return a newly initialized variable, with dimension and type initialized
 */
struct variable* new_variable(int dimension, int type);

/**
 * Adds a new dimension to the variable. Also updates the content of the inner structures,
 * moves the values and resizes them.
 * @param var - the variable
 * @param dimension - the dimension to add
 */
void variable_add_dimension(struct variable* var, int dimension);

/**
 * Sets the struct indexed int value
 */
void var_set_indexed_int_value(struct variable* var, long value, int index);

/**
 * Returns the basic type ofthe variable
 */
int variable_get_basic_type(const variable* const var);

/**
 * Sets the value at the given index
 */
void variable_set_indexed_value(struct variable* var, struct envelope* new_value, int index);

/**
 * Sets the value of this struct variable to the given string
 */
void var_set_indexed_string_value(struct variable* var,  char* src, int index);

/**
 * Sets the struct indexed value as double
 */
void var_set_indexed_double_value(struct variable* var, double value, int index);

/**
 * Returns the index in the variable's long list of envelope values for the given multi dimension
 */
long variable_get_index_for_multidim(struct variable* var, struct multi_dimension_def* indexes, int on1);

/**
 * Resolves the templates of the given variable
 */
void variable_resolve_templates(struct variable* the_variable, struct method* the_method, struct call_context* cc, const expression_with_location* expwloc);

/**
 * Returns the maximum indec for this varible
 */
int variable_get_max_index(struct variable* var);

/**
 * Tells us if this variable is static or not
 */
int variable_is_static(variable* var);

/**
 * Copies the definition of the given variable, but does not take over the memory area of it, instead
 * reserves its own copy.
 */
variable* variable_copy(const variable* src);

/**
 * Sets the given environment variable
 */
// char* set_env_var(char* name, envelope* new_value);


#endif
