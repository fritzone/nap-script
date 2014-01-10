#include "nap_runtime.h"
#include "utils.h"
#include "gtest/gtest.h"

/*
 * Macros for making the life easier
 */

#define SCRIPT_START \
    nap_runtime* runtime = nap_runtime_create(0);                  \
    ASSERT_FALSE(runtime == NULL);                                 \
    int found_indicator;                                           \
    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,    
    
#define SCRIPT_END \
    );                                                             \
    ASSERT_FALSE(bytecode == NULL);                                \
    int t = nap_runtime_execute(runtime, bytecode);                \
    ASSERT_EQ(1, t);                                               

#define VAR_INT(a)    nap_runtime_get_int(runtime, #a, &found_indicator)

#define SCRIPT_SHUTDOWN \
    nap_runtime_shutdown(&runtime);                                \
    ASSERT_TRUE(runtime == NULL);

/*
 * TESTS
 */

TEST(VariableDefinitions, SimpleInt)
{
    SCRIPT_START
    "                               \
        int a;                      \
        a = 2;                      \
    "                               
    SCRIPT_END
    
    ASSERT_EQ(2, VAR_INT(a));

    SCRIPT_SHUTDOWN;
}

TEST(Operations, BasicVariableOperations)
{

    SCRIPT_START
    "                               \
        int a;                      \
        int b;                      \
        int a_plus_b;               \
        int a_minus_b;              \
        int a_mul_b;                \
        int a_div_b;                \
        int a_mod_b;                \
        a = 9;                      \
        b = 3;                      \
        a_plus_b = a + b;           \
        a_minus_b = a - b;          \
        a_mul_b = a * b;            \
        a_div_b = a / b;            \
        a_mod_b = a % b;            \
    "                               
    SCRIPT_END

    ASSERT_TRUE( 9 == VAR_INT(a));
    ASSERT_TRUE( 3 == VAR_INT(b));
    ASSERT_TRUE(12 == VAR_INT(a_plus_b));
    ASSERT_TRUE( 6 == VAR_INT(a_minus_b));
    ASSERT_TRUE( 3 == VAR_INT(a_div_b));
    ASSERT_TRUE(27 == VAR_INT(a_mul_b));
    ASSERT_TRUE( 0 == VAR_INT(a_mod_b));

    SCRIPT_SHUTDOWN
}

TEST(Operations, BasicImmediateOperations)
{
    SCRIPT_START
    "                               \
        int a_plus_b;               \
        int a_minus_b;              \
        int a_mul_b;                \
        int a_div_b;                \
        int a_mod_b;                \
        a_plus_b = 9 + 3;           \
        a_minus_b = 9 - 3;          \
        a_mul_b = 9 * 3;            \
        a_div_b = 9 / 3;            \
        a_mod_b = 9 % 3;            \
    "                               
    SCRIPT_END
    
    ASSERT_TRUE(12 == VAR_INT(a_plus_b));
    ASSERT_TRUE( 6 == VAR_INT(a_minus_b));
    ASSERT_TRUE( 3 == VAR_INT(a_div_b));
    ASSERT_TRUE(27 == VAR_INT(a_mul_b));
    ASSERT_TRUE( 0 == VAR_INT(a_mod_b));
    
    SCRIPT_SHUTDOWN
}

TEST(Definitions, InvalidVariableName)
{
    nap_runtime* runtime = nap_runtime_create(0);
    ASSERT_TRUE(runtime != NULL);
    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
    "              \
    int a_ plus_b; \
    "
    );

    ASSERT_TRUE(bytecode == NULL);
    SCRIPT_SHUTDOWN
}

