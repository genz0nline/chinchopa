#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/io.h>
#include <unistd.h>
#include <pthread.h>

#include "server.h"
#include "handler.h"
#include "sys/net.h"
#include "sys/io.h"
#include "utils/err.h"
#include "utils/opt.h"
#include "utils/log.h"

typedef struct server {
    pthread_t accept_thread;
    options_t *options;
    network_t *network;
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

    log_printf("Server initialized with the following options:\n"
               "\t\t\t    help: %d\n"
               "\t\t\t     dir: %s\n"
               "\t\t\tusername: %s\n"
               "\t\t\t    port: %d\n",
               server->options->help,
               server->options->dir,
               server->options->username,
               server->options->h_port
               );


    server->network = network_init(server->options);
    if (!server->network) {
        perr("network_init");

        options_destroy(server->options);
        free(server);

        return NULL;
    }

    return server;
}

void *server_accept_connections(server_t *server) {

    return NULL;
}

static volatile sig_atomic_t stop = 0;

static void sig_handler(int sig) {
    stop = 1;
}

int server_run(server_t *server) {

    if (!server) {
        return 1;
    }

    if (server->options->help) {
        printf("Usage: chinchopa [-h] [-d path] [-u username] [--help] [--dir path] [--user username]\n");
        return 0;
    }

    struct sigaction sa = {0};
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    while (!stop) {
        io_t *io = accept_connection(server->network);
        if (!io) {
            continue;
        }

        if (handle_connection(io, server->options)) {
            continue;
        }
    }

    log_printf("Exiting...\n");
    return 0;
}


void server_destroy(server_t *server) {
    if (!server) {
        return;
    }

    network_destroy(server->network);
    options_destroy(server->options);
    free(server);
}
