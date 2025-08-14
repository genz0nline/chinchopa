#include "headers_tests.h"
#include "../resp.h"
#include "../headers.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

int add_resp_header_test(void *p) {
    response_t *response = (response_t *)malloc(sizeof(response_t));

    response->headers = NULL;
    response->h_len = 0;
    response->h_size = 0;

    add_resp_header(response, "header1", "value1", 0);
    if (!(response->h_len == 1)) {
        free(response);
        return 1;
    }
    if (strcmp(response->headers[0].name,  "header1")) {
        free(response);
        return 1;
    }
    if (strcmp(response->headers[0].value,  "value1")) {
        free(response);
        return 1;
    }

    free(response);
    return 0;
}
