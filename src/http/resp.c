#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../utils/opt.h"
#include "../utils/log.h"
#include "../utils/err.h"
#include "../sys/dsk.h"
#include "headers.h"
#include "req.h"
#include "resp.h"
#include "route.h"
#include "status.h"
#include "tmpl.h"


int validate_version(request_t *request) {
    if (request->major_version != 1)
        return 1;

    if (request->minor_version > 1)
        return 1;

    return 0;
}

response_t *finalize_response(response_t *response) {
    response->major_version = 1;
    response->minor_version = 1;

    if (set_reason_phrase(response)) {
        response_destroy(response);
        return NULL;
    }

    if (template_available(response->status)) {
        if (set_html_template_body(response)) {
            response_destroy(response);
            return NULL;
        }
    }

    if (set_content_length_header(response)) {
        response_destroy(response);
        return NULL;
    }

    return response;
}

response_t *response_init() {
    response_t *response = (response_t *)malloc(sizeof(response_t));

    response->h_len = 0;
    response->h_size = 0;
    response->headers = NULL;
    response->body = NULL;
    response->content_length = 0;

    return response;
}

response_t *form_response(options_t *options, request_t *request) {
    response_t *response = response_init();

    if (set_default_resp_headers(response)) {
        response_destroy(response);
        return NULL;
    }

    if (validate_version(request)) {
        response->status = 505;
        return finalize_response(response);
    }

    char *file_path = get_file_path(options, request);

    if (
        file_path == NULL ||
        read_file(response, file_path)
    ) {
        switch (errno) {
            case (EACCES):
                response->status = 403;
                break;
            default:
                response->status = 404;
                break;
        }
        free(file_path);
        return finalize_response(response);
    }

    response->status = 200;
    free(file_path);
    return finalize_response(response);
}

#define PROTO_LEN       8
#define STATUS_LEN      3

int serialize_response(response_t *response, size_t *len, char **bytes) {
    int status_line_len = snprintf(
        NULL, 0, "HTTP/%d.%d %d %s\r\n",
        response->major_version, response->minor_version,
        response->status, response->reason_phrase ? response->reason_phrase : ""
    );

    int bytes_len = status_line_len;

    for (int i = 0; i < response->h_len; i++) {
        bytes_len += strlen(response->headers[i].name) + 
            2 + // ": "
            strlen(response->headers[i].value) +
            2; // CRLF
    }

    bytes_len += 2 + // CRLF after headers
                 response->content_length;

    *bytes = (char *)malloc(bytes_len + 1);
    if (!(*bytes)) {
        return 1;
    }
    *len = bytes_len;

    char *p = *bytes;

    sprintf(p, "HTTP/%d.%d %d %s\r\n",
             response->major_version,
             response->minor_version,
             response->status,
             response->reason_phrase != NULL ? response->reason_phrase : "");

    p += strlen(p);


    int cp_len;
    for (int i = 0; i < response->h_len; i++) {
        cp_len = strlen(response->headers[i].name);
        memcpy(p, response->headers[i].name, cp_len);
        p += cp_len;

        memcpy(p, ": ", 2);
        p += 2;

        cp_len = strlen(response->headers[i].value);
        memcpy(p, response->headers[i].value, cp_len);
        p += cp_len;

        memcpy(p, "\r\n", 2);
        p += 2;
    }

    memcpy(p, "\r\n", 2);
    p += 2;

    if (response->content_length > 0) {
        memcpy(p, response->body, response->content_length);
        p += response->content_length;
    }

    *p = '\0';

    return 0;
}

void response_destroy(response_t *response) {
    for (int i = 0; i < response->h_len; i++) {
        if ((response->headers[i].flags & HF_OWN_NAME) == HF_OWN_NAME) {
            free(response->headers[i].name);
        }

        if ((response->headers[i].flags & HF_OWN_VALUE) == HF_OWN_VALUE) {
            free(response->headers[i].value);
        }
    }
    free(response->headers);
    free(response->body);
    free(response);
}
