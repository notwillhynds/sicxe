CC = gcc
CFLAGS = -Wall

assemble: assemble.c pass1.c pass2.c tables.c symtab.c
	$(CC) $(CFLAGS) -o assemble assemble.c pass1.c pass2.c tables.c symtab.c

clean:
	rm -f assemble intermediate.txt listing.txt objectcode.txt symtab.txt