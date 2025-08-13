#include "headers.h"
#include "../utils/utils.h"
#include "../utils/log.h"

#include "req.h"
#include "resp.h"
#include <stdio.h>
#include <stdlib.h>

int process_content_length_header(req_header_t *header, request_t *request) {
    request->content_length = atoi(request->bytes + header->value_pos);
    return 0;
}

int process_headers_semantics(request_t *request) {
    for (int i = 0; i < request->h_len; i++) {
        req_header_t *header = request->headers + i;

        if (strcicmp(request->bytes + header->name_pos, "Content-Length") == 0) {
            if (process_content_length_header(header, request)) {
                return 1;
            }
        }
        /* In future more strcicmp my appear here */

    }

    return 0;
}

int add_resp_header(response_t *response, char *name, char *value, uint8_t flags) {
    if (response->h_len == response->h_size) {
        response->h_size = response->h_size * 2 + 1;
        resp_header_t *new_headers = realloc(response->headers,
                                             sizeof(resp_header_t) * response->h_size);
        if (!new_headers) {
            return 1;
        }

        response->headers = new_headers;
    }

    resp_header_t new_header;
    new_header.name = name;
    new_header.value = value;
    new_header.flags = flags;

    response->headers[response->h_len++] = new_header;

    return 0;
}

int set_default_resp_headers(response_t *response) {
    if (add_resp_header(response, "Connection", "close", 0)) {
        return 1;
    }

    return 0;
}

int set_content_length_header(response_t *response) {
    size_t len = snprintf(NULL, 0, "%d", response->content_length);
    if (len < 0) {
        return 1;
    }

    char *cl = malloc(len + 1);
    if (!cl) {
        return 1;
    }

    if (sprintf(cl, "%d", response->content_length) < 0) {
        return 1;
    }

    if (add_resp_header(response, "Content-Length", cl, HF_OWN_VALUE)) {
        free(cl);
        return 1;
    }

    return 0;
}
