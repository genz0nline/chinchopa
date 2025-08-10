#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define DATE_LEN    19

void get_current_time(char *buf) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    snprintf(buf, DATE_LEN + 1, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
             tm->tm_year + 1900,
             tm->tm_mon + 1,
             tm->tm_mday,
             tm->tm_hour,
             tm->tm_min,
             tm->tm_sec);
}



void log_printf(char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);

    char time[DATE_LEN + 1];
    get_current_time(time);

    fprintf(stderr, "[%s]: ", time);
    vfprintf(stderr, fmt, ap);

    va_end(ap);
}
