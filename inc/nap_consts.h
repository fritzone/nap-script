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
static const int ERR_VM_0001 = 1; /* cannot allocate enough memory error */
static const int ERR_VM_0002 = 2; /* stack underflow error */
static const int ERR_VM_0003 = 3; /* cannot allocate a VM in interrupt 3*/
static const int ERR_VM_0004 = 4; /* interrupt implementation not found */
static const int ERR_VM_0005 = 5; /* eq works only on register/variable */

#endif // NAP_CONSTS_H
