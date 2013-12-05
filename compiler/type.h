#ifndef _TYPE_H_
#define _TYPE_H_

#include <stdint.h>
#include <string>

typedef uint32_t NUMBER_INTEGER_TYPE;

typedef double NUMBER_REAL_TYPE ;

/* operator identification */
typedef int m_optype;

/*
 * TypeIDs
 */
static const int BASIC_TYPE_DONTCARE        = 0;

static const int BASIC_TYPE_BOOL            = 10;
static const int BASIC_TYPE_INT             = 1; // same as in the opcodes
static const int BASIC_TYPE_REAL            = 2; // same as in the opcodes
static const int BASIC_TYPE_STRING          = 3; // same as in the opcodes
static const int BASIC_TYPE_CHAR            = 4; // same as in the opcodes
static const int BASIC_TYPE_VARIABLE        = 60;
static const int BASIC_TYPE_EXTERN_VARIABLE = 61;
static const int BASIC_TYPE_CLASS_VAR       = 65;
static const int BASIC_TYPE_INDEXED         = 70;
static const int BASIC_TYPE_USERDEF         = 80;
static const int MULTI_DIM_INDEX            = 90;
static const int NT_VARIABLE_DEF_LST        = 100;
static const int RESULT_STRING              = 110;
static const int FUNCTION_DEFINITION        = 120;
static const int FUNCTION_CALL              = 130;
static const int FUNCTION_CALL_CONSTRUCTOR  = 131;
static const int FUNCTION_CALL_OF_OBJECT    = 132;
static const int FUNCTION_CALL_STATIC       = 133;
static const int MEMBER_ACCESS_OF_OBJECT    = 134;
static const int FUNCTION_STRING_LEN        = 135;
static const int FUNCTION_CALL_NAP_EXEC     = 136;
static const int STATEMENT_IF               = 140;
static const int STATEMENT_IF_1L            = 150;
static const int STATEMENT_CLOSE_CC         = 160;
static const int STATEMENT_NEW_CC           = 170;
static const int BASIC_TYPE_VOID            = 180;
static const int BACKQUOTE_STRING           = 190;
static const int TEMPLATED_VARIABLE         = 200;
static const int RETURN_STATEMENT           = 210;
static const int ENVIRONMENT_VARIABLE       = 220;
static const int STATEMENT_WHILE            = 230;
static const int STATEMENT_WHILE_1L         = 240;
static const int STATEMENT_BREAK            = 250;
static const int STATEMENT_CONTINUE         = 260;
static const int STATEMENT_FOR              = 270;
static const int STATEMENT_FOR_1L           = 280;
static const int LIST_VALUE                 = 290;
static const int LIST_ELEMENT               = 300;
static const int STATEMENT_DO               = 310;
static const int CLASS_DECLARATION          = 320;
static const int ASM_BLOCK                  = 321;
static const int STATEMENT_NEW              = 330;
static const int KEYWORD_TRUE               = 340;
static const int KEYWORD_FALSE              = 350;
static const int KEYWORD_NULL               = 360;

static const int ENV_TYPE_CC                = 1000;
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
    OPERATOR_ADD = 10000,
    OPERATOR_MINUS,                         // 101

    /* unary + - */
    OPERATOR_UNARY_PLUS,                    // 102
    OPERATOR_UNARY_MINUS,                   // 103

    /* Divide, Multiplication, Modulo */
    OPERATOR_DIVIDE,                        // 104
    OPERATOR_MULTIPLY,                      // 105
    OPERATOR_MODULO,                        // 106

    /* Shift Left, Right */
    OPERATOR_SHIFT_LEFT,                    // 107
    OPERATOR_SHIFT_RIGHT,                   // 108

    /* And, Or, Xor, Comp */
    OPERATOR_BITWISE_AND,                   // 109
    OPERATOR_BITWISE_OR,                    // 110
    OPERATOR_BITWISE_XOR,                   // 111
    OPERATOR_BITWISE_COMP,                  // 112
    OPERATOR_NOT,                           // 113

    /* Assignment */
    OPERATOR_ASSIGN,                        // 114

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
    
    OPERATOR_DOT,

    /* the last operator is not used */
    OPERATOR_LAST
};

/**
 * The TypeIDs for comparison operators
 */
enum cmp_typeids
{
    COMP_START_IDX = 15000,
    COMP_EQUALEQUAL,
    COMP_NEQ,
    COMP_LT,
    COMP_GT,
    COMP_LTE,
    COMP_GTE,

    COMP_LAST
};

/**
 * Returns the type identifier for the given type
 */
int get_typeid(const std::string& type);

#endif
