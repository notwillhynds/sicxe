CC = gcc
CFLAGS = -Wall
SRCS = pass1.c pass2.c tables.c symtab.c
HEADERS = tabelss.h symtab.h
OBJECTS = $(SRCS:.c=.o)

EXEC = pass1 pass2

all: $(EXEC)

pass1: pass1.o tables.o symtab.o
	$(CC) $(CFLAGS) -o $@ $^

pass2: pass2.o tabels.o symtab.o
	$(CC) $(CFLAGS) -o $@ $^

.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) - c $<
clean: 
	rm -f $(OBJECTS) $(EXEC)