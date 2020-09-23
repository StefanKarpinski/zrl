CFLAGS ?= -std=c99 -O3
# assume UNIX by default:
CPP_DEFINES ?= -DIO_UNLOCKED
# set to -DIO_NOLOCK to avoid locking on Windows

all: zrle zrld

zrle zrld: zrl.c
	cc $(CFLAGS) -DZRLF=$@ $< -o $@ $(CPP_DEFINES)

test: zrle zrld
	./test.sh

clean:
	rm -f zrle zrld

.PHONY: all test clean
