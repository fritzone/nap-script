#ifndef TESTS_H
#define TESTS_H

#define UNUSED(x) (void)x;

#include "nap_runtime.h"
#include "utils.h"

/*
 * Macros for making the life of the test writer easier
 */

#define SCRIPT_START \
    nap_runtime* runtime = nap_runtime_create(0);                  \
    ASSERT_FALSE(runtime == NULL);                                 \
    int found_indicator;                                           \
    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,

#define SCRIPT_END \
    ,0);                                                             \
    ASSERT_FALSE(bytecode == NULL);                                \
    int t = nap_runtime_execute(runtime, bytecode);                \
    ASSERT_EQ(1, t);

#define VAR_INT(a)      nap_runtime_get_int(runtime, #a, &found_indicator)

#define VAR_REAL(a)     nap_runtime_get_real(runtime, #a, &found_indicator)

#define VAR_BYTE(a)     nap_runtime_get_byte(runtime, #a, &found_indicator)

#define VAR_STRING(a)   nap_runtime_get_string(runtime, #a, &found_indicator)

#define SCRIPT_SHUTDOWN \
    nap_runtime_shutdown(&runtime);                                \
    ASSERT_TRUE(runtime == NULL);


#ifdef _WINDOWS
	#pragma warning(disable: 4127)
#endif

#define SCRIPT_ASSERT_STREQ(with,what)                             \
    do {                                                           \
        char* what = VAR_STRING(what);                             \
        ASSERT_STREQ(with, what);                                  \
        free(what);                                                \
    } while(0);

#endif // TESTS_H
