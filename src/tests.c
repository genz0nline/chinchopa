#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "tests.h"
#include "http/route_tests.h"

test_t tests[] = {
    {"validate_uri_test1", validate_uri_test1},
    {NULL, NULL}
};

int main(void) {
    test_t *tst;
    int counter = 0;

    int total_tests = 0;
    while (tests[total_tests++].f != NULL);
    total_tests--;

    for (tst = tests; tst->f != NULL; tst++) {
        fprintf(stderr, "Running test %d: %s...\n", counter + 1, tst->nm);
        tst->f(NULL);
        fprintf(stderr, "\tPassed\n");
        counter++;
    }

    fprintf(stderr, "Passed tests: %d/%d\n", counter, total_tests);

    return 0;

}
