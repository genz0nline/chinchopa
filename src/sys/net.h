#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct network network_t;
typedef struct options options_t;

network_t *network_init(options_t *options);
int accept_connection(network_t *network);
int respond(int client, char *bytes, size_t len);
void network_destroy(network_t *network);
