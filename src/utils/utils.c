#include "utils.h"
#include <ctype.h>

int strcicmp(const char *s, const char *t) {
    while (*s && *t && tolower((unsigned char)*s) == tolower((unsigned char)*t)) {
        s++;
        t++;
    }
    return tolower((unsigned char)*s) - tolower((unsigned char)*t);
}
