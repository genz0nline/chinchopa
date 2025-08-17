#include "io.h"
#include <stdlib.h>

io_t *io_init() {
    io_t *io = malloc(sizeof(io_t));

    if (!io) {
        return NULL;
    }

    io->ctx = malloc(sizeof(int));
    if (!io) {
        free(io);
        return NULL;
    }

    return io;
}

void io_destroy(io_t *io) {
    if (!io) {
        return;
    }

    free(io->ctx);
    free(io);
}
