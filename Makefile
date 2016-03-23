#!/usr/bin/make

PRGM   = simulator
SRCS   = main.c screen.c ride.c attendee.c safeScreen.c log.c
LIBS   = ncurses pthread
CFLAGS = -std=c99 -g -Wall

#note to future self: do not modify below this line :)

CC     = gcc
ODIR   = bin
OBJS   = $(SRCS:%.c=$(ODIR)/%.o)
LFLAGS = $(LIBS:%=-l%)

$(PRGM): $(OBJS) $(ODIR)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) -o $(PRGM)

$(ODIR)/%.o: %.c $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR):
	mkdir $(ODIR)

all: clean $(PRGM)

clean:
	rm -rf $(PRGM) $(OBJS) $(ODIR)

release: $(PRGM)
	rm -rf $(OBJS) $(ODIR)
