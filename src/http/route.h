#pragma once

typedef struct request request_t;
typedef struct options options_t;

int validate_uri(request_t *request);
char *get_file_path(options_t *options, request_t *request);
