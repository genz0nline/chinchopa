#include "opt.h"
#include "err.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

options_t *options_init(int argc, char *argv[]) {
    options_t *opts = (options_t *)malloc(sizeof(options_t));

    opts->help = 0;
    opts->h_port = 80;
    opts->dir[0] = '\0';
    opts->username[0] = '\0';
    opts->ssl_cert[0] = '\0';
    opts->ssl_key[0] = '\0';

    for (int i = 1; i < argc; i += 2) {
        
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            opts->help = 1;
        }

        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 == argc) {
                free(opts);
                seterr(INCUS);
                return NULL;
            }

            int port = atoi(argv[i+1]);
            if (0 > port || port > 65535) {
                free(opts);
                seterr(INPORT);
                return NULL;
            }

            opts->h_port = (uint16_t) port;
        }

        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
            if (i + 1 == argc) {
                free(opts);
                seterr(INCUS);
                return NULL;
            }
            strncpy(opts->dir, argv[i+1], MAX_PATH_LEN);

            // No trailing slash in dir
            if (opts->dir[strlen(opts->dir) - 1] == '/') {
                opts->dir[strlen(opts->dir) - 1] = '\0';
            }
        }

        if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--user") == 0) {
            if (i + 1 == argc) {
                free(opts);
                seterr(INCUS);
                return NULL;
            }
            strncpy(opts->username, argv[i+1], MAX_USERNAME_LEN);
        }

        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--ssl_cert") == 0) {
            if (i + 1 == argc) {
                free(opts);
                seterr(INCUS);
                return NULL;
            }
            strncpy(opts->ssl_cert, argv[i+1], MAX_PATH_LEN);
        }

        if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--ssl_key") == 0) {
            if (i + 1 == argc) {
                free(opts);
                seterr(INCUS);
                return NULL;
            }
            strncpy(opts->ssl_key, argv[i+1], MAX_PATH_LEN);
        }
    }

    if (strlen(opts->ssl_key) > 0 && strlen(opts->ssl_cert) > 0) {
        opts->use_ssl = 1;
    } else {
        opts->use_ssl = 0;
    }

    return opts;
}

void options_destroy(options_t *options) {
    if (!options) {
        return;
    }

    free(options);
}
