#pragma once

typedef struct server server_t;
typedef struct options options_t;

server_t *server_init(int argc, char *argv[]);
int server_run(server_t *server);
void server_destroy(server_t *server);
