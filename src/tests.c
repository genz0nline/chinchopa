#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "tests.h"
#include "http/tests/route_tests.h"
#include "http/tests/headers_tests.h"

test_t tests[] = {
    {"validate_uri_test", validate_uri_test},
    {"add_resp_header_test", add_resp_header_test},
    {NULL, NULL}
};

void print_passed() {
    fprintf(stderr, "\t\033[32mPassed\033[m\n");
}

void print_failed() {
    fprintf(stderr, "\t\033[31mFailed\033[m\n");
}

int main(void) {
    test_t *tst;
    int counter = 0;
    int success = 0;

    int total_tests = 0;
    while (tests[total_tests++].f != NULL);
    total_tests--;

    int failed;
    for (tst = tests; tst->f != NULL; tst++) {

        fprintf(stderr, "Running test %d: %s...\n", counter + 1, tst->nm);
        failed = tst->f(NULL);
        if (failed) {
            print_failed();
        } else {
            print_passed();
        }
        success += failed ? 0 : 1;
        counter++;
    }

    fprintf(stderr, "Passed tests: %d/%d\n", success, total_tests);

    return 0;

}
