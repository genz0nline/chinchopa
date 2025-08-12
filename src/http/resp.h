#pragma once

#include <stddef.h>
#include <stdint.h>
typedef struct response response_t;
typedef struct request request_t;

response_t *form_response(request_t *request);
int serialize_response(response_t *response, size_t *len, uint8_t **bytes);
void response_destroy(response_t *response);
