CFLAGS ?= -std=c99 -O3
# assume UNIX by default:
CPP_DEFINES ?= -DIO_UNLOCKED
# set to -DIO_NOLOCK to avoid locking on Windows

OUTPUTS = zrle zrld leb128e leb128d

all: $(OUTPUTS)

$(OUTPUTS): zrl.c
	cc $(CFLAGS) -DZRLF=$@ $< -o $@ $(CPP_DEFINES)

test: $(OUTPUTS)
	./test.sh

clean:
	rm -f $(OUTPUTS)

.PHONY: all test clean
