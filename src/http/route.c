#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "req.h"
#include "route.h"
#include "../utils/opt.h"
#include "../utils/err.h"
#include "../utils/log.h"

int validate_uri(request_t *request) {
    int level = 0;

    if (*(request->bytes + request->uri_pos) == '/') {
        request->uri_pos++;
    }

    size_t uri_len = strlen(request->bytes + request->uri_pos);

    char *uri = malloc(uri_len + 1);
    strcpy(uri, request->bytes + request->uri_pos);

    for (char *p = strtok(uri, "/"); p != NULL; p = strtok(NULL, "/")) {
        if (strcmp(p, "..") == 0) {
            level--;
        } else if (strcmp(p, ".") == 0 || strlen(p) == 0) {
            continue;
        } else {
            level++;
        }

        if (level < 0) {
            free(uri);
            return 1;
        }
    }

    free(uri);
    return 0;
}

#define INDEX_HTML      "index.html"

char *get_file_path(options_t *options, request_t *request) {
    if (validate_uri(request)) {
        seterr(INVURI);
        return NULL;
    }

    size_t max_path_len = snprintf(NULL, 0, "%s/%s/%s",
                                  options->dir,
                                  request->bytes + request->uri_pos,
                                  INDEX_HTML);

    if (max_path_len < 0) {
        return NULL;
    }

    char *path = malloc(max_path_len + 1);

    if (sprintf(path, "%s/%s", options->dir, request->bytes + request->uri_pos) < 0) {
        free(path);
        return NULL;
    }

    struct stat st;

    if (stat(path, &st)) {
        free(path);
        return NULL;
    }

    if (S_ISREG(st.st_mode)) {
        return path;
    } else if (!S_ISDIR(st.st_mode)) {
        free(path);
        return NULL;
    }

    if (path[strlen(path) - 1] == '/') {
        if (sprintf(path, "%s/%s%s",
                options->dir,
                request->bytes + request->uri_pos,
                INDEX_HTML) < 0) {
            free(path);
            return NULL;
        }
    } else {
        if (sprintf(path, "%s/%s/%s",
                options->dir,
                request->bytes + request->uri_pos,
                INDEX_HTML) < 0) {
            free(path);
            return NULL;
        }
    }

    stat(path, &st);
    if (S_ISREG(st.st_mode)) {
        return path;
    }

    free(path);
    return NULL;
}
