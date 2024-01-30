CC=gcc
CFLAGS=-c -g -Wall -std=c17
 
SOURCES=nivel1.c nivel2.c nivel3.c nivel4.c nivel5.c nivel6.c my_shell.c
LIBRARIES= # Eliminado .o
INCLUDES= # Eliminado .h
PROGRAMS=nivel1 nivel2 nivel3 nivel4 nivel5 nivel6 my_shell
OBJS=$(SOURCES:.c=.o)
 
all: $(PROGRAMS)
 
$(PROGRAMS): $(OBJS)
	$(CC) $@.o -o $@ $(LDFLAGS)
 
nivel1.o: nivel1.c
	$(CC) $(CFLAGS) $< -o $@

nivel2.o: nivel2.c
	$(CC) $(CFLAGS) $< -o $@

nivel3.o: nivel3.c
	$(CC) $(CFLAGS) $< -o $@

nivel4.o: nivel4.c
	$(CC) $(CFLAGS) $< -o $@

nivel5.o: nivel5.c
	$(CC) $(CFLAGS) $< -o $@

nivel6.o: nivel6.c
	$(CC) $(CFLAGS) $< -o $@

my_shell.o: my_shell.c
	$(CC) $(CFLAGS) $< -o $@
 
.PHONY: clean
clean:
	rm -rf *.o *~ *.tmp $(PROGRAMS)
