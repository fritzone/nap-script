#ifndef _S_H_
#define _S_H_

/*
 * increment/decrement, index, equal operators.
 */
extern const char *STR_MINMIN;
extern const char *STR_MINUS;
extern const char *STR_PLUSPLUS;
extern const char *STR_PLUS;
extern const char *STR_IDXID;
extern const char *STR_EQUAL;
extern const char *STR_SQPAR_OP;
extern const char *STR_SQPAR_CL;
extern const char *STR_MUL;
extern const char *STR_DIV;
extern const char *STR_MOD;
extern const char *STR_OPEN_BLOCK;
extern const char *STR_CLOSE_BLOCK;
extern const char *STR_DOT;
extern const char *STR_COLON;
/*
 * Comparison operators
 */
extern const char *STR_EQUALEQUAL;
extern const char *STR_LT;
extern const char *STR_GT;
extern const char *STR_LTE;
extern const char *STR_GTE;
extern const char *STR_NEQ;

/*
 * Operators as char constants
 */
const char C_ADD            = '+';
const char C_SUB            = '-';
const char C_DIV            = '/';
const char C_MUL            = '*';
const char C_MOD            = '%';
const char C_EQ                = '=';
const char C_DOLLAR            = '$';
const char C_COMMA            = ',';
const char C_OPEN_BLOCK        = '{';
const char C_CLOSE_BLOCK    = '}';
const char C_SEMC            = ';';

/*
 * Bitwise operators as characters
 */
const char C_AND            = '&';
const char C_OR                = '|';
const char C_XOR            = '^';
const char C_NOT            = '!';
const char C_COMP            = '~';

/*
 * Logical operators
 */
extern const char *STR_LOGIC_AND;
extern const char *STR_LOGIC_OR;
extern const char *STR_LOGIC_NOT;

/*
 * Bitwise operators
 */
extern const char *STR_BIT_AND;
extern const char *STR_BIT_OR;
extern const char *STR_BIT_XOR;
extern const char *STR_BIT_COMP;
extern const char *STR_SHLEFT;
extern const char *STR_SHRIGHT;

/*
 * Parenthesis, others
 */
const char C_PAR_OP            = '(';
const char C_PAR_CL            = ')';
const char C_SQPAR_OP          = '[';
const char C_BRACKET_OP        = '{';
const char C_SQPAR_CL          = ']';
const char C_LT                = '<';
const char C_GT                = '>';
const char C_QUOTE             = '"';
const char C_SQUOTE            = '\'';
const char C_BACKQUOTE         = '`';
const char C_BACKSLASH         = '\\';
const char C_SPACE             = ' ';
const char C_HASH              = '#';
const char C_SLASH             = C_DIV;
const char C_STAR              = C_MUL;
const char C_NEWLINE           = '\n';
const char C_CARRET            = '\r';
const char C_TAB               = '\t';
const char C_UNDERLINE         = '_';

/*
 * x= operators (+=, -=, etc ...)
 */
extern const char *STR_PLUS_EQUAL;
extern const char *STR_MINUS_EQUAL;
extern const char *STR_MUL_EQUAL;
extern const char *STR_DIV_EQUAL;
extern const char *STR_AND_EQUAL;
extern const char *STR_OR_EQUAL;
extern const char *STR_SHL_EQUAL;
extern const char *STR_SHR_EQUAL;
extern const char *STR_COMP_EQUAL;
extern const char *STR_XOR_EQUAL;

extern const char *STR_RETURN;
extern const char *STR_IF;
extern const char *STR_ELSE;
extern const char *STR_WHILE;
extern const char *STR_BREAK;
extern const char *STR_CONTINUE;
extern const char *STR_FOR;
extern const char *STR_EXTERN;
extern const char *STR_USE;
extern const char *STR_NEW;

extern const char *STR_GLOBAL;


/*
 * Numerical constants
 */
