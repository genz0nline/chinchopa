#include <ctype.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
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

int get_file_path(char *uri, char **file_path);

int main(int argc, char *argv[]) {

    setbuf(stdout, NULL);

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
    int content_length;
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

response *init_response() {
    response *resp = malloc(sizeof(response));

    resp->major_version = 1;
    resp->minor_version = 1;

    resp->reason_phrase = NULL;

    resp->headers = (header *)malloc(4 * sizeof(header));
    resp->headers_len = 0;
    resp->headers_capacity = 4;

    resp->body = NULL;
    resp->content_length = 0;

    return resp;
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

void free_response(response *resp) {
    if (resp == NULL) return;

    for (int i = 0; i < resp->headers_len; i++) {
        header *h = resp->headers + i;
        free(h->name);
        free(h->value);
    }
    free(resp->headers);
    free(resp->body);
    free(resp);
}

// Only support HTTP1.x
int supported_version(request *req) {
    if (req->major_version != 1) return 0;

    if (req->minor_version > 1) return 0;

    return 1;
}

struct reason {
    int status;
    char *reason_phrase;
};

struct reason reasons[] = {
    {200, "Ok"},
    {400, "Bad Request"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {500, "Server Error"},
    {505, "HTTP Version Not Supported"},
    {-1, "Unknown"},
};

char *get_reason_phrase(int status) {
    int i = 0;
    while (1) {
        if (status == reasons[i].status) {
            return reasons[i].reason_phrase;
        }
        if (reasons[i].status == -1) {
            return NULL;
        }
        i--;
    }
}

int validate_uri(char *uri) {
    if (uri == NULL)
        return 1;

    char *copy = malloc(strlen(uri) + 1);
    memcpy(copy, uri, strlen(uri) + 1);

    int level = 0;

    if (*copy++ != '/')
        return 1;

    char *p;
    int end = 0;
    for (p = copy; p < copy + strlen(copy) + 1; p++) {
        if (*p == '/' || *p == '\0') {
            if (*p == '\0') {
                end = 1;
            }
            *p = '\0';
            if (strcmp(copy, "..") == 0) {
                level--;
            } else if (strcmp(copy, ".") == 0) {
                continue;
            } else {
                level ++;
            }

            if (level < 0) {
                return 1;
            }
            if (!end) {
                copy = p + 1;
            }
        }
    }

    return 0;
}

void append(char **to, char* from) {
    int to_len = strlen(*to);
    int from_len = strlen(from);

    *to = realloc(*to, to_len + from_len + 1);
    memcpy(*to + to_len, from, from_len);
    (*to)[to_len + from_len] = '\0';
}

int get_file_path(char *uri, char **file_path) {
    *file_path = malloc(strlen(S.root) + strlen(uri) + 1);
    sprintf(*file_path, "%s%s", S.root, uri);
    (*file_path)[strlen(S.root) + strlen(uri)] = '\0';

    struct stat path_stat;
    if (stat(*file_path, &path_stat)) {
        return 1;
    }

    if (S_ISREG(path_stat.st_mode)) {
        return 0;
    } else if (S_ISDIR(path_stat.st_mode)) {
        if ((*file_path)[strlen(*file_path) - 1] != '/') {
            append(file_path, "/");
        }

        append(file_path, "index.html");
    } else {
        return 1;
    }

    if (stat(*file_path, &path_stat)) {
        return 1;
    }

    if (S_ISREG(path_stat.st_mode)) {
        return 0;
    } else {
        return 1;
    }
}

int add_header(response *resp, char *name, char *str_value, int num_value) {
    if (resp->headers_len == resp->headers_capacity) {
        resp->headers_capacity *= 2;
        resp->headers = realloc(resp->headers, sizeof(header) * resp->headers_capacity);
        if (!resp->headers){
            return 1;
        }
    }

    header new_header;
    new_header.name = malloc(strlen(name) + 1);
    memcpy(new_header.name, name, strlen(name) + 1);

    if (str_value) {
        new_header.value = malloc(strlen(str_value) + 1);
        memcpy(new_header.value, str_value, strlen(str_value) + 1);
    } else {
        new_header.value = malloc(64);
        snprintf(new_header.value, 64, "%d", num_value);
    }

    resp->headers[resp->headers_len++] = new_header;

    return 0;
}

int set_response_content(response *resp, char *file_path) {
    FILE *f = fopen(file_path, "rb");
    if (!f) {
        return 1;
    }

    int capacity = 128;
    resp->body = malloc(capacity);
    if (!resp->body) {
        fclose(f);
        return 1;
    }

    resp->content_length = 0;

    char c;
    while (fread(&c, 1, 1, f) > 0) {
        if (resp->content_length == capacity) {
            capacity *= 2;
            resp->body = realloc(resp->body, capacity);
            if (!resp->body) {
                fclose(f);
                return 1;
            }
        }

        resp->body[resp->content_length++] = c;
    }

    add_header(resp, "Content-Length", NULL, resp->content_length);

    return 0;
}

int set_close_header(response *resp) {
    if (add_header(resp, "Connection", "close", 0)) {
        return 1;
    }

    return 0;
}

response *form_response(request *req) {
    response *resp = init_response();

    char *reason_phrase;

    if (req == NULL) {
        resp->status = 400;
        resp->reason_phrase = get_reason_phrase(resp->status);
        return resp;
    }

    if (!supported_version(req)) {
        resp->status = 505;
        resp->reason_phrase = get_reason_phrase(resp->status);
        return resp;
    }

    if (req->method != GET) {
        resp->status = 405;
        resp->reason_phrase = get_reason_phrase(resp->status);
        return resp;
    }

    if (validate_uri(req->uri)) {
        resp->status = 404;
        resp->reason_phrase = get_reason_phrase(resp->status);
    }

    char *file_path;
    if (get_file_path(req->uri, &file_path)) {
        resp->status = 404;
        resp->reason_phrase = get_reason_phrase(resp->status);
    }

    if (set_response_content(resp, file_path)) {
        resp->status = 500;
        resp->reason_phrase = get_reason_phrase(resp->status);
    }

    if (set_close_header(resp)) {
        resp->status = 500;
        resp->reason_phrase = get_reason_phrase(resp->status);
    }

    resp->status = 200;
    resp->reason_phrase = get_reason_phrase(resp->status);

    return resp;
}

#define PROTO_LEN           8
#define STATUS_CODE_LEN     3

int serialize_response(response *resp, char **serialized) {

    int status_line_len = PROTO_LEN // HTTP-Version
                        + 1 // SP
                        + STATUS_CODE_LEN // status-code
                        + 1 // SP
                        + strlen(resp->reason_phrase) // reason-phrase
                        + 2 // CRLF
                        + 2 // CRLF
                        + resp->content_length; // body
    
    for (int i = 0; i < resp->headers_len; i++) {
        header h = resp->headers[i];
        status_line_len += strlen(h.name)
                         + 2 // ": "
                         + strlen(h.value)
                         + 2; // CRLF
    }

    char *buf = malloc(status_line_len * 2);

    char *p = buf;

    char proto[PROTO_LEN + 1];
    sprintf(proto, "HTTP/%d.%d", resp->major_version, resp->minor_version);
    memcpy(p, proto, PROTO_LEN);
    p += PROTO_LEN;

    memcpy(p, " ", 1);
    p++;

    char status[STATUS_CODE_LEN + 1];
    sprintf(status, "%d", resp->status);
    memcpy(p, status, STATUS_CODE_LEN);
    p += STATUS_CODE_LEN;

    memcpy(p, " ", 1);
    p++;

    memcpy(p, resp->reason_phrase, strlen(resp->reason_phrase));
    p += strlen(resp->reason_phrase);

    memcpy(p, "\r\n", 2);
    p += 2;

    for (int i = 0; i < resp->headers_len; i++) {
        header h = resp->headers[i];
        memcpy(p, h.name, strlen(h.name));
        p += strlen(h.name);

        memcpy(p, ": ", 2);
        p += 2;

        memcpy(p, h.value, strlen(h.value));
        p += strlen(h.value);

        memcpy(p, "\r\n", 2);
        p += 2;
    }

    memcpy(p, "\r\n", 2);
    p += 2;

    if (resp->content_length > 0) {
        memcpy(p, resp->body, resp->content_length);
        p += resp->content_length;
    }

    *serialized = buf;

    return p - buf;
}

int respond(int client, char *resp, size_t len) {
    if (write(client, resp, len) != len) return 1;
    return 0;
}

void print_request(request *req) {
    printf("=== Request ===\n");
    for (int i = 0; i < METHODS_NUM; i++) {
        if (req->method == methods[i].method) printf("%s ", methods[i].s);
    }

    printf("%s ", req->uri);
    printf("HTTP/%d.%d\n", req->major_version, req->minor_version);

    for (int i = 0; i < req->headers_len; i++) {
        printf("%s: %s\n", req->headers[i].name, req->headers[i].value);
    }
    printf("\n");

    if (req->body) {
        for (int i = 0; i < req->content_length; i++) {
            printf("%c", req->body[i]);
        }
        printf("\n");
    }
}

void print_response(response *resp) {
    printf("=== Response ===\n");
    printf("%d HTTP/%d.%d", resp->status, resp->major_version, resp->minor_version);

    for (int i = 0; i < resp->headers_len; i++) {
        printf("%s: %s\n", resp->headers[i].name, resp->headers[i].value);
    }
    printf("\n");
}

void handle_request(int client) {
    request *req = parse_request(client);
    print_request(req);

    response *resp = form_response(req);
    print_response(resp);
    free_request(req);

    char *serialized_response = NULL;
    size_t serialized_response_len = serialize_response(resp, &serialized_response);
    free_response(resp);

    if (respond(client, serialized_response, serialized_response_len)) {
        free(serialized_response);
        die("write");
    }

    free(serialized_response);
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
                        printf("S.root = %s\n", S.root);
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
