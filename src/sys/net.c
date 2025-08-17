#include <asm-generic/socket.h>
#include <openssl/evp.h>
#include <openssl/prov_ssl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <openssl/ssl.h>

#include "net.h"
#include "../http/conn.h"
#include "../utils/opt.h"
#include "../utils/log.h"
#include "../utils/err.h"
#include "io.h"

typedef struct network {
    int srv;
    SSL_CTX *tls;
} network_t;

network_t *network_init(options_t *options) {
    network_t *network = (network_t *)malloc(sizeof(network_t));
    network->tls = NULL;

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

    if (options->use_ssl) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        SSL_CTX *tls = SSL_CTX_new(TLS_server_method());
        SSL_CTX_set_min_proto_version(tls, TLS1_2_VERSION);
        SSL_CTX_use_certificate_file(tls, options->ssl_cert, SSL_FILETYPE_PEM);
        SSL_CTX_use_PrivateKey_file(tls, options->ssl_key, SSL_FILETYPE_PEM);

        log_printf("Initialized TLS\n");
        network->tls = tls;
    }

    return network;
}

ssize_t ssl_read(void *ctx, void *buf, size_t nbytes) {
    return SSL_read((SSL *)ctx, buf, (int) nbytes);
}

ssize_t ssl_write(void *ctx, const void *buf, size_t nbytes) {
    return SSL_write((SSL *)ctx, buf, (int) nbytes);
}

int ssl_shutdown(void *ctx) {
    SSL_shutdown((SSL *)ctx);
    return 0;
}

ssize_t fd_read(void *ctx, void *buf, size_t nbytes) {
    return read(*(int *)ctx, buf, nbytes);
}

ssize_t fd_write(void *ctx, const void *buf, size_t nbytes) {
    return write(*(int *)ctx, buf, nbytes);
}

conn_t *accept_connection(network_t *network) {
    conn_t *conn = malloc(sizeof(conn_t));
    io_t *io = io_init();

    int client_fd = accept(network->srv, NULL, NULL);

    if (network->tls) {
        log_printf("Establishing new TLS connections");
        SSL *ssl = SSL_new(network->tls);
        SSL_set_fd(ssl, client_fd);
        if (SSL_accept(ssl) <= 0) {
            log_printf("\tEstablishing new TLS failed");
            close(client_fd);
            io_destroy(io);
            free(conn);
            return NULL;
        }
        log_printf("\tEstablishing new TLS succeeded");

        conn->ssl=ssl;
        io->ctx = ssl;
        io->read=ssl_read;
        io->write=ssl_write;
        io->shutdown=ssl_shutdown;
    } else {
        conn->ssl = NULL;
        *(int *)io->ctx = client_fd;
        io->read=fd_read;
        io->write=fd_write;
        io->shutdown = NULL;
    }

    conn->io = io;
    conn->cliend_fd = client_fd;
    return conn;
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
    if (network->tls) {
        free(network->tls);
    }

    free(network);
}
