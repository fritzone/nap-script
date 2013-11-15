#define CATCH_CONFIG_MAIN

#include "catch.h"
#include "nap_runtime.h"
#include <stdlib.h>

    #define SCRIPT_START nap_runtime* runtime = nap_runtime_create(0);              \
                     nap_bytecode_chunk* bytecode = nap_runtime_compile(runtime,\
                     

    #define SCRIPT_END  ); \
                    int t = nap_runtime_execute(runtime, bytecode); \
                    REQUIRE(1 == t); 

    TEST_CASE("Variables", "[vm_variables]")
    {
    SCRIPT_START                    
    "                               \
        int a;                      \
        a = 2;                      \
    "                               
    SCRIPT_END
    
    REQUIRE(2 == nap_runtime_get_int(runtime, "a"));
    free(runtime);
    }

