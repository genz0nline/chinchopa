#include <asm-generic/socket.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/io.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "net.h"
#include "io.h"
#include "../utils/opt.h"
#include "../utils/err.h"

typedef struct network {
    int srv;
} network_t;

network_t *network_init(options_t *options) {
    network_t *network = (network_t *)malloc(sizeof(network_t));

    network->srv = socket(AF_INET, SOCK_STREAM, 0);
    if (network->srv < 0) {
        perr("socket");
        free(network);
        return NULL;
    }

    int opt = 1;
    if (setsockopt(network->srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perr("setsockopt");
        free(network);
        return NULL;
    }

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(options->h_port);


    if (bind(network->srv, (struct sockaddr *)&addr, (socklen_t)sizeof(addr)) < 0) {
        perr("bind");
        free(network);
        return NULL;
    }

    if (listen(network->srv, SOMAXCONN) < 0) {
        perr("listen");
        free(network);
        return NULL;
    }

    return network;
}


ssize_t fd_read(void *ctx, void *buf, size_t nbytes) {
    return read(*(int *)ctx, buf, nbytes);
}

ssize_t fd_write(void *ctx, const void *buf, size_t nbytes) {
    return write(*(int *)ctx, buf, nbytes);
}

io_t *accept_conncetion_no_tls(network_t *network) {
    io_t *io = io_init();

    *(int *)(io->ctx) = accept(network->srv, NULL, NULL);
    io->read=fd_read;
    io->write=fd_write;
    io->shutdown = NULL;

    return io;
}

io_t *accept_connection(network_t *network) {
    return accept_conncetion_no_tls(network);
}

int respond(io_t *io, char *bytes, size_t len) {
    size_t total_written = 0;

    while (total_written < len) {
        int written = io->write(io->ctx, bytes + total_written, len - total_written);

        if (written < 0) {
            return 1;
        }

        total_written += written;
    }

    return 0;
}

void network_destroy(network_t *network) {
    if (!network) {
        return;
    }

    close(network->srv);

    free(network);
}