TEST(StringToIntConversion, DifferentBases)
{
    SCRIPT_START
    "                                          \
    int binary = 0;                            \
    int decimal = 0;                           \
    int zero = 1;                              \
    int hexa = 0;                              \
    int octal = 0;                             \
    asm                                        \
    {                                          \
        mov reg string 0, \"9877\"             \
        mov reg int 0, reg string 0            \
        mov global.decimal, reg int 0          \
        mov reg string 0, \"0xABCDE\"          \
        mov reg int 0, reg string 0            \
        mov global.hexa, reg int 0             \
        mov reg string 0, \"0665544\"          \
        mov reg int 0, reg string 0            \
        mov global.octal, reg int 0            \
        mov reg string 0, \"0b1001001\"        \
        mov reg int 0, reg string 0            \
        mov global.binary, reg int 0           \
        mov reg string 0, \"0\"                \
        mov reg int 0, reg string 0            \
        mov global.zero, reg int 0             \
    }                                          \
    "
    SCRIPT_END

    ASSERT_TRUE(9877   == VAR_INT(decimal));
    ASSERT_TRUE(703710 == VAR_INT(hexa));
    ASSERT_TRUE(224100 == VAR_INT(octal));
    ASSERT_TRUE(73     == VAR_INT(binary));
    ASSERT_TRUE(0      == VAR_INT(zero));

    SCRIPT_SHUTDOWN
}

TEST(Operations, PostPreIncrement)
{
    SCRIPT_START
            "               \
            int i=0;        \
            int y = i++;    \
            int z = ++i;    \
            "
    SCRIPT_END

    ASSERT_TRUE(2 == VAR_INT(i));
    ASSERT_TRUE(0 == VAR_INT(y));
    ASSERT_TRUE(2 == VAR_INT(z));

    SCRIPT_SHUTDOWN
}

TEST(Functions, MessyCodeTest)
{
    SCRIPT_START
    "                             \
    int func(int a, int b)        \
    {                             \
        if(a == 4)                \
        {                         \
            int rr = 9;           \
            return 5;             \
        }                         \
    }                             \
    int func2()                   \
    {                             \
    }                             \
    int z = 3;                    \
    if(z == 3)                    \
    {                             \
      int y = 2;                  \
      z = 4;                      \
    }                             \
    int g = func(z, 66);          \
    int ll = func2();             \
    int y = 1;                    \
    "
    SCRIPT_END
    ASSERT_EQ(5, VAR_INT(g));

    SCRIPT_SHUTDOWN
}

TEST(Keywords, Break)
{
    SCRIPT_START
    "                             \
    int y = 1;                    \
    for(int i = 0; i< 11; i++)    \
    {                             \
      int t = y ++;               \
      if (t == 7)                 \
      {                           \
        break;                    \
      }                           \
    }                             \
    "
    SCRIPT_END
    ASSERT_EQ(8, VAR_INT(y));

    SCRIPT_SHUTDOWN
}

TEST(Functions, DefaultReturnValue)
{
    SCRIPT_START
    "                             \
    int func()                    \
    {                             \
    }                             \
    int z = func();               \
    "
    SCRIPT_END
    ASSERT_EQ(0, VAR_INT(z));

    SCRIPT_SHUTDOWN
}

TEST(InvalidSyntax, PostPreIncMess)
{

    nap_runtime* runtime = nap_runtime_create(0);
    ASSERT_TRUE(runtime != NULL);
    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
    "                              \
     int z = 3;                    \
     int g = z++ ++;               \
    "
    );

    ASSERT_TRUE(bytecode == NULL);
    SCRIPT_SHUTDOWN
}

TEST(RuntimeCompilation, SimpleCheck)
{
    SCRIPT_START
    "                                   \
    int a = 2;                          \
    int b = 3;                          \
    int c;                              \
    nap_execute(\"c = a + b\");         \
    "
    SCRIPT_END

    ASSERT_EQ(5, VAR_INT(c));

    SCRIPT_SHUTDOWN
}

TEST(VariableDefinitions, SimpleIndexedOperation)
{
    SCRIPT_START
    "                               \
        int a[10];                  \
        a[1] = 2;                   \
        int b = a[2];               \
    "
    SCRIPT_END

    ASSERT_EQ(2, VAR_INT(b));

    SCRIPT_SHUTDOWN;
}
