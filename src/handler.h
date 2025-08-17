#pragma once

typedef struct options options_t;
typedef struct io io_t;

int handle_connection(io_t *io, options_t *options);
