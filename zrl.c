#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef __GNUC__
#define fgetc  getc_unlocked
#define fputc  putc_unlocked
#define feof   feof_unlocked
#define ferror ferror_unlocked
#endif

int read_byte(const char *path, FILE *file) {
    int c = fgetc(file);
    if (c == EOF) {
        int err = ferror(file);
        if (err || !feof(file)) {
            fprintf(stderr, "Error reading '%s': %s\n", path, strerror(err));
            exit(1);
        }
    }
    return c;
}

void write_byte(int c) {
    if (c == EOF) {
        fprintf(stderr, "Programmer error: write_byte(EOF) called\n");
        exit(1);
    }
    int r = fputc(c, stdout);
    if (r != c) {
        int err = ferror(stdout);
        fprintf(stderr, "Error writing to stdout: %s\n", strerror(err));
        exit(1);
    }
}

uint64_t read_leb128(const char *path, FILE *file) {
    uint64_t n = 0;
    for (int shift = 0;; shift += 7) {
        int c = read_byte(path, file);
        if (c == EOF) {
            fprintf(stderr, "Premature end of file '%s'", path);
            exit(1);
        }
        n |= (c & 0x7f) << shift;
        if (!(c & 0x80)) return n;
    }
}

void write_leb128(uint64_t n) {
    while (1) {
        uint8_t c = n & 0x7f;
        uint8_t more = (n >>= 7) != 0;
        c |= more << 7;
        write_byte(c);
        if (!more) return;
    }
}

void encode(const char *path, FILE *file) {
    while (1) {
        int c = read_byte(path, file);
    top:
        if (c == EOF) return;
        write_byte(c);
        if (c == 0) {
            for (uint64_t z = 0;; z++) {
                c = read_byte(path, file);
                if (c) {
                    write_leb128(z);
                    goto top;
                }
            }
        }
    }
}

void decode(const char *path, FILE *file) {
    while (1) {
        int c = read_byte(path, file);
        if (c == EOF) return;
        write_byte(c);
        if (c == 0) {
            uint64_t z = read_leb128(path, file);
            while (z-->0) write_byte(0);
        }
    }
}

#define zrle 'E'
#define zrld 'D'

void zrlf(const char *path, FILE *file) {
#if ZRLF == zrle
    encode(path, file);
#elif ZRLF == zrld
    decode(path, file);
#else
#error "ZRLF C preprocessor variable must be 'zlre' or 'zrld'"
#endif
}

int main(int argc, char *argv[]) {
    // if no arguments, process stdin
    if (argc == 1) {
        zrlf("<stdin>", stdin);
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
        zrlf(path, file);
        if (fclose(file)) {
            fprintf(stderr, "Error closing '%s': %s\n", path, strerror(errno));
            exit(1);
        }
    }
    return 0;
}
