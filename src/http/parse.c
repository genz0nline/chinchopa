#include "headers.h"
#include "req.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/err.h"
#include "../utils/log.h"
#include "parse.h"

#define METHODS_NUM     9

struct method methods[METHODS_NUM] = {
    {GET, "GET"},
    {POST, "POST"},
    {PUT, "PUT"},
    {PATCH, "PATCH"},
    {DELETE, "DELETE"},
    {HEAD, "HEAD"},
    {OPTIONS, "OPTIONS"},
    {CONNECT, "CONNECT"},
    {TRACE, "TRACE"},
};

parser_t *parser_init(request_t *request) {
    parser_t *parser = (parser_t *)malloc(sizeof(parser_t));
    parser->request = request;
    parser->state = REQ_LINE;
    parser->read_idx = 0;

    return parser;
}

void print_parsed_request(request_t *request) {
    log_printf("=== Parsed Request ===");
    for (int i = 0; i < METHODS_NUM; i++) {
        if (request->method == methods[i].method) {
            log_printf("\t%s %s HTTP/%d.%d",
                       methods[i].method_s,
                       request->bytes + request->uri_pos,
                       request->major_version,
                       request->minor_version);
        }
    }
    for (int i = 0; i < request->h_len; i++) {
        log_printf("%s: %s",
                   request->bytes + request->headers[i].name_pos,
                   request->bytes + request->headers[i].value_pos);
    }

    if (request->content_length > 0) {
        log_printf("%s", request->bytes + request->body_pos);
    }
}

void parser_destroy(parser_t *parser) {
    free(parser);
}

int parse_request_line_fields(parser_t *parser) {
    char *line = parser->request->bytes + parser->read_idx;
    char *method = strtok(line, " ");
    if (!method) {
        return 1;
    }

    int known_method = 0;
    for (int i = 0; i < METHODS_NUM; i++) {
        if (strcmp(method, methods[i].method_s) == 0) {
            parser->request->method = methods[i].method;
            known_method = 1;
        }
    }
    if (!known_method) {
        seterr(UNKNOWN_METHOD);
        return 1;
    }

    char *route = strtok(NULL, " ");
    if (!route) {
        seterr(INVREQ);
        return 1;
    }

    parser->request->uri_pos = parser->read_idx + route - line;

    char *protocol = strtok(NULL, " ");
    if (!protocol) {
        seterr(INVREQ);
        return 1;
    }

    if (sscanf(protocol, "HTTP/%d.%d", &parser->request->major_version, &parser->request->minor_version) != 2) {
        seterr(INVVER);
        return 1;
    }

    return 0;
}

int parse_request_line(parser_t *parser) {
    char *c, *prev;

    char *line = parser->request->bytes + parser->read_idx;

    prev = NULL;
    for (c = line; *c != '\0'; c++) {
        if (*c == '\n' && prev && *prev == '\r') {
            *prev = '\0';
            parser->state = HEADERS;

            if (parse_request_line_fields(parser)) {
                return -1;
            }

            parser->read_idx = c - parser->request->bytes + 1;

            return 1;
        }
        prev = c;
    }


    return 0;
}

int parse_header_fields(parser_t *parser) { 
    req_header_t new_header;


    char *line = parser->request->bytes + parser->read_idx;
    char *name = strtok(line, ":");
    if (!name) {
        seterr(INVHEADER);
        return 1;
    }
    new_header.name_pos = parser->read_idx;

    char *value = strtok(NULL, ":");
    if (!value) {
        seterr(INVHEADER);
        return 1;
    }

    new_header.value_pos = value - parser->request->bytes;
    // Leading spaces
    while (isblank(*value)) {
        value++;
        new_header.value_pos++;
    }

    // Trailing spaces
    char *last_byte = value + strlen(value) - 1;
    while (isblank(*last_byte)) {
        *last_byte = '\0';
        last_byte--;
    }

    if (parser->request->h_len == parser->request->h_size) {
        parser->request->h_size = parser->request->h_size * 2 + 1;
        req_header_t *new_headers = (req_header_t *)realloc(parser->request->headers, sizeof(req_header_t) * parser->request->h_size);
        if (!new_headers) {
            return 1;
        }
        parser->request->headers = new_headers;
    }

    parser->request->headers[parser->request->h_len++] = new_header;

    return 0;
}

int parse_header(parser_t *parser) {
    char *line = parser->request->bytes + parser->read_idx;

    char *c, *prev;

    prev = NULL;
    for (c = line; *c != '\0'; c++) {
        if (*c == '\n' && prev && *prev == '\r') {
            *prev = '\0';
            if (c - line == 1) {
                process_headers_semantics(parser->request);
                if (parser->request->content_length > 0) {
                    parser->request->body_pos = c - parser->request->bytes + 1;
                    parser->state = BODY;
                } else {
                    parser->state = FINISHED;
                }
            } else {
                if (parse_header_fields(parser)) {
                    return -1;
                }
            }
            parser->read_idx = c - parser->request->bytes + 1;
            return 1;
        }
        prev = c;
    }

    return 0;
}

int parse_body(parser_t *parser) {
    log_printf("In parse body\n");
    if (strlen(parser->request->bytes + parser->request->body_pos) < parser->request->content_length) {
        return 0;
    }

    parser->request->bytes[parser->request->body_pos + parser->request->content_length] = '\0';
    parser->state = FINISHED;

    return 1;
}

int parse_line(parser_t *parser) {
    switch (parser->state) {
        case REQ_LINE:
            return parse_request_line(parser);
        case HEADERS:
            return parse_header(parser);
        case BODY:
            return parse_body(parser);
        case FINISHED:
            return 0;
    }
}

int parse_chunk(parser_t *parser) {
    int parsed_lines;

    while ((parsed_lines = parse_line(parser)) > 0);

    if (parsed_lines < 0) {
        return 1;
    }

    return 0;
}
