#pragma once

#include <stdint.h>

#define MAX_DIR_LEN             128
#define MAX_USERNAME_LEN        32
#define MAX_PORT_LEN            6

typedef struct options {
    int help;
    char dir[MAX_DIR_LEN];
    char username[MAX_USERNAME_LEN];
    uint16_t h_port;
} options_t;


options_t *options_init(int argc, char *argv[]);
void options_destroy(options_t *options);
