#pragma once

typedef enum {
    REQ_LINE,
    HEADERS,
    BODY,
    FINISHED,
} state_t;

typedef struct request request_t;

typedef struct parser {
    request_t *request;
    int read_idx;
    state_t state;
} parser_t;

parser_t *parser_init(request_t *request);
void print_parsed_request(request_t *request);
int parse_chunk(parser_t *parser);
void parser_destroy(parser_t *parser);
