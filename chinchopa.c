#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*** data ***/

#define ARGS_FLG_HELP   001
#define ARGS_FLG_DIR    002

// reserved flags
#define ARGS_FLG_RES1   004
#define ARGS_FLG_RES2   010
#define ARGS_FLG_RES3   020
#define ARGS_FLG_RES4   040
#define ARGS_FLG_RES5   0100
#define ARGS_FLG_RES6   0200

struct state {
    char *root;
    uint8_t arg_flags;
};

struct state S;

int parse_args(int argc, char *argv[]);
void init_flags();

int main(int argc, char *argv[]) {

    init_flags();

    if (parse_args(argc, argv)) {
        fprintf(stderr, "Invalid command-line arguements\n");
        exit(2);
    }

    if (S.arg_flags & ARGS_FLG_HELP) {
        fprintf(stderr, "Help message\n");
    }

    if (S.arg_flags & ARGS_FLG_DIR) {
        fprintf(stderr, "Directory: %s\n", S.root);
    }

    return 0;

}

void init_flags() {
    S.arg_flags = 0;
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
