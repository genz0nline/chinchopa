#include "../req.h"
#include "../route.h"
#include "../../tests.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int validate_uri_test(void *p) {
    char *cases[] = {
        "/",
        "/index.html",
        "/../index.html",
        NULL,
    };

    int results[] = {
        0,
        0,
        1,
        0,
    };

    request_t *request = (request_t *)malloc(sizeof(request_t));

    int failed = 0;

    for (int i = 0; cases[i] != NULL; i++) {
        fprintf(stderr, "\t\tCase %d, uri = \"%s\": ", i + 1, cases[i]);
        request->bytes = cases[i];
        request->uri_pos = 0;

        if ((validate_uri(request) != results[i])) {
            failed = 1;
        }
        if (failed) {
            print_failed();
        } else {
            print_passed();
        }
    }

    free(request);
    return failed;
}
