#pragma once

#include <stdint.h>

#define MAX_PATH_LEN             128
#define MAX_USERNAME_LEN        32
#define MAX_PORT_LEN            6

typedef struct options {
    int help;
    char dir[MAX_PATH_LEN];
    char username[MAX_USERNAME_LEN];
    uint16_t h_port;
    char ssl_cert[MAX_PATH_LEN];
    char ssl_key[MAX_PATH_LEN];
    int use_ssl;
} options_t;


options_t *options_init(int argc, char *argv[]);
void options_destroy(options_t *options);
