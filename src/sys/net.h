#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct network network_t;
typedef struct options options_t;
typedef struct io io_t;
typedef struct conn conn_t;

network_t *network_init(options_t *options);
conn_t *accept_connection(network_t *network);
int respond(io_t *io, char *bytes, size_t len);
void network_destroy(network_t *network);
