#include <asm-generic/socket.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "net.h"
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

int accept_connection(network_t *network) {
    return accept(network->srv, NULL, NULL);
}

int respond(int client, uint8_t *bytes, size_t len) {

}

void network_destroy(network_t *network) {
    if (!network) {
        return;
    }

    close(network->srv);

    free(network);
}
