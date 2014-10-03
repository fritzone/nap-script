#include "tests.h"
#include "gtest/gtest.h"

/*
 * Test for handling various variable definitions
 */

/* Define a one dim array, set the first element to a value, and see that it
   is as expected */
TEST(VariableDefinitions, SimpleIndexedOperationInt)
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

/* Define an int variable as a substring */
TEST(VariableDefinitions, IntFromSubstring)
{
    SCRIPT_START
    "                              \
        string a = \"12456789\";   \
        int t = a[1,3];            \
        int t2 = a[3,4];           \
        int t3 = a[3];             \
        int t4 = a[3, a.len - 1];  \
    "
    SCRIPT_END

    ASSERT_EQ(245, VAR_INT(t));
    ASSERT_EQ(56, VAR_INT(t2));
    ASSERT_EQ(5, VAR_INT(t3));
    ASSERT_EQ(56789, VAR_INT(t4));
    SCRIPT_SHUTDOWN
}

/* Define a one dim array, set the first element to a value, and see that it
   is as expected */
TEST(VariableDefinitions, SimpleIndexedOperationByte)
{
    SCRIPT_START
    "                               \
        byte a[10];                 \
        a[1] = 2;                   \
        byte b = a[1];              \
    "
    SCRIPT_END

    ASSERT_EQ(2, VAR_BYTE(b));

    SCRIPT_SHUTDOWN;
}

TEST(VariableDefinitions, ComplicatedIndexedOperationInt)
{
    SCRIPT_START
    "                               \
    int[] fun()                     \
    {                               \
        int t = 12;                 \
        int result[t];              \
        result[0] = 9;              \
        return result;              \
    }                               \
    int b[] =fun();                 \
    int a = b[0];                   \
    "
    SCRIPT_END

    ASSERT_EQ(9, VAR_INT(a));

    SCRIPT_SHUTDOWN;
}

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


/* Define a simple byte type variable, assign a value to it. */
TEST(VariableDefinitions, SimpleByte)
{
    SCRIPT_START
    "                               \
        byte a;                      \
        a = 2;                      \
    "
    SCRIPT_END

    ASSERT_EQ(2, VAR_BYTE(a));

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

/* Define a string and use the substring operation on it to set the characters
   between [x,y] to a specific string*/
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


TEST(VariableDefinitions, UsingVarFromAboveScope)
{
    SCRIPT_START
    "                                 \
            int b;                    \
            for(int i=0; i<10; i++)   \
            {                         \
                int a;                \
                a = a + i;            \
                b += a;               \
            }                         \
    "
    SCRIPT_END

    ASSERT_EQ(1+2+3+4+5+6+7+8+9, VAR_INT(b));

    SCRIPT_SHUTDOWN;
}

