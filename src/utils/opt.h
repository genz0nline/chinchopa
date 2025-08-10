#pragma once

#define MAX_DIR_LEN             128
#define MAX_USERNAME_LEN        32

typedef struct options {
    int help;
    char dir[MAX_DIR_LEN];
    char username[MAX_USERNAME_LEN];
} options_t;


options_t *options_init(int argc, char *argv[]);
void options_destroy(options_t *options);
