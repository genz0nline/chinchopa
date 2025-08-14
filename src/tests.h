#pragma once

typedef struct test {
    char *nm;
    int (*f) (void *);
} test_t;

void print_passed();
void print_failed();
