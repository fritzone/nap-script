#include "tests.h"
#include "gtest/gtest.h"

/*
 * This file has the tests that are related to code execution in a
 * virtual machine as triggered by the "outsisde" world (ie: C runtime)
 */

/* Have some code in a VM.
 * Execute some other code in the VM.
 */
TEST(CodeExecution, ExecuteSomeCodeInAVm)
{
    nap_runtime* runtime = nap_runtime_create(0);
    int found_indicator = 0;

    ASSERT_FALSE(runtime == NULL);

    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
    "int c = 0;                       \
     int a = 1;                       \
     int b = 2;                       \
    "
    ,0);

    ASSERT_FALSE(bytecode == NULL);
    nap_runtime_execute(runtime, bytecode);
    nap_execute_code(runtime, "c = a + b");
    ASSERT_EQ(3, VAR_INT(c));

    nap_runtime_shutdown(&runtime);
    ASSERT_TRUE(runtime == NULL);
}


/* Define a function. Call it via the runtime API
 * Use the function, see that it returns the default return value (0).
 */
TEST(CodeExecution, ExternalCallingOfInternalMethod)
{
    nap_runtime* runtime = nap_runtime_create(0);
    int found_indicator = 0;

    ASSERT_FALSE(runtime == NULL);

    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
    "int c = 0;                       \
     int some_fun(int a , int b)      \
     {                                \
         c = a + b;                   \
     }" ,0);

    ASSERT_FALSE(bytecode == NULL);
    nap_runtime_execute(runtime, bytecode);
    nap_int_t p1 = 1;
    nap_int_t p2 = 2;
    nap_execute_method(runtime, 0, "some_fun", p1, p2);
    ASSERT_EQ(3, VAR_INT(c));

    nap_runtime_shutdown(&runtime);
    ASSERT_TRUE(runtime == NULL);
}


/* Define a function. Call it via the runtime API
 * Use the function, see that it returns the default return value (0).
 */
TEST(CodeExecution, ExternalCallingOfInternalMethodWithIntReturnType)
{
    nap_runtime* runtime = nap_runtime_create(0);
    int found_indicator = 0;

    ASSERT_FALSE(runtime == NULL);

    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
    "int c = 0;                       \
     int some_fun(int a , int b)      \
     {                                \
         c = a + b;                   \
         return c;                    \
     }" ,0);

    ASSERT_FALSE(bytecode == NULL);
    nap_runtime_execute(runtime, bytecode);
    nap_int_t p1 = 1;
    nap_int_t p2 = 2;
    nap_int_t ret = 0;
    nap_execute_method(runtime, &ret, "some_fun", p1, p2);
    ASSERT_EQ(3, VAR_INT(c));
    ASSERT_EQ(3, ret);

    nap_runtime_shutdown(&runtime);
    ASSERT_TRUE(runtime == NULL);
}

/* Define a function. Call it via the runtime API
 * Use the function, see that it returns the default return value (0).
 */
TEST(CodeExecution, ExternalCallingOfInternalMethodWithStringReturnType)
{
    nap_runtime* runtime = nap_runtime_create(0);
    int found_indicator = 0;

    ASSERT_FALSE(runtime == NULL);

    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
    "string c = \"ABC\";                      \
     string some_fun(string a, string b)      \
     {                                        \
         c = a + b;                           \
         return c;                            \
     }" ,0);

    ASSERT_FALSE(bytecode == NULL);
    nap_runtime_execute(runtime, bytecode);
    nap_string_t p1 = (nap_string_t)"DEF";
    nap_string_t p2 = (nap_string_t)"GHI";
    nap_string_t ret = 0;
    nap_execute_method(runtime, &ret, "some_fun", p1, p2);
    char *c = VAR_STRING(c);
    ASSERT_STREQ("DEFGHI", c);
    ASSERT_STREQ("DEFGHI", ret);
    free(ret);
    free(c);

    nap_runtime_shutdown(&runtime);
    ASSERT_TRUE(runtime == NULL);
}
