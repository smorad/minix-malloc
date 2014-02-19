#include "alloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct{
	int f1;
	int f2;
	long f3;
	long f4;

}test;

typedef struct{
	long long big_arr[65535];
}big;

void test_list(){
	int handle;
	// First fit  = 0x08
	handle = meminit(4096, 0x10, 4, 0);
	if(handle == -1){
		printf("handle == -1\n");
	}
	void* mem_area1 = memalloc(64,handle);
	void* mem_area2 = memalloc(16,handle);
	void* mem_area3 = memalloc(16,handle);
	void* mem_area4 = memalloc(64,handle);
	void* mem_area5 = memalloc(2048,handle);
	// Test overflow
	memalloc(2048,handle);
	memfree(mem_area1);
	memfree(mem_area2);
	memfree(mem_area3);
	memfree(mem_area4);
	memfree(mem_area5);
	mem_area1 = memalloc(64,handle);
}


void test_buddy(){
	int handle = meminit(4096, 0x1, 4, 0);
	printf("ptr: %p\n", memalloc(128, handle));
	printf("ptr: %p\n", memalloc(256, handle));
	printf("ptr: %p\n", memalloc(128, handle));
}

void aux_test_buddy(){
	void* h1 = memalloc(128, 0);
	void* h2 = memalloc(256, 0);
	void* h3 = memalloc(128, 0);
	test *t = memalloc(sizeof(test), 0);
	printf("assigning %p->f1 to 1\n. %p->f1==%d", t,t, t->f1);
	t->f1 = 1;
	t->f2 = 2;
	t->f3 = 3;
	t->f4 = 4;
	int i;
/*	for(i=0; i<128/(sizeof (int)); i++){
		((int*)h1)[i] = i;
		printf("%d==%d\n", i, ((int*)h1)[i]);
	}*/

		
}


void speed_test_buddy(){
	int h = meminit(65536, 0x1, 4, 0);
	clock_t start = clock();
	int k;
	for(k=0; k<1000; k++){
		int i;
		big *b;
		for(i=0; i<1024; i++)
			b = memalloc(128, h);	
	}
	clock_t end = clock();
	printf("buddy speed test: %g seconds\n start: %g\n end: %g\n", (double)(end-start)/CLOCKS_PER_SEC, (double)start, (double)end);
}

void speed_test_list(){
	int h = meminit(65536, 0x10, 4, 0);
	clock_t start = clock();
	int k;
	for(k=0; k<1000; k++){
		int i;
		big *b;
		for(i=0; i<1024; i++)
			b = memalloc(128, h);	
	}
	clock_t end = clock();
	printf("buddy speed test: %g seconds\n start: %g\n end: %g\n", (double)(end-start)/CLOCKS_PER_SEC, (double)start, (double)end);
}

int main(){
	char* test = malloc(1024);
	//test_buddy();
	//aux_test_buddy();
	speed_test_buddy();
	speed_test_list();
	//test_list();
	
}
