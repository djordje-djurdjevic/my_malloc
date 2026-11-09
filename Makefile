CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude -pthread
SRC     = src/my_malloc.c src/test_funcs.c src/main.c
HDR     = include/my_malloc.h include/test_funcs.h

all: test

test: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -O2 $(SRC) -o test

debug: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -g -O0 $(SRC) -o test_debug

clean:
	rm -f test test_debug

.PHONY: all debug clean
