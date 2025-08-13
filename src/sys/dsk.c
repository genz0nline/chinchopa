#include "dsk.h"

#include "../http/resp.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define START_BUF_SIZE      1024 * 2
#define CHUNK_SIZE          1024

int read_file(response_t *response, char *file_path) {
    FILE *f = fopen(file_path, "rb");
    if (!f) {
        return 1;
    }

    size_t size = START_BUF_SIZE;
    response->body = malloc(size);
    if (!response->body) {
        fclose(f);
        return 1;
    }

    response->content_length = 0;
    for (;;) {
        if (size - response->content_length < CHUNK_SIZE) {
            size_t new_size = size * 2;
            if (new_size < size) {
                fclose(f);
                return 1;
            }
            char *new_buf = realloc(response->body, new_size);
            if (!new_buf) {
                fclose(f);
                return 1;
            }
            response->body = new_buf;
            size = new_size;
        }

        int n = fread(response->body + response->content_length, 1, CHUNK_SIZE, f);
        response->content_length += n;
        if (n < CHUNK_SIZE) {
            if (feof(f)) break;
            if (ferror(f)) {
                fclose(f);
                free(response->body);
                response->body = NULL;
                response->content_length = 0;
                return 1;
            }
        }

    }

    fclose(f);
    return 0;
}
