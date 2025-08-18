#pragma once

typedef struct response response_t;

int template_available(int status);
int set_html_template_body(response_t *response);
