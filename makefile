CC = gcc
CFLAGS = -Wall

all: pass1 pass2

pass1: pass1.c tables.c symtab.c
	$(CC) $(CFLAGS) -o pass1 pass1.c tables.c symtab.c

pass2: pass2.c tables.c symtab.c
	$(CC) $(CFLAGS) -o pass2 pass2.c tables.c symtab.c

clean:
	rm -f pass1 pass2