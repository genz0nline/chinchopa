#include "opt.h"

#include <string.h>
#include <stdlib.h>

options_t *options_init(int argc, char *argv[]) {
    options_t *opts = (options_t *)malloc(sizeof(options_t));

    opts->help = 0;

    for (int i = 1; i < argc; i += 2) {
        
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            opts->help = 1;
        }

        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
            if (i + 1 == argc) {
                free(opts);
                return NULL;
            }
            strncpy(opts->dir, argv[i+1], MAX_DIR_LEN);
        }

        if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--user") == 0) {
            if (i + 1 == argc) {
                free(opts);
                return NULL;
            }
            strncpy(opts->username, argv[i+1], MAX_USERNAME_LEN);
        }
    }

    return opts;
}

void options_destroy(options_t *options) {
    if (!options) {
        return;
    }

    free(options);
}
