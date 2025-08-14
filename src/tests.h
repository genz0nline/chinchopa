#pragma once

typedef struct test {
    char *nm;
    void (*f) (void *);
} test_t;
