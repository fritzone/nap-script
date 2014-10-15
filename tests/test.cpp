#include "tests.h"
#include "nbci.h"
#include "gtest/gtest.h"

#include "compiler.h"
#include "nap_runtime.h"

TEST(Assembly, PushPop)
{
  nap_runtime* runtime = nap_runtime_create("$");
  ASSERT_FALSE(runtime == NULL);
  int found_indicator;
  nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
  "                        \
  int a;                   \
  asm                      \
  {                        \
       push 23;            \
       pop global.a;       \
  }                        \
  "
  SCRIPT_END

  ASSERT_EQ(23, VAR_INT(a));

  SCRIPT_SHUTDOWN
}

TEST(Definitions, CodeBlocksWithScope)
{
    SCRIPT_START
    "                                    \
            int a = 1;                   \
            {  int c = 3;int a = 2;}     \
            int b = 2;                   \
    "
    SCRIPT_END
    ASSERT_TRUE( 1 == VAR_INT(a));
    ASSERT_TRUE( 2 == VAR_INT(b));

    SCRIPT_SHUTDOWN
}

TEST(Floats, BasicRealOperations)
{
    SCRIPT_START
    "                                    \
        real a = 5.6;                    \
        real b = a * 10;                 \
    "
    SCRIPT_END
	double _b = (double)VAR_REAL(b);
    ASSERT_DOUBLE_EQ((double)56.0, _b);

    SCRIPT_SHUTDOWN
}

