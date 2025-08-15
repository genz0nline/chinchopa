#include "route_tests.h"
#include "../req.h"
#include "../route.h"
#include <CUnit/CUnit.h>
#include <assert.h>
#include <stdlib.h>

request_t *test_request;

int init_validate_uri_suite(void) {
    test_request = (request_t *)malloc(sizeof(request_t));
    if (!test_request) {
        return 1;
    }

    return 0;
}

int clean_validate_uri_suite(void) {
    free(test_request);
    return 0;
}

void validate_uri_test1(void) {
    test_request->bytes = "/";
    CU_ASSERT(validate_uri(test_request) == 0);
}

void validate_uri_test2(void) {
    test_request->bytes = "/index.html";
    CU_ASSERT(validate_uri(test_request) == 0);
}

void validate_uri_test3(void) {
    test_request->bytes = "/../index.html";
    CU_ASSERT(validate_uri(test_request) == 1);
}
