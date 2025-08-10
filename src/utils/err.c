#include "err.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

void seterr(errors_t err) {
    errno = err;
}

char *strerr(int errnum) {
    if (errno >= 0) {
        return strerror(errno);
    } else {
        switch (errno) {
            // My custom errno values
            case INCUS:
                return "Incorrect usage, try: chinchopa --help";
            default:
                return "Unknown error";
        }
    }
}

void perr(const char *s) {
    if (s && *s) {
        fprintf(stderr, "%s: %s\n", s, strerr(errno));
    } else {
        fprintf(stderr, "%s\n", strerr(errno));
    }
}
