#include "alloc.h"
#include <stdio.h>

int main(){
	int handle;
	// First fit  = 0x08
	handle = meminit(4096, 0x08, 4, 0);
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
	printf("mem_area1\n");
	memfree(mem_area1);
	printf("mem_area2\n");
	memfree(mem_area2);
	printf("mem_area3\n");
	memfree(mem_area3);
	printf("mem_area4\n");
	memfree(mem_area4);
	printf("mem_area5\n");
	memfree(mem_area5);
}
