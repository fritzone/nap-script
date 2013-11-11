#include "consts.h"

#include "type.h"

const char* STR_MINMIN        = "--";
const char* STR_MINUS        = "-";
const char* STR_PLUSPLUS    = "++";
const char* STR_PLUS        = "+";
const char* STR_IDXID        = "[]";
const char* STR_EQUAL        = "=";
const char* STR_SQPAR_OP    = "[";
const char* STR_SQPAR_CL    = "]";
const char* STR_MUL        = "*";
const char* STR_DIV        = "/";
const char* STR_MOD        = "%";
const char* STR_OPEN_BLOCK    = "{";
const char* STR_CLOSE_BLOCK    = "}";
const char* STR_DOT             = ".";
const char* STR_COLON           = ":";

/*
 * Comparison operators
 */
const char* STR_EQUALEQUAL    = "==";
const char* STR_LT        = "<";
const char* STR_GT        = ">";
const char* STR_LTE        = "<=";
const char* STR_GTE        = ">=";
const char* STR_NEQ        = "!=";

/*
 * Logical operators
 */
const char* STR_LOGIC_AND    = "&&";
const char* STR_LOGIC_OR    = "||";
const char* STR_LOGIC_NOT    = "!";

/*
 * Bitwise operators
 */
const char* STR_BIT_AND        = "&";
const char* STR_BIT_OR        = "|";
const char* STR_BIT_XOR        = "^";
const char* STR_BIT_COMP    = "~";
const char* STR_SHLEFT        = "<<";
const char* STR_SHRIGHT        = ">>";

/*
 * x= operators (+=, -=, etc ...)
 */
const char* STR_PLUS_EQUAL    = "+=";
const char* STR_MINUS_EQUAL    = "-=";
const char* STR_MUL_EQUAL    = "*=";
const char* STR_DIV_EQUAL    = "/=";
const char* STR_AND_EQUAL    = "&=";
const char* STR_OR_EQUAL    = "|=";
const char* STR_SHL_EQUAL    = "<<=";
const char* STR_SHR_EQUAL    = ">>=";
const char* STR_COMP_EQUAL    = "~=";
const char* STR_XOR_EQUAL    = "^=";

const char* STR_RETURN        = "return";
const char* STR_IF        = "if";
const char* STR_ELSE        = "else";
const char* STR_WHILE        = "while";
const char* STR_BREAK        = "break";
const char* STR_CONTINUE    = "continue";
const char* STR_FOR        = "for";
const char* STR_EXTERN        = "extern";
const char* STR_USE        = "use";
const char* STR_NEW             = "new";

const char* STR_GLOBAL        = "global";

/*
 * Error message strings
 */

const char* const E0001_NODEFVAL    = "[E0001] No value specified and no initial value for parameter";
const char* const E0002_DIVBY0        = "[E0002] Division by zero";
const char* const E0003_UNDEFOPRTR    = "[E0003] Undefined binary operator";
const char* const E0004_UNDEFUNOPR    = "[E0004] Undefined unary operator";
const char* const E0005_NOCOMP1        = "[E0005] Cannot compare values: [real] with [character]";
const char* const E0006_NOCOMP2        = "[E0006] Cannot compare these complex elements";
const char* const E0007_INVLDINDX    = "[E0007] Invalid index";
const char* const E0008_INVLDINDX    = "[E0008] Invalid indexed element";
const char* const E0009_PARAMISM    = "[E0009] Parentheses mismatch in";
const char* const E0010_INVFNCDF    = "[E0010] Invalid function definition in statement: ";
const char* const E0011_IDDEFERR    = "[E0011] Index definition error";
const char* const E0012_SYNTAXERR    = "[E0012] Syntax error in";
const char* const E0013_INVEXPR        = "[E0013] Invalid Expression";
const char* const E0014_UNSINDX        = "[E0014] Unsupported index type";
const char* const E0015_MIDXINTRQ    = "[E0015] Multi index is not an integer value";
const char* const E0016_UNSUPOPP    = "[E0016] Unsupported operation";
const char* const E0017_CHAINASSIGN    = "[E0017] Chain assignment is not supported";
const char* const E0018_INVIDENT    = "[E0018] Invalid identifier";
const char* const E0019_NOINITVCPXT    = "[E0019] Initial value not supported for complex types";
const char* const E0020_ACCTNOTP    = "[E0020] Variable accessed as templated but has no template definition";
const char* const E0021_NOPREINCLV    = "[E0021] Cannot pre increment a non lvalue";
const char* const E0022_NOPOSTINCLV    = "[E0022] Cannot post increment a non lvalue";
const char* const E0023_UNKNOWNTYPE    = "[E0023] Undefined type";
const char* const E0024_DBLINDXINVLD    = "[E0024] Invalid indexing operation";
const char* const E0025_QMISM        = "[E0025] Double Quotes mismatch";
const char* const E0026_TOOMANYIDX    = "[E0026] Too many indexes...";
const char* const E0027_NOTENOUGHIDX    = "[E0027] Not enough indexes...";
const char* const E0028_CANNOTASSIGN    = "[E0028] Cannot fulfil this assignment";
const char* const E0029_PARANOTALL    = "[E0029] Parantheses are not allowed here";
const char* const E0030_PARMNOTALL    = "[E0030] Parameters not allowed for this type of variable";
const char* const E0031_NOREFHERE    = "[E0031] Reference parameters not allowed here";
const char* const E0032_NOARRHERE    = "[E0032] Arrays are not allowed here";
const char* const E0033_INCORRDEF    = "[E0033] Incorrect variable template parameter definition";
const char* const E0034_SYMBOLDEFD    = "[E0034] Symbol already defined";
const char* const E0035_NOOP        = "[E0035] Cannot make an operation with these complex elements";
const char* const E0036_NOBREAK        = "[E0036] Cannot use break/continue here";
const char* const E0037_INV_IDENTF    = "[E0037] Invalid identifier name";
const char* const E0038_DYNDIMNALL    = "[E0038] Dynamic dimensions are not allowed for multidimension arrays";


const char* const STR_CALL_CONTEXT_SEPARATOR = ".";
const char* const STR_UNNAMED_CC = "unnamed";

const char* STR_INVALID_DIMENSION    = "Invalid dimension for a variable being created";
const char* STR_INDEX_OUT_OF_RANGE    = "Index [%d] out of range for";
const char* STR_INTERNAL_ERROR        = "Internal error";
const char* STR_UNDEFINED_VARIABLE    = "Undefined variable";

/* the width of the real number when formatted int oa string */
const char* STR_REAL_FORMAT    =    "%.8G";

/* the string representatives of the basic types */
const char* STR_INT                = "int";
const char* STR_REAL                = "real";
const char* STR_STRING                = "string";
const char* STR_CHAR                            = "char";
const char* STR_BOOL                            = "bool";
const char* STR_NULL                            = "null";

const char* STR_UNKNOWN                = "unknown";

m_optype NO_OPERATOR = -1;

const char* keywords[] =
{
    STR_RETURN,
    STR_IF,
    STR_ELSE,
    STR_WHILE,
    STR_BREAK,
    STR_CONTINUE,
    STR_FOR,
    STR_EXTERN,
    STR_USE,
    STR_GLOBAL,
    STR_INT,
    STR_REAL,
    STR_STRING,
    STR_NULL
};

char* EMPTY = (char*)"";
