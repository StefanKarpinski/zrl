CFLAGS ?= -O3

all: zrle zrld

zrle zrld: zrl.c
	cc $(CFLAGS) -DZRLF=$@ $< -o $@

test: zrle zrld
	./test.sh

clean:
	rm -f zrle zrld

.PHONY: all test clean
