#!/usr/bin/make

PRGM   = simulator
HDRS   = screen.h ride.h attendee.h safeScreen.h
SRCS   = main.c screen.c ride.c attendee.c safeScreen.c
LIBS   = -lncurses -lpthread

#note to future self: do not modify below this line :)

ODIR   = bin
CC     = gcc
OBJS   = $(SRCS:%.c=$(ODIR)/%.o)
CFLAGS = -std=c99 -g -Wall -DNDEBUG

$(PRGM): $(OBJS) $(ODIR)
	$(CC) $(CFLAGS) $(LIBS) $(OBJS) -o $(PRGM)

$(ODIR)/%.o: %.c $(HDRS) $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR):
	mkdir $(ODIR)

all: clean $(PRGM)

clean:
	rm -rf $(PRGM) $(OBJS) $(ODIR)

release: $(PRGM)
	rm -rf $(OBJS) $(ODIR)