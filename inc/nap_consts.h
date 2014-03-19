#ifndef NAP_CONSTS_H
#define NAP_CONSTS_H

#include <stdint.h>

/** Means the execution was succesful */
static const int NAP_EXECUTE_SUCCESS = 1;

/** Means the execution was not successful */
static const int NAP_EXECUTE_FAILURE = 0;

/** Means the requested variable was found */
static const int NAP_VARIABLE_FOUND     = 1;

/** Means the requested variable was not found */
static const int NAP_VARIABLE_NOT_FOUND = 0;

/** Represents that there is no such value*/
static const int NAP_NO_VALUE = 0xFF;

/* Success and failure indicators */
static const int NAP_FAILURE                      = 0;
static const int NAP_SUCCESS                      = 1;

/* Error codes for intr 2 */
static const uint16_t INTR_2_CANNOT_SET_SOURCE         = 0x0201;
static const uint16_t INTR_2_CANNOT_COMPILE_SOURCE     = 0x0202;

/* Error codes for intr 3 */
static const uint16_t INTR_3_CANNOT_CREATE_VM          = 0x0301;
static const uint16_t INTR_3_COULD_NOT_RUN_CODE        = 0x0302;

/* Error codes for intr 4 */
static const uint16_t INTR_4_CANNOT_LOAD_LIBRARY       = 0x0401;
static const uint16_t INTR_4_CANNOT_LOAD_FUNCTION      = 0x0402;

/* Constants for the data type codes */
static const char TYPE_INT    = 'i';
static const char TYPE_BYTE   = 'b';
static const char TYPE_REAL   = 'r';
static const char TYPE_STRING = 's';

/* Errors for index handling in mov.c*/
static const int INVALID_INDEX_COUNT = -1;
static const int INDEX_OUT_OF_RANGE  = -2;

/* the error codes for the VM*/
static const int ERR_VM_0001 = 0x01; /* cannot allocate enough memory error */
static const int ERR_VM_0002 = 0x02; /* stack underflow error */
static const int ERR_VM_0003 = 0x03; /* cannot allocate a VM in interrupt 3*/
static const int ERR_VM_0004 = 0x04; /* interrupt implementation not found */
static const int ERR_VM_0005 = 0x05; /* eq works only on register/variable */
static const int ERR_VM_0006 = 0x06; /* internal stack overflow/too deep recursion*/
static const int ERR_VM_0007 = 0x07; /* internal stack empty, cannot popall */
static const int ERR_VM_0008 = 0x08; /* internal: a variable is not found */
static const int ERR_VM_0009 = 0x09; /* cannot decrement something */
static const int ERR_VM_0010 = 0x0A; /* cannot increment something */
static const int ERR_VM_0011 = 0x0B; /* cannot execute a mov operation */
static const int ERR_VM_0012 = 0x0C; /* cannot execute an arithemtic operation */
static const int ERR_VM_0013 = 0x0D; /* cannot peek the stack */
static const int ERR_VM_0014 = 0x0E; /* cannot pop from the stack */
static const int ERR_VM_0015 = 0x0F; /* cannot push onto the stack */
static const int ERR_VM_0016 = 0x10; /* cannot create a marker object */
static const int ERR_VM_0017 = 0x11; /* unimplemented interrupt */
static const int ERR_VM_0018 = 0x12; /* variable was not initialized correctly */
static const int ERR_VM_0019 = 0x13; /* too deep recursion when calling */
static const int ERR_VM_0020 = 0x14; /* invalid jump index */
static const int ERR_VM_0021 = 0x15; /* cannot leave when there are no more call frame */
static const int ERR_VM_0022 = 0x16; /* invalid internal call */
static const int ERR_VM_0023 = 0x17; /* division by zero */

/* the character conversion multiplier when converting to UTF-32BE*/
static const int CC_MUL = 4;

#endif

