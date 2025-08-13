#include "server.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

    server_t *server = server_init(argc, argv);

    if (!server) {
        return 1;
    }

    int err = server_run(server);
    server_destroy(server);

    return err;
}
