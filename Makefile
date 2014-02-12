CC=gcc
all: libmem.a test

libmem.a : alloc.o
	ar rcs $@ $^

alloc.o: alloc.c
	gcc alloc.c -o alloc.o

test: test.o alloc.h libmem.a
	gcc -Llibmem.a -o test test.c alloc.c alloc.h

	
