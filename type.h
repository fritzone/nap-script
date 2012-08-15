#ifndef _TYPE_H_
#define _TYPE_H_

/* the typeid type, just a simple int, used to identify the types found in leaf nodes */
typedef int m_typeid;

/* operator identification */
typedef int m_optype;

/*
 * TypeIDs
 */
static const int BASIC_TYPE_DONTCARE		= 0;

/*  be careful that BASIC_TYPE_REAL always should be bigger than BASIC_TYPE_INT
    BASIC_TYPE_CHAR should be bigger than BASIC_TYPE_BOOL and so on */

static const int BASIC_TYPE_BOOL			= 1;
static const int BASIC_TYPE_CHAR			= 2;
static const int BASIC_TYPE_INT				= 3;
static const int BASIC_TYPE_REAL			= 4;
static const int BASIC_TYPE_STRING			= 5;
static const int BASIC_TYPE_VARIABLE		= 6;
static const int BASIC_TYPE_INDEXED			= 7;
static const int BASIC_TYPE_USERDEF			= 8;
static const int MULTI_DIM_INDEX			= 9;
static const int NT_VARIABLE_DEF_LST		= 10;
static const int RESULT_STRING				= 11;
static const int FUNCTION_DEFINITION		= 12;
static const int FUNCTION_CALL				= 13;
static const int STATEMENT_IF				= 14;
static const int STATEMENT_IF_1L			= 15;
static const int STATEMENT_CLOSE_CC			= 16;
static const int STATEMENT_NEW_CC			= 17;
static const int BASIC_TYPE_VOID			= 18;
static const int BACKQUOTE_STRING			= 19;
static const int TEMPLATED_VARIABLE			= 20;
static const int RETURN_STATEMENT			= 21;
static const int ENVIRONMENT_VARIABLE		= 22;
static const int STATEMENT_WHILE			= 23;
static const int STATEMENT_WHILE_1L			= 24;
static const int STATEMENT_BREAK			= 25;
static const int STATEMENT_CONTINUE			= 26;
static const int STATEMENT_FOR				= 27;
static const int STATEMENT_FOR_1L			= 28;
static const int LIST_VALUE					= 29;
static const int LIST_ELEMENT				= 30;
static const int STATEMENT_DO				= 31;
static const int CLASS_DECLARATION			= 31;

static const int ENV_TYPE_CC				= 100;
/*
 * Operator types
 */
extern m_optype NO_OPERATOR;

/**
 * TypeIDs for common operators with two arguments
 */
enum op_typeids
{
    /* Addition, Substraction */
    OPERATOR_ADD = 100,
    OPERATOR_MINUS,													// 101

    /* unary + - */
    OPERATOR_UNARY_PLUS,											// 102
    OPERATOR_UNARY_MINUS,											// 103

    /* Divide, Multiplication, Modulo */
    OPERATOR_DIVIDE,											// 104
    OPERATOR_MULTIPLY,											// 105
    OPERATOR_MODULO,											// 106

    /* Shift Left, Right */
    OPERATOR_SHIFT_LEFT,											// 107
    OPERATOR_SHIFT_RIGHT,											// 108

    /* And, Or, Xor, Comp */
    OPERATOR_BITWISE_AND,											// 109
    OPERATOR_BITWISE_OR,											// 110
    OPERATOR_BITWISE_XOR,											// 111
    OPERATOR_BITWISE_COMP,											// 112
    OPERATOR_NOT,													// 113

    /* Assignment */
    OPERATOR_ASSIGN,											// 114

    /* Pre/Post-increment/Decrement */
    OPERATOR_POSTINC,
    OPERATOR_POSTDEC,
    OPERATOR_PREINC,
    OPERATOR_PREDEC,

    /* +=, -=, etc...*/
    OPERATOR_PLUS_EQUAL,
    OPERATOR_MINUS_EQUAL,
    OPERATOR_MUL_EQUAL,
    OPERATOR_DIV_EQUAL,
    OPERATOR_AND_EQUAL,
    OPERATOR_OR_EQUAL,
    OPERATOR_XOR_EQUAL,
    OPERATOR_SHL_EQUAL,
    OPERATOR_SHR_EQUAL,

    /* the last operator is not used */
    OPERATOR_LAST
};

/**
 * The TypeIDs for comparison operators
 */
enum cmp_typeids
{
    COMP_START_IDX = 150,
    COMP_EQUALEQUAL,
    COMP_NEQ,
    COMP_LT,
    COMP_GT,
    COMP_LTE,
    COMP_GTE,

    COMP_LAST
};

/**
 * Returns the size of the default types, later it will include the sizes of different user objects
 */
int get_size(const char* type);

/**
 * Returns the type identifier for the given type
 */
int get_typeid(const char* type);

/**
 * Returns the description of the given type_id'd element
 */
const char* get_desc(int type_id);

#endif
