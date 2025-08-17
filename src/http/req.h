#pragma once

#include <stddef.h>
typedef enum {
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    HEAD,
    OPTIONS,
    CONNECT,
    TRACE,
} method_t;

struct method {
    method_t method;
    char *method_s;
};

typedef struct req_header {
    int name_pos;
    int value_pos;
} req_header_t;

typedef struct request {
    char *bytes;
    int bytes_len;

    method_t method;

    int uri_pos;
    int major_version;
    int minor_version;

    req_header_t *headers;
    size_t h_len;
    size_t h_size;

    int body_pos;
    int content_length;
} request_t;

typedef struct io io_t;

request_t *get_request(io_t *io);
void request_destroy(request_t *request);