TEST(Operations, UnaryMathOperations)
{
    SCRIPT_START
    "                                 \
        int a = 5;                    \
        int b = -5;                   \
        int c = +b;                   \
        int d = -a;                   \
        int e = 3 - (-2);             \
    "
    SCRIPT_END

    ASSERT_EQ(5, VAR_INT(c));
    ASSERT_EQ(-5, VAR_INT(d));
    ASSERT_EQ(5, VAR_INT(e));

    SCRIPT_SHUTDOWN

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

TEST(Operations, BasicBitwiseOperations)
{

    SCRIPT_START
    "                                    \
            int a1 = 1;                  \
            int a2 = 2;                  \
            int a3 = 3;                  \
            int a4 = 1;                  \
            int a5 = 2;                  \
            int a6 = 2;                  \
            int b1 = a1 << 1;            \
            int b2 = a2 >> 1;            \
            int b3 = a3 &  1;            \
            int b4 = a4 |  2;            \
            int b5 = a5 ^  1;            \
            int b6 = ~a6;                \
    "
    SCRIPT_END

    ASSERT_TRUE( 2 == VAR_INT(b1));
    ASSERT_TRUE( 1 == VAR_INT(b2));
    ASSERT_TRUE( 1 == VAR_INT(b3));
    ASSERT_TRUE( 3 == VAR_INT(b4));
    ASSERT_TRUE( 3 == VAR_INT(b5));
    ASSERT_TRUE(-3 == VAR_INT(b6));

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
    ,0);

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

TEST(InvalidSyntax, PostPreIncMess)
{

    nap_runtime* runtime = nap_runtime_create(0);
    ASSERT_TRUE(runtime != NULL);
    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
    "                              \
     int z = 3;                    \
     int g = z++ ++;               \
    "
    ,0);

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

/*
 * TESTS FOR FUNCTIONS AND THEIR ASSOCIATED BEHAVIOUR
 */

/* Define a function returning an in which takes in an int parameter. Return the
 next value of the parameter (ie: par + 1). Check in the calling code the
 value. */
TEST(Functions, SimpleFunctionCall)
{
    SCRIPT_START
    "                               \
        int func(int a)             \
        {                           \
            return a + 1;           \
        }                           \
        int a = 5;                  \
        a = func(a);                \
    "
    SCRIPT_END

    ASSERT_EQ(6, VAR_INT(a));

    SCRIPT_SHUTDOWN;
}

/* Define a function returning and int. Do not put return statements in the body.
 * Use the function, see that it returns the default return value (0).
 */
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

/* Define an external function, also implement it in the test file.
 * Use the function, see that it does not fail.
 */
TEST(Functions, ExternalCalling)
{
    nap_runtime* runtime = nap_runtime_create("a");
    ASSERT_FALSE(runtime == NULL);

    int found_indicator;
    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,    "  \
    extern void external_callee(int, int);                            \
    external_callee(1,2);                                             \
    "
    SCRIPT_END

    SCRIPT_SHUTDOWN

    UNUSED(found_indicator);
}

/* Define a function with an int reference as parameter. Assign a value to
 * the parameter in the function body and see that the caller gets modified */
TEST(Functions, IntReference)
{
    SCRIPT_START
    "                             \
    void func(int& a)             \
    {                             \
        a = 6;                    \
    }                             \
    int z = 7; func(z);           \
    "
    SCRIPT_END
    ASSERT_EQ(6, VAR_INT(z));

    SCRIPT_SHUTDOWN
}

/* Define a function with an byte reference as parameter. Assign a value to
 * the parameter in the function body and see that the caller gets modified */
TEST(Functions, ByteReference)
{
    SCRIPT_START
    "                             \
    void func(byte& a)            \
    {                             \
        a = 6;                    \
    }                             \
    byte z = 7; func(z);          \
    "
    SCRIPT_END
    ASSERT_EQ(6, VAR_BYTE(z));

    SCRIPT_SHUTDOWN
}

/* Define a function with an real reference as parameter. Assign a value to
 * the parameter in the function body and see that the caller gets modified */
TEST(Functions, RealReference)
{
    SCRIPT_START
    "                             \
    void func(real& a)            \
    {                             \
        a = 6.7;                  \
    }                             \
    real z = 7.8; func(z);        \
    "
    SCRIPT_END
    double _z = (double)VAR_REAL(z);
    ASSERT_DOUBLE_EQ((double)6.7, _z);
    SCRIPT_SHUTDOWN
}

/* Define a function with a real array as parameter. Assign a value to
 * the parameter in the function body and see that the caller does not get
 * modified */
TEST(Functions, RealArray)
{
    SCRIPT_START
    "                           \
    void func(real a[])         \
    {                           \
        a[0] = 6.7;             \
    }                           \
    real z[10];                 \
    z[0] = 7.8; func(z);        \
    real b = z[0];              \
    "
    SCRIPT_END
    double _b = (double)VAR_REAL(b);
    ASSERT_DOUBLE_EQ((double)7.8, _b);
    SCRIPT_SHUTDOWN
}

/* Define a function with an int array as parameter. Assign a value to
 * the parameter in the function body and see that the caller does not get
 * modified */
TEST(Functions, IntArray)
{
    SCRIPT_START
    "                           \
    void func(int a[])          \
    {                           \
        a[0] = 6;               \
    }                           \
    int z[10];                  \
    z[0] = 8; func(z);          \
    int b = z[0];               \
    "
    SCRIPT_END
    ASSERT_EQ(8, VAR_INT(b));

    SCRIPT_SHUTDOWN
}

/* Define a function with a byte array as parameter. Assign a value to
 * the parameter in the function body and see that the caller does not get
 * modified */
TEST(Functions, ByteArray)
{
    SCRIPT_START
    "                           \
    void func(byte a[])         \
    {                           \
        a[0] = 6;               \
    }                           \
    byte z[10];                 \
    z[0] = 8; func(z);          \
    byte b = z[0];              \
    "
    SCRIPT_END
    ASSERT_EQ(8, VAR_BYTE(b));

    SCRIPT_SHUTDOWN
}

/* Define a function with a byte array reference parameter. Assign a value to
 * the parameter in the function body and see that the caller does get
 * modified */
TEST(Functions, ByteArrayReference)
{
    SCRIPT_START
    "                           \
    void func(byte& a[])        \
    {                           \
        a[0] = 6;               \
    }                           \
    byte z[10];                 \
    z[0] = 8; func(z);          \
    byte b = z[0];              \
    "
    SCRIPT_END
    ASSERT_EQ(6, VAR_BYTE(b));

    SCRIPT_SHUTDOWN
}

/* Define a function with an int array reference parameter. Assign a value to
 * the parameter in the function body and see that the caller does get
 * modified */
TEST(Functions, IntArrayReference)
{
    SCRIPT_START
    "                           \
    void func(int& a[])         \
    {                           \
        a[0] = 6;               \
    }                           \
    int z[10];                  \
    z[0] = 8; func(z);          \
    int b = z[0];               \
    "
    SCRIPT_END
    ASSERT_EQ(6, VAR_INT(b));

    SCRIPT_SHUTDOWN
}

/* Define a function with a real array as parameter. Assign a value to
 * the parameter in the function body and see that the caller does not get
 * modified */
TEST(Functions, RealArrayReference)
{
    SCRIPT_START
    "                           \
    void func(real& a[])        \
    {                           \
        a[0] = 6.7;             \
    }                           \
    real z[10];                 \
    z[0] = 7.8; func(z);        \
    real b = z[0];              \
    "
    SCRIPT_END
    double _b = (double)VAR_REAL(b);
    ASSERT_DOUBLE_EQ((double)6.7, _b);
    SCRIPT_SHUTDOWN
}

NAP_EXPORTS
void external_callee(nap_int_t a, nap_int_t b)
{
    printf("%"PRINT_d" %"PRINT_d"\n", a, b);
    if(a != 1 || b != 2) FAIL();
}
