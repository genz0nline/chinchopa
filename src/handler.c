#include "handler.h"
#include "http/parse.h"
#include "http/req.h"
#include "http/resp.h"
#include "sys/net.h"
#include <stdint.h>

int handle_connection(int client, options_t *options) {
    int failed;

    request_t *request = get_request(client);
    if (!request) {
        return 1;
    }

    print_parsed_request(request);

    // response_t *response = form_response(request);
    request_destroy(request);
    // if (!response) {
    //     return 1;
    // }

    // size_t bytes_len;
    // uint8_t *bytes = NULL;
    // failed = serialize_response(response, &bytes_len, &bytes);
    // response_destroy(response);
    // if (failed) {
    //     free(bytes);
    //     return 1;
    // }
    //
    // failed = respond(client, bytes, bytes_len);
    // free(bytes);

    return failed;
}
