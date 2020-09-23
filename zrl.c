#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifdef IO_UNLOCKED // UNIX systems
#define fgetc getc_unlocked
#define fputc putc_unlocked
#define feof feof_unlocked
#define ferror ferror_unlocked
#elif IO_NOLOCK // Windows systems
#define fgetc _getc_nolock
#define fputc _putc_nolock
#endif

static int fpeek(FILE *file) {
    int c = fgetc(file);
    ungetc(c, file);
    return c;
}

static int read_byte(const char *path, FILE *file) {
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

static void write_byte(int c) {
    if (c == EOF) {
        fprintf(stderr, "Internal error: write_byte(EOF) called\n");
        exit(1);
    }
    int r = fputc(c, stdout);
    if (r != c) {
        int err = ferror(stdout);
        fprintf(stderr, "Error writing to stdout: %s\n", strerror(err));
        exit(1);
    }
}

static uint64_t read_leb128(const char *path, FILE *file) {
    uint64_t n = 0;
    for (int shift = 0; shift < 64; shift += 7) {
        uint64_t c = read_byte(path, file);
        if (c == EOF) {
            fprintf(stderr, "Premature end of file '%s'", path);
            exit(1);
        }
        int more = (c & 0x80) != 0;
        c &= 0x7f;
        if (c << shift >> shift != c) break;
        n |= c << shift;
        if (!more) return n;
    }
    fprintf(stderr, "Too large LEB128 value in file '%s'", path);
    exit(1);
}

static void write_leb128(uint64_t n) {
    while (1) {
        uint8_t b = 0x7f & n;
        n >>= 7;
        uint8_t more = n != 0;
        b |= more << 7;
        write_byte(b);
        if (!more) return;
    }
}

static void zrl_encode(const char *path, FILE *file) {
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

static void zrl_decode(const char *path, FILE *file) {
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

static void leb128_encode(const char *path, FILE *file) {
    while (fpeek(file) != EOF) {
        uint64_t n = 0;
        for (int shift = 0; shift < 64; shift += 8) {
            uint64_t c = read_byte(path, file);
            if (c == EOF) break;
            n |= c << shift;
        }
        write_leb128(n);
    }
}

static void leb128_decode(const char *path, FILE *file) {
    while (fpeek(file) != EOF) {
        uint64_t n = read_leb128(path, file);
        for (int i = 0; i < 8; i++) {
            write_byte(n & 0xff);
            n >>= 8;
        }
    }
}

#define zrle 1
#define zrld 2
#define leb128e 3
#define leb128d 4

static void zrlf(const char *path, FILE *file) {
#if ZRLF == zrle
    zrl_encode(path, file);
#elif ZRLF == zrld
    zrl_decode(path, file);
#elif ZRLF == leb128e
    leb128_encode(path, file);
#elif ZRLF == leb128d
    leb128_decode(path, file);
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
