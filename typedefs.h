#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

// #define max(a,b) ((a),b))?(a):(b)


///* prototype for a comparison function which calls something else ... :) */
//typedef struct envelope* (*CMP_FUNCTION)(struct envelope* lvalue, struct envelope* rvalue, int lvalue_index, int rvalue_index);

///* function type for the PRE/POST increment operations */
//typedef struct envelope* (*PREPOST_OPERATIONS)(struct variable*, int) ;

///* the generic funtion of a basic operation */
//typedef struct envelope* (*BASIC_OP_TWO_NUMBERS)(const number* const nr1,const number* const nr2, int reqd_type, int op);

///* operator function prototypes definition */
//typedef struct envelope* (*BASIC_OP_NUMBER_NUMBER)( struct number*  ,  struct number* , int, BASIC_OP_TWO_NUMBERS, int) ;
//typedef struct envelope* (*BASIC_OP_NUMBER_VARIABLE)( struct number*  ,  struct variable* , int, BASIC_OP_TWO_NUMBERS, int) ;
//typedef struct envelope* (*BASIC_OP_VARIABLE_NUMBER)( struct variable*  ,  struct number* , int, BASIC_OP_TWO_NUMBERS, int) ;
//typedef struct envelope* (*BASIC_OP_VARIABLE_VARIABLE)( struct variable*  ,  struct variable* , int, int, BASIC_OP_TWO_NUMBERS, int) ;

///* operations with strings */
///* function which takes a number and a string and returns an envelope containing a string object. Also it takes the operation*/
//typedef struct envelope* (STRING_OP_NUMBER_STRING)( struct number* ,  struct bt_string* , int);

///* a structure type which holds the four basic type of operator functions */
//struct basic_op_functions
//{
//	BASIC_OP_VARIABLE_NUMBER op_var_nr;
//	BASIC_OP_NUMBER_VARIABLE op_nr_var;
//	BASIC_OP_VARIABLE_VARIABLE op_var_var;
//};


#endif
