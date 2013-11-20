#define CATCH_CONFIG_MAIN
#include "catch.h"

#include "nap_runtime.h"


TEST_CASE("Variable definitions", "[variables]")
{
    nap_runtime* runtime = nap_runtime_create(0);
    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
    "                               \
        int a;                      \
        a = 2;                      \
    "                               
    );
    int t = nap_runtime_execute(runtime, bytecode);
    REQUIRE(1 == t);

    REQUIRE(2 == nap_runtime_get_int(runtime, "a"));
    nap_runtime_shutdown(&runtime);
    REQUIRE(runtime == 0);
}

TEST_CASE("Operations on variables", "[operations][variables]")
{
    nap_runtime* runtime = nap_runtime_create(0);
    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
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
    );
    int t = nap_runtime_execute(runtime, bytecode);
    REQUIRE(1 == t);

    REQUIRE(9 == nap_runtime_get_int(runtime, "a"));
    REQUIRE(3 == nap_runtime_get_int(runtime, "b"));
    REQUIRE(12 == nap_runtime_get_int(runtime, "a_plus_b"));
    REQUIRE(6 == nap_runtime_get_int(runtime, "a_minus_b"));
    REQUIRE(3 == nap_runtime_get_int(runtime, "a_div_b"));
    REQUIRE(27 == nap_runtime_get_int(runtime, "a_mul_b"));
    REQUIRE(0 == nap_runtime_get_int(runtime, "a_mod_b"));
    
    nap_runtime_shutdown(&runtime);
    REQUIRE(runtime == 0);
}

TEST_CASE("Operations on immediate values", "[operations][immediates]")
{
    nap_runtime* runtime = nap_runtime_create(0);
    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
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
    );
    int t = nap_runtime_execute(runtime, bytecode);
    REQUIRE(1 == t);

    REQUIRE(12 == nap_runtime_get_int(runtime, "a_plus_b"));
    REQUIRE(6 == nap_runtime_get_int(runtime, "a_minus_b"));
    REQUIRE(3 == nap_runtime_get_int(runtime, "a_div_b"));
    REQUIRE(27 == nap_runtime_get_int(runtime, "a_mul_b"));
    REQUIRE(0 == nap_runtime_get_int(runtime, "a_mod_b"));
    
    nap_runtime_shutdown(&runtime);
    REQUIRE(runtime == 0);
}

TEST_CASE("Testing of compile time error", "[error][compile]")
{
    nap_runtime* runtime = nap_runtime_create(0);
    nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,
    "                               \
        int a_ plus_b;               \
    "
    );
    
    int t = nap_runtime_execute(runtime, bytecode);
    REQUIRE(1 == t);
    nap_runtime_shutdown(&runtime);
    REQUIRE(runtime == 0);
}
