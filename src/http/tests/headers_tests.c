#include "headers_tests.h"
#include "../resp.h"
#include "../headers.h"

#include <CUnit/CUnit.h>
#include <stdlib.h>
#include <string.h>

void add_resp_header_test(void) {
    response_t *response = (response_t *)malloc(sizeof(response_t));

    response->headers = NULL;
    response->h_len = 0;
    response->h_size = 0;

    CU_ASSERT_FATAL(add_resp_header(response, "header1", "value1", 0) == 0);
    CU_ASSERT(response->h_len == 1);
    CU_ASSERT_STRING_EQUAL(response->headers[0].name, "header1");
    CU_ASSERT_STRING_EQUAL(response->headers[0].value, "value1");

    free(response);
}
