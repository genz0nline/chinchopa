#include <CUnit/Basic.h>
#include <CUnit/TestDB.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <CUnit/CUnit.h>

#include "http/tests/route_tests.h"
#include "http/tests/headers_tests.h"

int main(void) {
    if (CU_initialize_registry() != CUE_SUCCESS) {
        fprintf(stderr, "Couldn't initialize CU\n");
    }

    CU_pSuite validate_uri_suite = CU_add_suite("validate_uri_suite",
                                   init_validate_uri_suite,
                                   clean_validate_uri_suite);

    CU_ADD_TEST(validate_uri_suite, validate_uri_test1);
    CU_ADD_TEST(validate_uri_suite, validate_uri_test2);
    CU_ADD_TEST(validate_uri_suite, validate_uri_test3);

    CU_pSuite add_resp_header_suite = CU_add_suite("add_resp_header_suite", NULL, NULL);

    CU_ADD_TEST(add_resp_header_suite, add_resp_header_test);

    CU_basic_run_tests();

    CU_cleanup_registry();

    return 0;
}
