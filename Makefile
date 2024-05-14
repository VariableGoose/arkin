CC := gcc
CFLAGS := -std=c99 -ggdb -Wall
IFLAGS := -Iinclude
LFLAGS := -lm
DFLAGS :=

SRC := $(wildcard src/*.c) $(wildcard tests/*.c)

test:
	$(CC) $(CFLAGS) $(SRC) -o tests/test $(IFLAGS) $(LFLAGS)
