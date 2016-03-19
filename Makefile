#!/usr/bin/make

CC=gcc
CFLAGS=-Wall -std=gnu99 -DTEST_SCREEN
LIBS=-lncurses

targets  = simulator
simulator_c = screen.c

all: $(targets)

simulator: $(simulator_c) screen.h
	$(CC) $(CFLAGS) $(LIBS) $(simulator_c) -o $@

clean:
	rm -f $(targets)
