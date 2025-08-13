#pragma once


#include <stdint.h>
typedef struct request request_t;
typedef struct response response_t;

int process_headers_semantics(request_t *request);
int add_resp_header(response_t *response, char *name, char *value, uint8_t flags);
int set_default_resp_headers(response_t *response);
int set_content_length_header(response_t *response);
