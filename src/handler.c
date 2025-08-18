#include "handler.h"
#include "http/parse.h"
#include "http/req.h"
#include "http/resp.h"
#include "sys/net.h"
#include "utils/log.h"
#include <openssl/ssl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "sys/io.h"
#include "http/conn.h"

int handle_connection(conn_t *conn, options_t *options) {
    int failed;

    request_t *request = get_request(conn->io);
    if (!request) {
        if (conn->io->shutdown)
            conn->io->shutdown(conn->io->ctx);
        if (conn->ssl)
            SSL_free(conn->ssl);
        close(conn->cliend_fd);
        io_destroy(conn->io);
        return 1;
    }

    log_parsed_request(request);

    response_t *response = form_response(options, request);
    request_destroy(request);
    if (!response) {
        if (conn->io->shutdown)
            conn->io->shutdown(conn->io->ctx);
        if (conn->ssl)
            SSL_free(conn->ssl);
        close(conn->cliend_fd);
        io_destroy(conn->io);
        return 1;
    }

    size_t bytes_len;
    char *bytes = NULL;
    failed = serialize_response(response, &bytes_len, &bytes);
    response_destroy(response);
    if (failed) {
        free(bytes);
        if (conn->io->shutdown)
            conn->io->shutdown(conn->io->ctx);
        if (conn->ssl)
            SSL_free(conn->ssl);
        close(conn->cliend_fd);
        io_destroy(conn->io);
        return 1;
    }

    failed = respond(conn->io, bytes, bytes_len);
    free(bytes);

    if (conn->io->shutdown) {
        conn->io->shutdown(conn->io->ctx);
    }
    close(conn->cliend_fd);
    io_destroy(conn->io);
    return failed;
}
