CC=gcc
all: libmem.a test

libmem.a : alloc.o
	ar rcs $@ $^

alloc.o: alloc.c
	gcc -lm -g -O -c alloc.c

test: test.o alloc.h libmem.a
	gcc -lm -Llibmem.a -g -o test test.c alloc.c alloc.h
	
tar: alloc.c alloc.h Makefile test.c README.md DESIGN paper.pdf
        tar -cvf proj3.tar $^

clean:
	rm *.o *.a	
