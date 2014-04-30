#ifndef _NAP_TYPES_H_
#define _NAP_TYPES_H_

#include <stdint.h>

/**
 * @file nap_types.h
 *
 * The file contains the definition of the C/C++ types which are to be used for 
 * interaction with the nap runtime.
 */

/** @defgroup NapTypes C/C++ types 
 * 
 * The mapping of nap script types to C types. In order to properly integrate your
 * C/C++ code with the nap runtime use these types. They are just typedefs, however 
 * the nap runtime was compiled with these in mind, so in order to avoid different
 * sizes for the same nap script type you have to use these data types.
 * 
 * @{ 
 */

/** 
 * The corresponding C type of a nap-script "int" type. This is a 64 bit signed 
 * integer.
 */
typedef int64_t  nap_int_t;

/* the NAP type of a real number */
typedef double   nap_real_t;

/* the NAP type of a string */
typedef char*    nap_string_t;

/* the NAP type of a byte */
typedef uint8_t  nap_byte_t;

/** @} */

#endif
