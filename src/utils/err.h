#pragma once

typedef enum errors {
    INCUS = -1,
} errors_t;

void seterr(errors_t err);
char *strerr(int errnum);
void perr(const char *s);
