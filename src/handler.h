#pragma once

typedef struct options options_t;
typedef struct conn conn_t;

int handle_connection(conn_t *conn, options_t *options);
