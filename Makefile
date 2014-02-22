CC=gcc
all: libmem.a test

libmem.a : alloc.o
	ar rcs $@ $^

alloc.o: alloc.c
	gcc -lm -g -O -c alloc.c

test: test.o alloc.h libmem.a
	gcc -lm -Llibmem.a -g -o test test.c alloc.c alloc.h

clean:
	rm *.o *.a	
