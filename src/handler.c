#include "handler.h"
#include "http/parse.h"
#include "http/req.h"
#include "http/resp.h"
#include "sys/net.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "sys/io.h"

int handle_connection(io_t *io, options_t *options) {
    int failed;

    request_t *request = get_request(io);
    if (!request) {
        if (io->shutdown)
            io->shutdown(io->ctx);
        close(*(int *)io->ctx);
        io_destroy(io);
        return 1;
    }

    log_parsed_request(request);

    response_t *response = form_response(options, request);
    request_destroy(request);
    if (!response) {
        if (io->shutdown)
            io->shutdown(io->ctx);
        close(*(int *)io->ctx);
        io_destroy(io);
        return 1;
    }

    size_t bytes_len;
    char *bytes = NULL;
    failed = serialize_response(response, &bytes_len, &bytes);
    response_destroy(response);
    if (failed) {
        free(bytes);
        if (io->shutdown)
            io->shutdown(io->ctx);
        close(*(int *)io->ctx);
        io_destroy(io);
        return 1;
    }

    failed = respond(io, bytes, bytes_len);
    free(bytes);
    if (io->shutdown)
        io->shutdown(io->ctx);
    close(*(int *)io->ctx);

    io_destroy(io);

    return failed;
}
