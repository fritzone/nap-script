#include "stack.h"

/* variables for the stack */
static struct stack_entry** stack = NULL;           /* in this stack */
static uint64_t stack_size = STACK_INIT;            /* initial stack size */
static int64_t stack_pointer = -1;                  /* the stack pointer */
