#include "headers.h"
#include "../utils/utils.h"

#include "req.h"
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
