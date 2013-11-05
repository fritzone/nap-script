#include "nbci.h"

#include <check.h>
#include <stdlib.h>

START_TEST(test_vm_push_immediate)
{
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
END_TEST

Suite* suite_vm_push()
{
    Suite* s = suite_create("push");

    /* Core test case */
    TCase *tc_immediate = tcase_create ("immediate");
    tcase_add_test (tc_immediate, test_vm_push_immediate);
    suite_add_tcase (s, tc_immediate);

    return s;
}


int main ()
{
    int number_failed = 0;
    Suite *s = suite_vm_push();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
