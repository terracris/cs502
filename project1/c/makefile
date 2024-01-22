all: doit

doit: process.o shell.o doit.o
	gcc -g process.o shell.o doit.o -o doit

process.o: process.c process.h
	gcc -g -c process.c

shell.o: shell.c shell.h
	gcc -g -c shell.c

doit.o: doit.c
	gcc -g -c doit.c

clean:
	rm -f *.o shell process doit
