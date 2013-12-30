#ifndef NAP_CONSTS_H
#define NAP_CONSTS_H

/** Means the execution was succesful */
static const int NAP_EXECUTE_SUCCESS = 1;

/** Means the execution was not successful */
static const int NAP_EXECUTE_FAILURE = 0;

/** Means the requested variable was found */
static const int NAP_VARIABLE_FOUND     = 1;

/** Means the requested variable was not found */
static const int NAP_VARIABLE_NOT_FOUND = 0;

/** Represents that there is no such value*/
static const int NAP_NO_VALUE = 0x0BADF00D;

/* Success and failure indicators */
static const int NAP_SUCCESS = 1;
static const int NAP_FAILURE = 0;

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

#endif // NAP_CONSTS_H
