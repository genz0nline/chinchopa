#pragma once

#include <sys/types.h>
#include <openssl/ssl.h>

typedef struct io {
    void *ctx;
    ssize_t (* read)(void *ctx, void *buf, size_t n);
    ssize_t (* write)(void *ctx, const void *buf, size_t n);
    int (* shutdown)(void *ctx);
} io_t;

io_t *io_init();
void io_destroy(io_t *io);
