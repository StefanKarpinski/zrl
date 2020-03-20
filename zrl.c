#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void encode(FILE *file) {
    // TODO: encoding
    return;
}

void decode(FILE *file) {
    // TODO: decoding
    return;
}

#define zrle 'E'
#define zrld 'D'

void zrlf(FILE *file) {
#if ZRLF == zrle
    encode(file);
#elif ZRLF == zrld
    decode(file);
#else
#error "ZRLF C preprocessor variable must be 'zlre' or 'zrld'"
#endif
}

int main(int argc, char *argv[]) {
    // if no arguments, process stdin
    if (argc == 1) {
        zrlf(stdin);
        return 0;
    }
    // process arguments
    for (int i = 1; i < argc; i++) {
        char *path = argv[i];
        FILE *file = fopen(path, "r");
        if (!file) {
            fprintf(stderr, "Error opening '%s': %s\n", path, strerror(errno));
            exit(1);
        }
        zrlf(file);
        if (fclose(file)) {
            fprintf(stderr, "Error closing '%s': %s\n", path, strerror(errno));
            exit(1);
        }
    }
    return 0;
}
