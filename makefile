CC=gcc

all: myShell

myShell: lucasShell.o builtin.o
	$(CC) -o $@ $?

builtin.o: builtin.c builtin.h
	$(CC) -c builtin.c -o builtin.o

lucasShell.o: lucasShell.c builtin.h

.c.o:
	$(CC) -c $< -o $@

clean:
	-rm myShell *.o
