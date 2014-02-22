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


void test_buddy(){
	int handle = meminit(4096, 0x1, 5, 0);
	int* h1 = memalloc(128, handle);
	int* h2 = memalloc(256, handle);
	int* h3 = memalloc(128, handle);
	int* h4 = 0x0;
	printf("\nHANDLE 1: %p\n", h1);
	memfree(h1);
	printf("\nHANDLE 2: %p\n", h2);
	memfree(h2);
	printf("\nHANDLE 3: %p\n", h3);
	memfree(h3);
	//memfree(h4);
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
}


void speed_test_buddy(){
	int h = meminit(65536, 0x1, 4, 0);
	clock_t start = clock();
	int k;
//	for(k=0; k<10; k++){
		int i;
		big *b;
		for(i=0; i<128; i++){
			b = memalloc(128, h);	
			b->big_arr[0] = 1;
		}
//	}
	clock_t end = clock();
	printf("buddy speed test: %g seconds\n start: %g\n end: %g\n", (double)(end-start)/CLOCKS_PER_SEC, (double)start, (double)end);
}

void speed_test_list(unsigned flags, unsigned page_size){
	int h = meminit(65536, flags, page_size, 0);
	clock_t start = clock();
	int k;
	int i;
	big *b;
	for(i=0; i<1024; i++)
		b = memalloc(128, h);	
	clock_t end = clock();
	printf("buddy speed test: %g seconds\n start: %g\n end: %g\n", (double)(end-start)/CLOCKS_PER_SEC, (double)start, (double)end);
}


void speed_test_all(unsigned page_size){
	printf("BUDDY page size: %u\n", page_size);
	speed_test_buddy(0x1, page_size);
	printf("FIRST FIT page size: %u\n", page_size);
	speed_test_list(0x4 | 0x0, page_size); //first fit
	printf("NEXT FIT page size: %u\n", page_size);
	speed_test_list(0x4 | 0x8, page_size); //next fit
	printf("BEST FIT page size: %u\n", page_size);
	speed_test_list(0x4 | 0x10, page_size); //best fit
	printf("WORST FIT page size: %u\n", page_size);
	speed_test_list(0x4 | 0x20, page_size); //worst fit
	printf("RANDOM FIT page size: %u\n", page_size);
	speed_test_list(0x4 | 0x40, page_size); //random fit
}

void buddy_holes(int page_size){
	int h = meminit(65536, 0x1, page_size, 0);
	big *b;
	int i;

	for(i=0; i<64; i++)
		b = memalloc(1024, h);
//	free(b);	
	count_holes(h);
}

void p1_test1(){
	// Best fit
	int handle = meminit(8388608,0x14,12,0);
	unsigned int i;
	for(i = 0; i < 256; i++){
		void *b1 = memalloc(4096, handle);
	}
	count_holes(handle);
	// Worst fit
	handle = meminit(8388608,0x24,12,0);
	for(i = 0; i < 256; i++){
		void *b1 = memalloc(4096, handle);
	}
	count_holes(handle);
	// Random fit
	handle = meminit(8388608,0x44,12,0);
	for(i = 0; i < 256; i++){
		void *b1 = memalloc(4096, handle);
	}
	count_holes(handle);
}

void p1_test2(){
	// Best fit
	int handle = meminit(8388608,0x14,12,0);
	int num_pages = 1;
	uint page_size = 4096;
	unsigned int i;
	int mem_locs[50];
	// Alloc
	for(i = 0; i < 50; i++){
		mem_loc[i] = memalloc(num_pages*page_size, handle);
		num_pages++;
	}
	// Free
	for(i = 0; i < 50; i += 2){
		memfree(mem_loc[i]);
	}
	// Realloc
	num_pages = 1;
	unsigned int j = 0;
	for(i = 0; i < 25; i++){
		mem_loc[j] = memalloc(num_pages*page_size, handle);
		num_pages++;
		j += 2;
	}
}

int main(){
	//p1_test1();
	p1_test2();
//	test_buddy();
	//buddy_holes(5);
	//aux_test_buddy();
	//speed_test_buddy();
	/*speed_test_list(0x4 | 0x0, 4); //first fit
	speed_test_list(0x4 | 0x8, 4); //next fit
	speed_test_list(0x4 | 0x10, 4); //best fit
	speed_test_list(0x4 | 0x20, 4); //worst fit
	speed_test_list(0x4 | 0x40, 4); //random fit*/
	/*
	speed_test_all(4);
	speed_test_all(8);
	speed_test_all(16);
	speed_test_all(32);
	speed_test_all(64);*/

//	test_list();
	
}
