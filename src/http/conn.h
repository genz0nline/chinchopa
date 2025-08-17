#pragma once

#include <openssl/ssl.h>

typedef struct io io_t;

typedef struct conn {
    int cliend_fd;
    SSL *ssl;
    io_t *io;
} conn_t;
