#define CATCH_CONFIG_MAIN

#include "nbci.h"
#include "catch.h"
#include <stdlib.h>

TEST_CASE("VM is initialized", "[vm_init]")
{
	REQUIRE(0 == 1);
}

/*{
    nap_runtime* runtime = nap_runtime_create();
    nap_runtime_execute(runtime,
                     "                            \
                      int a;                      \
                      a = 1;                      \
                     "
                    );
    ck_assert_int_eq(1, runtime->get_int("a"));
    free(runtime);
}
*/