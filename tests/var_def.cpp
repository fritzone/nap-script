#include "tests.h"
#include "gtest/gtest.h"

/*
 * Test for handling various variable definitions
 */

/* Define a one dim array, set the first element to a value, and see that it
   is as expected */
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

