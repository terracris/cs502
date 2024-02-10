LIB=-lpthread
CC=gcc

all: mutexthr-c pcthreads-c

mutexthr-c: mutexthr.c
	 $(CC) mutexthr.c -o mutexthr-c $(LIB)

pcthreads-c: pcthreads.c
	 $(CC) pcthreads.c -o pcthreads-c $(LIB)

clean: 
	/bin/rm -f *.o core mutexthr-c pcthreads-c

