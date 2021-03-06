#ifndef _OPCODES_H_
#define _OPCODES_H_

static const unsigned char OPCODE_INT           = (unsigned char)0x01; /**/
static const unsigned char OPCODE_REAL          = (unsigned char)0x02;
static const unsigned char OPCODE_STRING        = (unsigned char)0x03;
static const unsigned char OPCODE_BYTE          = (unsigned char)0x04; /**/
static const unsigned char OPCODE_IDX           = (unsigned char)0x05;
static const unsigned char OPCODE_GENERIC       = (unsigned char)0x06; /**/
static const unsigned char OPCODE_IMMEDIATE_INT = (unsigned char)0x11; /**/
static const unsigned char OPCODE_IMMEDIATE_REAL= (unsigned char)0x12; /**/
static const unsigned char OPCODE_SHORT         = (unsigned char)0x16; /**/
static const unsigned char OPCODE_INC           = (unsigned char)0x1C; /**/
static const unsigned char OPCODE_DEC           = (unsigned char)0x1D; /**/
static const unsigned char OPCODE_INTR          = (unsigned char)0x1E;

/* internal functions also have their opcode. TODO: should these be interrupts? */
static const unsigned char OPCODE_CCIDX         = (unsigned char)0x21;
static const unsigned char OPCODE_GROW          = (unsigned char)0x22;
static const unsigned char OPCODE_LEN           = (unsigned char)0x23;
static const unsigned char OPCODE_LONG          = (unsigned char)0x32; /**/

/* SP and BP for peek */
static const unsigned char OPCODE_SP            = (unsigned char)0x50; /**/
static const unsigned char OPCODE_BP            = (unsigned char)0x51; /**/

static const unsigned char OPCODE_SHL           = (unsigned char)0x54; /**/
static const unsigned char OPCODE_SHR           = (unsigned char)0x55; /**/
static const unsigned char OPCODE_AND           = (unsigned char)0x56; /**/
static const unsigned char OPCODE_OR            = (unsigned char)0x57; /**/
static const unsigned char OPCODE_XOR           = (unsigned char)0x58; /**/
static const unsigned char OPCODE_NOT           = (unsigned char)0x59; /**/
static const unsigned char OPCODE_BCOM          = (unsigned char)0x60; /**/
static const unsigned char OPCODE_NEG           = (unsigned char)0x61; /**/
static const unsigned char OPCODE_POS           = (unsigned char)0x62; /**/
static const unsigned char OPCODE_HUGE          = (unsigned char)0x64; /**/
static const unsigned char OPCODE_SUB           = (unsigned char)0x8B; /**/
static const unsigned char OPCODE_STORE         = (unsigned char)0x90; /**/
static const unsigned char OPCODE_RESTORE       = (unsigned char)0x91; /**/
static const unsigned char OPCODE_MUL           = (unsigned char)0xA8; /**/
static const unsigned char OPCODE_MOD           = (unsigned char)0xA9; /**/
static const unsigned char OPCODE_ADD           = (unsigned char)0xAD; /**/
static const unsigned char OPCODE_CLRS_NAME     = (unsigned char)0xB6; /**/
static const unsigned char OPCODE_MARKS_NAME    = (unsigned char)0xB7; /**/
static const unsigned char OPCODE_MOV           = (unsigned char)0xB8; /**/
static const unsigned char OPCODE_JMP           = (unsigned char)0xBA; /**/
static const unsigned char OPCODE_PUSH          = (unsigned char)0xBC; /**/
static const unsigned char OPCODE_PUSHALL       = (unsigned char)0xBD; /**/
static const unsigned char OPCODE_JLBF          = (unsigned char)0xBF; /**/
static const unsigned char OPCODE_CREA          = (unsigned char)0xC7; /**/
static const unsigned char OPCODE_POKE          = (unsigned char)0xC8; /**/
static const unsigned char OPCODE_SERVE         = (unsigned char)0xC9; /**/
static const unsigned char OPCODE_CALL          = (unsigned char)0xCA; /**/
static const unsigned char OPCODE_POP           = (unsigned char)0xCB; /**/
static const unsigned char OPCODE_CALL_INT      = (unsigned char)0xCC; /**/
static const unsigned char OPCODE_PEEK          = (unsigned char)0xCD; /**/
static const unsigned char OPCODE_CLIDX         = (unsigned char)0xCE;
static const unsigned char OPCODE_POPALL        = (unsigned char)0xCF;
static const unsigned char OPCODE_CLBF          = (unsigned char)0xD7; /**/
static const unsigned char OPCODE_DIV           = (unsigned char)0xD8; /**/
static const unsigned char OPCODE_EQ            = (unsigned char)0xE0; /**/
static const unsigned char OPCODE_NEQ           = (unsigned char)0xE1; /**/
static const unsigned char OPCODE_LT            = (unsigned char)0xE2; /**/
static const unsigned char OPCODE_GT            = (unsigned char)0xE3; /**/
static const unsigned char OPCODE_LTE           = (unsigned char)0xE4; /**/
static const unsigned char OPCODE_GTE           = (unsigned char)0xE5; /**/
static const unsigned char OPCODE_REG           = (unsigned char)0xEE; /**/
static const unsigned char OPCODE_REF           = (unsigned char)0xEF;
static const unsigned char OPCODE_LEAVE         = (unsigned char)0xFB; /**/
static const unsigned char OPCODE_RV            = (unsigned char)0xFC; /**/
static const unsigned char OPCODE_RETURN        = (unsigned char)0xFD; /**/
static const unsigned char OPCODE_VAR           = (unsigned char)0xFE; /**/
static const unsigned char OPCODE_EXIT          = (unsigned char)0xFF; /**/
#endif
