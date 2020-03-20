all: zrle zrld

zrle zrld: zrl.c
	cc -DZRLF=$@ $< -o $@

clean:
	rm -f zrle zrld

.PHONY: all clean
