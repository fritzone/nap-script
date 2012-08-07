#ifndef _BSD_EXTR_H_
#define _BSD_EXTR_H_


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

#endif
