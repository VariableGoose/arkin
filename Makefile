CC := gcc
CFLAGS := -std=c99 -ggdb -Wall
IFLAGS := -Iinclude
LFLAGS := -lm
DFLAGS :=

SRC := $(wildcard src/*.c)

build:
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o bin/a.out $(IFLAGS) $(LFLAGS)