const int MAX_IDENTIFIER_LENGTH            = 1024;

const int DEF_INTERN                           = 0; // a normal method
const int DEF_EXTERN                           = 1; // an "extern" method defined in native code/library
// a method defined in the VM chain. Used when compiling a code used by the runtime
// when calling a method from a VM via nap_runtime_call_method
const int DEF_VMCHAIN                          = 2;

const int MAX_NR_AS_STR_LEN            = 32;

const int NO_INDEX                = -1;

/*
 * Differrent method calls
 */
const int METHOD_CALL_NORMAL            = 0;
const int METHOD_CALL_CONSTRUCTOR       = 1;
const int METHOD_CALL_OF_OBJECT         = 2;
const int METHOD_CALL_STATIC_OF_CLASS   = 3; // do not use 4,5
const int METHOD_CALL_SPECIAL_EXECUTE   = 6;
const int METHOD_CALL_SPECIAL_PRINT     = 7;

/*
 * Error message strings
 */

extern const char *const E0001_NODEFVAL;
extern const char *const E0002_DIVBY0;
extern const char *const E0003_UNDEFOPRTR;
extern const char *const E0004_UNDEFUNOPR;
extern const char *const E0005_NOCOMP1;
extern const char *const E0006_NOCOMP2;
extern const char *const E0007_INVLDINDX;
extern const char *const E0008_INVLDINDX;
extern const char *const E0009_PARAMISM;
extern const char *const E0010_INVFNCDF;
extern const char *const E0011_IDDEFERR;
extern const char *const E0012_SYNTAXERR;
extern const char *const E0013_INVEXPR;
extern const char *const E0014_UNSINDX;
extern const char *const E0015_MIDXINTRQ;
extern const char *const E0016_UNSUPOPP;
extern const char *const E0017_CHAINASSIGN;
extern const char *const E0018_INVIDENT;
extern const char *const E0019_NOINITVCPXT;
extern const char *const E0020_ACCTNOTP;
extern const char *const E0021_NOPREINCLV;
extern const char *const E0022_NOPOSTINCLV;
extern const char *const E0023_UNKNOWNTYPE;
extern const char *const E0024_DBLINDXINVLD;
extern const char *const E0025_QMISM;
extern const char *const E0026_TOOMANYIDX;
extern const char *const E0027_NOTENOUGHIDX;
extern const char *const E0028_CANNOTASSIGN;
extern const char *const E0029_PARANOTALL;
extern const char *const E0030_PARMNOTALL;
extern const char *const E0031_NOREFHERE;
extern const char *const E0032_NOARRHERE;
extern const char *const E0033_INCORRDEF;
extern const char *const E0034_SYMBOLDEFD;
extern const char *const E0035_NOOP;
extern const char *const E0036_NOBREAK;
extern const char *const E0037_INV_IDENTF;
extern const char *const E0038_DYNDIMNALL;


extern const char *const STR_CALL_CONTEXT_SEPARATOR;
extern const char *const STR_UNNAMED_CC;

extern const char *STR_INVALID_DIMENSION;
extern const char *STR_INDEX_OUT_OF_RANGE;
extern const char *STR_INTERNAL_ERROR;
extern const char *STR_UNDEFINED_VARIABLE;

/* the width of the real number when formatted int oa string */
extern const char *STR_REAL_FORMAT;

/* the string representatives of the basic types */
extern const char *STR_INT;
extern const char *STR_BYTE;
extern const char *STR_REAL;
extern const char *STR_STRING;
extern const char *STR_VOID;
extern const char *STR_CHAR;
extern const char *STR_BOOL;
extern const char *STR_NULL;

extern const char *STR_UNKNOWN;

extern const char *keywords[15];
extern char *EMPTY;

/* strings for reserved commands */
extern const char* STR_NAP_EXECUTE;
extern const char* STR_NAP_PRINT;
extern const char* STR_NAP_EXTERN;

#endif
