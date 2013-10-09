#ifndef _OPCODES_H_
#define _OPCODES_H_

static const unsigned char OPCODE_INT           = 0x01;
static const unsigned char OPCODE_FLOAT         = 0x02;
static const unsigned char OPCODE_STRING        = 0x03;
static const unsigned char OPCODE_CHAR          = 0x04;
static const unsigned char OPCODE_BYTE          = 0x08;
static const unsigned char OPCODE_SUB           = 0x8B;
static const unsigned char OPCODE_SHORT         = 0x16;
static const unsigned char OPCODE_LONG          = 0x32;
static const unsigned char OPCODE_HUGE          = 0x64;
static const unsigned char OPCODE_IMMEDIATE     = 0x11;
static const unsigned char OPCODE_INC           = 0x1C;
static const unsigned char OPCODE_MUL           = 0xA8;
static const unsigned char OPCODE_MOD           = 0xA9;
static const unsigned char OPCODE_ADD           = 0xAD;
static const unsigned char OPCODE_CLRS_NAME     = 0xB6;
static const unsigned char OPCODE_MARKS_NAME    = 0xB7;
static const unsigned char OPCODE_MOV           = 0xB8;
static const unsigned char OPCODE_JMP           = 0xBA;
static const unsigned char OPCODE_MARKS         = 0xBB;
static const unsigned char OPCODE_PUSH          = 0xBC;
static const unsigned char OPCODE_CLRS          = 0xBD;
static const unsigned char OPCODE_JLBF          = 0xBF;
static const unsigned char OPCODE_CALL          = 0xCA;
static const unsigned char OPCODE_POP           = 0xCB;
static const unsigned char OPCODE_PEEK          = 0xCD;
static const unsigned char OPCODE_DIV           = 0xD8;
static const unsigned char OPCODE_EQ            = 0xE0;
static const unsigned char OPCODE_NEQ           = 0xE1;
static const unsigned char OPCODE_LT            = 0xE2;
static const unsigned char OPCODE_GT            = 0xE3;
static const unsigned char OPCODE_LTE           = 0xE4;
static const unsigned char OPCODE_GTE           = 0xE5;
static const unsigned char OPCODE_REG           = 0xEE;
static const unsigned char OPCODE_REF           = 0xEF;
static const unsigned char OPCODE_RETURN        = 0xFD;
static const unsigned char OPCODE_VAR           = 0xFE;
static const unsigned char OPCODE_EXIT          = 0xFF;
#endif
