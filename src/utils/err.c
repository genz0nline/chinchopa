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
            case INCUS:
                return "Incorrect usage, try: chinchopa --help";
            case INPORT:
                return "Invalid port value, port has to be between 1 and 65535";
            case REQCAP:
                return "Request exceeds allowed limit";
            case INVREQ:
                return "Invalid HTTP request\n";
            case UNKNOWN_METHOD:
                return "Unknown method\n";
            case INVVER:
                return "Invalid protocol version\n";
            case INVHEADER:
                return "Invalid header\n";
            case INVURI:
                return "Invalid uri\n";
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
