#include "tmpl.h"
#include "resp.h"
#include <stdio.h>
#include <stdlib.h>

#define TEMPLATE    "<html><head><title>%d %s</title></head><body>" \
                    "<h1 style=\"width: fit-content; margin-right: auto; margin-left: auto\">%d %s</h1>" \
                    "<hr />" \
                    "<h6 style=\"width: fit-content; margin-right: auto; margin-left: auto\">chinchopa v0.0.1</h6>" \
                    "</body></html>"

int template_available(int status) {
    if (
        status == 404 ||
        status == 403 ||
        status == 500 ||
        status == 505
    ) {
        return 1;
    }

    return 0;
}

int set_html_template_body(response_t *response) {
    int len = snprintf(NULL, 0, TEMPLATE, 
                       response->status, response->reason_phrase,
                       response->status, response->reason_phrase
                       );

    response->body = malloc(len);
    if (!response->body) {
        return 1;
    }

    if ((len = sprintf(response->body, TEMPLATE,
                       response->status, response->reason_phrase,
                       response->status, response->reason_phrase
                       )) <= 0) {
        return 1;
    }

    response->content_length = len;

    return 0;
}
