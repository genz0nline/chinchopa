#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "req.h"
#include "../utils/err.h"
#include "parse.h"

#define START_BUF_SIZE      256
#define CHUNK_SIZE          128 
#define REQUEST_SIZE_CAP    (16 * 1024 * 1024)

request_t *get_request(int client) {
    request_t *request = (request_t *)malloc(sizeof(request_t));
    if (!request) 
        return NULL;

    request->content_length = 0;
    request->headers = NULL;
    request->h_len = 0;
    request->h_size = 0;

    parser_t *parser = parser_init(request);

    request->bytes = malloc(START_BUF_SIZE);
    int size = START_BUF_SIZE;
    int len = 0;

    int read_bytes;
    while ((read_bytes = read(client, request->bytes + len, CHUNK_SIZE)) > 0) {
        len += read_bytes;
        request->bytes[len] = '\0';

        if (parse_chunk(parser)) {
            parser_destroy(parser);
            request_destroy(request);
            return NULL;
        }

        if (len > REQUEST_SIZE_CAP) {
            seterr(REQCAP);
            request_destroy(request);
            return NULL;
        }

        if (parser->state == FINISHED) {
            break;
        }

        if (size - len < CHUNK_SIZE + 1) {
            size *= 2;
            char *new_bytes = realloc(request->bytes, size);
            if (!new_bytes) {
                parser_destroy(parser);
                request_destroy(request);
                return NULL;
            }

            request->bytes = new_bytes;
        }
    }

    if (read_bytes < 0) {
        parser_destroy(parser);
        request_destroy(request);
        return NULL;
    }

    request->bytes_len = len;
    parser_destroy(parser);
    return request;
}

void request_destroy(request_t *request) {
    if (request->headers) {
        free(request->headers);
        request->headers = NULL;
    }

    if (request->bytes) {
        free(request->bytes);
        request->bytes = NULL;
    }
    free(request);
}
