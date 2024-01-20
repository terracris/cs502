all: shell

shell: process.o shell.o
	gcc -g process.o shell.o -o shell

process.o: process.c process.h
	gcc -g -c process.c

shell.o: shell.c shell.h
	gcc -g -c shell.c

clean:
	rm -f *.o
