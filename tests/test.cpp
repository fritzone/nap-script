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

#define VAR_STRING(a)    nap_runtime_get_string(runtime, #a, &found_indicator)

#define SCRIPT_SHUTDOWN \
    nap_runtime_shutdown(&runtime);                                \
    ASSERT_TRUE(runtime == NULL);

#define SCRIPT_ASSERT_STREQ(with,what)                             \
    do {                                                           \
        char* what = VAR_STRING(what);                             \
        ASSERT_STREQ(with, what);                                  \
        free(what);                                                \
    } while(0);
/*
 * TESTS
 */

/* Define a simple integer type variable, assign a value to it. */
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

/* Define a string variable. Use the [] operator to change the second
   character in it.*/
TEST(VariableDefinitions, StringIndexedOperation)
{
    SCRIPT_START
    "                               \
        string b = \"AABB\";        \
        b[1] = \"c\";               \
    "
    SCRIPT_END

    SCRIPT_ASSERT_STREQ("AcBB", b);

    SCRIPT_SHUTDOWN;
}

/* Define a string variable, use the [,] operator to change a part from it.
   The second indexe should be greater than the first one */
TEST(VariableDefinitions, StringSubstringIndexedOperation1)
{
    SCRIPT_START
    "                               \
        string b = \"AABB\";        \
        b[1,2] = \"cc\";            \
    "
    SCRIPT_END

    SCRIPT_ASSERT_STREQ("AccB", b);

    SCRIPT_SHUTDOWN;
}

/* Define a string variable, use the [,] operator to change a part of it.
   The second index should be greater than the length of the string.
   Expected outcome is that the end of the string will be removed and
   it will end with the new string. */
TEST(VariableDefinitions, StringSubstringIndexedOperation2)
{
    SCRIPT_START
    "                               \
        string b = \"AABB\";        \
        b[1,5] = \"cc\";            \
    "
    SCRIPT_END

    SCRIPT_ASSERT_STREQ("Acc", b);

    SCRIPT_SHUTDOWN;
}

TEST(VariableDefinitions, StringSubstringIndexedOperation3)
{
    SCRIPT_START
    "                               \
        string b = \"AABB\";        \
        b[1,3] = \"cc\";            \
    "
    SCRIPT_END

    SCRIPT_ASSERT_STREQ("Acc", b);

    SCRIPT_SHUTDOWN;
}

TEST(VariableDefinitions, StringSubstringIndexedOperationInsertion)
{
    SCRIPT_START
    "                               \
        string b = \"ABCD\";        \
        b[1,1] = \"cc\";            \
    "
    SCRIPT_END

    SCRIPT_ASSERT_STREQ("AccCD", b);

    SCRIPT_SHUTDOWN;
}


TEST(Operations, BasicIntVariableOperations)
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

TEST(Operations, BasicStringVariableOperations1)
{

    SCRIPT_START
    "                               \
        string a = \"AA\";          \
        string b = \"BB\";          \
        string a_plus_b;            \
        a_plus_b = a + b;           \
    "
    SCRIPT_END

    SCRIPT_ASSERT_STREQ("AABB", a_plus_b);

    SCRIPT_SHUTDOWN
}

TEST(Operations, BasicStringVariableOperations2)
{

    SCRIPT_START
    "                               \
        string a = \"A\";           \
        string b = a + \"B\";       \
    "
    SCRIPT_END

    SCRIPT_ASSERT_STREQ("AB", b);

    SCRIPT_SHUTDOWN
}

TEST(Operations, BasicStringVariableOperations4)
{

    SCRIPT_START
    "                                      \
        string sa = \"A\";                 \
        string sb ;                        \
        asm                                \
        {                                  \
        mov reg string(0), global.sa       \
        mov reg string(1), \"B\"           \
        add reg string(0), reg string(1)   \
        mov global.sb, reg string(0)       \
        }                                  \
    "
    SCRIPT_END

    SCRIPT_ASSERT_STREQ("AB", sb);

    SCRIPT_SHUTDOWN
}

/* Define a string variable, copy out one character from it into an int register
   see that the conversion succeeded.*/
TEST(Operations, BasicStringVariableOperations5)
{

    SCRIPT_START
    "                                        \
        string sa = \"A123B\";               \
        int ib = 9;                          \
        asm                                  \
        {                                    \
        mov reg idx(0), 1                    \
        mov reg int(0), @#ccidx(global.sa, 1)\
        mov global.ib, reg int (0)           \
        }                                    \
    "
    SCRIPT_END

    ASSERT_EQ(1, VAR_INT(ib));

    SCRIPT_SHUTDOWN
}

/* Define a string variable, copy out a substring from it into an int register
   see that the conversion succeeded.*/
TEST(Operations, BasicStringVariableOperations6)
{

    SCRIPT_START
    "                                        \
        string sa = \"A123B\";               \
        int ib = 9;                          \
        asm                                  \
        {                                    \
        mov reg idx(0), 1                    \
        mov reg idx(1), 3                    \
        mov reg int(0), @#ccidx(global.sa, 2)\
        mov global.ib, reg int (0)           \
        }                                    \
    "
    SCRIPT_END

    ASSERT_EQ(123, VAR_INT(ib));

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

TEST(RuntimeCompilation, CompoundedExpression)
{
    SCRIPT_START
    "                                   \
    int a = 2;                          \
    int b = 3;                          \
    int c;                              \
    string sa = \"c=a\";                \
    string sb = \"+b\";                 \
    nap_execute(sa + sb);               \
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
        int b = a[1];               \
    "
    SCRIPT_END

    ASSERT_EQ(2, VAR_INT(b));

    SCRIPT_SHUTDOWN;
}
