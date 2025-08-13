#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct request request_t;
typedef struct options options_t;

enum {
    HF_OWN_NAME  = 1 << 1,
    HF_OWN_VALUE = 1 << 0,
};

typedef struct resp_header {
    char *name;
    char *value;
    uint8_t flags;
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

response_t *form_response(options_t *options, request_t *request);
int serialize_response(response_t *response, size_t *len, char **bytes);
void response_destroy(response_t *response);
