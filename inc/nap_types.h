#ifndef _NAP_TYPES_H_
#define _NAP_TYPES_H_

#include <stdint.h>

/* types for manipulating the addresses, indexes, etc */
typedef uint32_t nap_addr_t;    /* the type of a NAP address*/
typedef uint32_t nap_mark_t;    /* the type of a marker pushed on the stack */
typedef uint32_t nap_index_t;   /* the type of an index */

/* nap builtin types */
typedef int64_t  nap_int_t;     /* the NAP type of an integer number */
typedef double   nap_real_t;    /* the NAP type of a real number */
typedef char*    nap_string_t;  /* the NAP type of a string */
typedef uint8_t  nap_byte_t;    /* the NAP type of a string */

#endif
