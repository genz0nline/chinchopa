#include <ctype.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

/*** data ***/

#define ARGS_FLG_HELP   001
#define ARGS_FLG_DIR    002
#define ARGS_FLG_PORT   004

// reserved flags
#define ARGS_FLG_RES2   010
#define ARGS_FLG_RES3   020
#define ARGS_FLG_RES4   040
#define ARGS_FLG_RES5   0100
#define ARGS_FLG_RES6   0200

struct state {
    char *root;
    uint8_t arg_flags;
    uint16_t port;
    int server_socket;
};

struct state S;

int parse_args(int argc, char *argv[]);
void state_init();
void print_help();
void server_start();
void die(char *s);
void die_msg(char *s);

int main(int argc, char *argv[]) {

    state_init();

    if (parse_args(argc, argv)) {
        die_msg("parse_args: Invalid command-line arguements");
    }

    if (S.arg_flags & ARGS_FLG_HELP) {
        print_help();
        exit(0);
    }

    server_start();

    return 0;
}

void die_msg(char *s) {
    fprintf(stderr, "%s\n", s);
    exit(1);
}

void die(char *s) {
    fprintf(stderr, "%s: %s\n", s, strerror(errno));
    exit(1);
}

#define DEFAULT_PORT        80

typedef enum http_method {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS,
    CONNECT,
    TRACE,
    UNKNOWN = -1
} http_method;

typedef struct header {
    char *name;
    char *value;
} header;

typedef struct request {
    int method;
    char *uri;
    int minor_version;
    int major_version;

    header *headers;
    int headers_capacity;
    int headers_len;

    char *body;
    int content_length;
} request;

typedef struct response {
    int minor_version;
    int major_version;
    int status;
    char *reason_phrase;
    
    header *headers;
    int headers_capacity;
    int headers_len;

    char *body;
} response;

#define REQUEST_LINE    0
#define HEADERS         1
#define BODY            2

request *init_request() {
    request *req = malloc(sizeof(request));

    req->method = UNKNOWN;
    req->uri = NULL;
    req->headers = (header *)malloc(4 * sizeof(header));
    req->headers_len = 0;
    req->headers_capacity = 4;
    req->body = NULL;
    req->content_length = 0;

    return req;
}



struct method {
    char *s;
    http_method method;
};

#define METHODS_NUM     9

const struct method methods[METHODS_NUM] = {
    {"GET", GET},
    {"POST", POST},
    {"PUT", PUT},
    {"DELETE", DELETE},
    {"PATCH", PATCH},
    {"HEAD", HEAD},
    {"OPTIONS", OPTIONS},
    {"CONNECT", CONNECT},
    {"TRACE", TRACE},
};


int parse_request_line(char *buf, request *req) {
    char *method = strtok(buf, " ");
    int len = strlen(method);

    for (int i = 0; i < METHODS_NUM; i++) {
        if (strcmp(method, methods[i].s) == 0) {
            req->method = methods[i].method;
            break;
        }
    }

    if (req->method == UNKNOWN) {
        return 1;
    }

    char *uri = strtok(NULL, " ");
    if (uri == NULL) {
        return 1;
    }
    req->uri = malloc(strlen(uri) + 1);
    memcpy(req->uri, uri, strlen(uri) + 1);

    char *version = strtok(NULL, " ");
    if (version == NULL) {
        return 1;
    }

    char *protocol = strtok(version, "/");
    if (protocol == NULL) {
        return 1;
    }

    if (strcmp(protocol, "HTTP")) {
        return 1;
    }

    char *major_version = strtok(NULL, ".");
    if (major_version == NULL) {
        return 1;
    }
    req->major_version = atoi(major_version);

    char *minor_version = strtok(NULL, ".");
    if (minor_version == NULL) {
        return 1;
    }
    req->minor_version = atoi(minor_version);

    return 0;
}

void free_request(request *req) {
    if (req == NULL) return;
    free(req->uri);
    for (int i = 0; i < req->headers_len; i++) {
        header *h = req->headers + i;
        free(h->name);
        free(h->value);
    }
    free(req->headers);
    free(req->body);
    free(req);
}

int parse_header(char *buf, request *req) {
    int len = strlen(buf);
    while (isblank(buf[len-1])) {
        buf[len-1] = '\0';
        len--;
    }

    if (req->headers_len == req->headers_capacity) {
        req->headers_capacity *= 2;
        header *new_headers = realloc(req->headers, sizeof(header) * req->headers_capacity);
        if (!new_headers) {
            free_request(req);
            free(buf);
            die("realloc");
        }
        req->headers = new_headers;
    }

    header new_header;

    char *name = strtok(buf, ":");
    if (isblank(name[strlen(name) - 1])) return 1;
    new_header.name = malloc(strlen(name) + 1);
    memcpy(new_header.name, name, strlen(name) + 1);

    for (int i = 0; i < strlen(new_header.name); i++) {
        new_header.name[i] = tolower(new_header.name[i]);
    }

    char *value = strtok(NULL, ":");
    if (value == NULL) {
        return 1;
    }
    while (isblank(*value)) value++;

    new_header.value = malloc(strlen(value) + 1);
    memcpy(new_header.value, value, strlen(value) + 1);

    req->headers[req->headers_len++] = new_header;

    return 0;
}

#define CONTENT_LENGTH          "content-length"


