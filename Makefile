CC=gcc
all: libmem.a test

libmem.a : alloc.o
	ar rcs $@ $^

alloc.o: alloc.c
	gcc -g -O -c alloc.c

test: test.o alloc.h libmem.a
	gcc -Llibmem.a -g -o test test.c alloc.c alloc.h

clean:
	rm *.o *.a	
