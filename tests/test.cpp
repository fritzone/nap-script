#define CATCH_CONFIG_MAIN
#include "catch.h"

#include "nap_runtime.h"


TEST_CASE("Variables", "[vm_variables]")
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
