CFLAGS ?= -std=c99 -O3

all: zrle zrld

zrle zrld: zrl.c
	cc $(CFLAGS) -DZRLF=$@ $< -o $@ $(CPP_DEFINES)

test: zrle zrld
	./test.sh

clean:
	rm -f zrle zrld

.PHONY: all test clean
