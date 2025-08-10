#include <stdlib.h>
#include <stdio.h>

#include "server.h"
#include "utils/err.h"
#include "utils/opt.h"

typedef struct server {
    options_t *options;
} server_t;

server_t *server_init(int argc, char *argv[]) {
    server_t *server = (server_t *)malloc(sizeof(server_t));
    if (!server) return NULL;

    server->options = options_init(argc, argv);
    if (!server->options) {
        perr("options_init");
        free(server);
        return NULL;
    }

    return server;
}

int server_run(server_t *server) {

    if (!server) {
        return 1;
    }

    if (server->options->help) {
        printf("Usage: chinchopa [-h] [-d path] [-u username] [--help] [--dir path] [--user username]\n");
        return 0;
    }

    return 0;
}


void server_destroy(server_t *server) {
    if (!server) {
        return;
    }

    options_destroy(server->options);
    free(server);
}
