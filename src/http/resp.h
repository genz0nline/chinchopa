#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct request request_t;
typedef struct resp_header {
    char *name;
    char *value;
} resp_header_t;

typedef struct response {
    int major_version;
    int minor_version;
    int status;
    char *reason_phrase;

    resp_header_t *headers;
    size_t h_len;
    size_t h_size;

    char *body;
    int content_length;
} response_t;

response_t *form_response(request_t *request);
int serialize_response(response_t *response, size_t *len, char **bytes);
void response_destroy(response_t *response);
