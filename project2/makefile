all: traceanal

traceanal: traceanal.o hashtable.o
	gcc traceanal.o hashtable.o -o traceanal

traceanal.o: traceanal.c
	gcc -c traceanal.c

hashtable.o: hashtable.c hashtable.h
	gcc -c hashtable.c

clean:
	rm *.o traceanal