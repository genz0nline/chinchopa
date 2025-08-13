#pragma once

typedef enum errors {
    INCUS = -1,
    INPORT = -2,
    REQCAP = -3,
    INVREQ = -4,
    UNKNOWN_METHOD = -5,
    INVVER = -6,
    INVHEADER = -7,
    INVURI = -8,
} errors_t;

void seterr(errors_t err);
char *strerr(int errnum);
void perr(const char *s);
