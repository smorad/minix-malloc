#include "alloc.h"

int main(){
	int handle;
	// First fit  = 0x08
	handle = meminit(4096, 0x08, 4, 0);
	if(handle == -1){
		printf("handle == -1\n");
	}
	memalloc(64,handle);
	memalloc(16,handle);
	memalloc(16,handle);
	memalloc(64,handle);
	void* mem_area = memalloc(2048,handle);
	// Test overflow
	memalloc(2048,handle);
	memfree(mem_area);
}
