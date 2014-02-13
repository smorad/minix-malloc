#include "alloc.h"

int main(){
/*	printf("%d\n",meminit(128, 0x1, 0, 0));	
	printf("%d\n",meminit(128, 0x4, 0, 0));
	printf("%d\n",meminit(128, 0x8, 0, 0));
	printf("%d\n",meminit(128, 0x10, 0, 0));
	printf("%d\n",meminit(128, 0x20, 0, 0));
	printf("%d\n",meminit(128, 0x40, 0, 0));*/

	int handle;
	handle = meminit(4096, 0x1, 4, 0);
	if(handle == -1){
		printf("handle == -1\n");
	}
	memalloc(64,handle);
	memalloc(16,handle);
	memalloc(16,handle);
	memalloc(64,handle);
}
