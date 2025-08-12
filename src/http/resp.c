#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "resp.h"
#include "../utils/log.h"

response_t *form_response(request_t *request) {
    response_t *response = (response_t *)malloc(sizeof(response_t));


    response->status = 200;
    response->reason_phrase = "Ok";
    response->major_version = 1;
    response->minor_version = 1;

    response->headers = (resp_header_t *)malloc(sizeof(resp_header_t) * 2);
    response->headers[0].name = "Connection";
    response->headers[0].value = "close";
    response->headers[1].name = "Content-Length";
    response->headers[1].value = "0";
    response->h_len = 2;
    response->h_size = 2;

    response->body = NULL;
    response->content_length = 0;

    return response;
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
    free(response->headers);
    free(response->body);
    free(response);
}
