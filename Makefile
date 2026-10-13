CC	   	= gcc
CFLAGS	= -Wall -Wextra
SRC 	= my_malloc.c main.c
HDR 	= my_malloc.h

all: test

test: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o2 $(SRC) -o test

debug: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -g -o0 $(SRC) -o test_debug

clean: $(SRC) $(HDR)
	rm -f test test_debug

.PHONY: all debug clean