int derive_content_length(request *req) {
    for (int i = 0; i < req->headers_len; i++) {
        if (strcmp(req->headers[i].name, CONTENT_LENGTH) == 0) {
            req->content_length = atoi(req->headers[i].value);
            return req->content_length;
        }
    }

    return 0;
}

request *parse_request(int client) {

    size_t buf_size = 128;
    size_t buf_len = 0;
    char *buf = malloc(buf_size);

    int state = REQUEST_LINE;

    char c, prev = 0;

    request *req = init_request();

    while (read(client, &c, 1)) {
        if (c == '\n' && prev == '\r') {
            if (state == REQUEST_LINE) { // end of request line
                buf[buf_len - 1] = '\0';
                if (parse_request_line(buf, req)) {
                    free_request(req);
                    free(buf);
                    return NULL;
                }
                state = HEADERS;
            } else if (state == HEADERS && buf_len == 1) { // end of empty line after headers
                if (derive_content_length(req) > 0) {
                    state = BODY;
                }
                buf_len = 0;
                break;
            } else if (state == HEADERS) { // end of a header
                buf[buf_len - 1] = '\0';
                parse_header(buf, req);
            }
            buf_len = 0;
        } else {
            if (buf_len == buf_size) {
                buf_size *= 2;
                buf = realloc(buf, buf_size);
            }
            buf[buf_len++] = c;
        }

        prev = c;
    }

    if (state == BODY) {
        while (buf_len < req->content_length) {
            int r = read(client, &c, 1);
            if (r <= 0) {
                free_request(req);
                free(buf);
                return NULL;
            }

            if (buf_len == buf_size) {
                buf_size *= 2;
                buf = realloc(buf, buf_size);
            }
            buf[buf_len++] = c;
        }

        req->body = malloc(req->content_length);
        memcpy(req->body, buf, req->content_length);
    }

    free(buf);

    return req;
}

response *form_response(request *req) {
    if (req == NULL) {
        // TODO: Bad request 400
    }
    return NULL;
}

void free_response(response *resp) {
    if (resp == NULL) return;
    free(resp->reason_phrase);
    for (int i = 0; i < resp->headers_len; i++) {
        header *h = resp->headers + i;
        free(h->name);
        free(h->value);
    }
    free(resp->headers);
    free(resp->body);
    free(resp);
}

int serialize_response(response *resp, char **serialized) {
    return 0;
}

int respond(int client, char *resp, size_t len) {
    if (write(client, resp, len) != len) return 1;
    return 0;
}

void print_request(request *req) {
    printf("=== Request Line Parsed ===\n");
    printf("method: ");
    for (int i = 0; i < METHODS_NUM; i++) {
        if (req->method == methods[i].method) printf("%s\n", methods[i].s);
    }

    printf("uri: \"%s\"\n", req->uri);
    printf("Major version: %d\n", req->major_version);
    printf("Minor version: %d\n", req->minor_version);

    printf("=== Headers Parsed ===\n");

    for (int i = 0; i < req->headers_len; i++) {
        printf("%s: %s\n", req->headers[i].name, req->headers[i].value);
    }

    if (req->body) {
        printf("=== Body ===\n");
        for (int i = 0; i < req->content_length; i++) {
            printf("%c", req->body[i]);
        }
        printf("\n");
    }
}

void handle_request(int client) {
    request *req = parse_request(client);
    print_request(req);

    // response *resp = form_response(req);
    free_request(req);

    // char *serialized_response;
    // size_t serialized_response_len = serialize_response(resp, &serialized_response);
    // free_response(resp);
    //
    // if (respond(client, serialized_response, serialized_response_len)) {
    //     free(serialized_response);
    //     die("write");
    // }
    // free(serialized_response);
}

void server_exit() {
    close(S.server_socket);
}

void handle_sig(int sig) {
    exit(sig);
}

void server_start() {
    S.server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (S.server_socket < 0) {
        die("socket");
    }

    atexit(server_exit);
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);
    signal(SIGHUP, handle_sig);
    signal(SIGQUIT, handle_sig);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(S.port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    if (setsockopt(S.server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        die("setsockopt");
    }

    if (bind(S.server_socket, (struct sockaddr *)&addr, sizeof(addr))) {
        die("bind");
    }

    if (listen(S.server_socket, SOMAXCONN)) {
        die("listen");
    }

    printf("Accepting connections on %d port.\n", S.port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t size = sizeof(client_addr);
        int client = accept(S.server_socket, (struct sockaddr *)&client_addr, &size);
        if (client < 0) {
            die("accept");
        }

        handle_request(client);
        close(client);
    }
}

void print_help() {
    printf("Usage: chinchopa [-h] [-d directory]\n");
}

void state_init() {
    S.arg_flags = 0;
    S.root = NULL;
    S.port = 80;
}

int parse_args(int argc, char *argv[]) {
    for (int i = 1; i < argc; i += 2) {
        if (argv[i][0] == '-') {
            if (strlen(argv[i]) == 2) {
                switch (argv[i][1]) {
                    case 'd':
                        S.arg_flags |= ARGS_FLG_DIR;
                        if (i + 1 >= argc) return 1;
                        S.root = argv[i+1];
                        break;
                    case 'p':
                        S.arg_flags |= ARGS_FLG_PORT;
                        if (i +1 >= argc) return 1;
                        S.port = (uint16_t) atoi(argv[i+1]);
                        break;
                    case 'h':
                        S.arg_flags |= ARGS_FLG_HELP;
                        break;
                }
            } else {
                return 1;
            }
        }
    }

    return 0;
}
