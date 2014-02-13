#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define true 1
#define false 0
typedef struct {
	unsigned count;
	long beg;
	long end;
	unsigned page_size;
	unsigned byte *bitmap;
	unsigned bitmap_size;

} mem_ptr;

mem_ptr mp;

/*
 * Check the bitmap for a free area of the target size
 * size - # of pages
 * Returns theending indiex in the bitmap
 * For freelist
 */
int area_free(int size){
	int i;
	int count = 0;
	for(i = 0; i < mp.bitmap_size; i++){
		// Check to see if there is enough free space
		if(count >= size) return i;	
		if(mp.bitmap[i]) count = 0;
		else count++;
	}
	// There is not enough free space
	return -1;
}


int power2(long x){
	return (x & (x-1))==0;
}

int buddy_init(long n_bytes, int parm1){	
	if (power2(n_bytes)!=0)
		return -1;
	mp.beg = malloc(n_bytes);
	if (mp.beg == NULL)
		return -1;
	mp.page_size = pow(2, parm1);
	mp.bitmap_size = n_bytes/mp.page_size;
	mp.bitmap = calloc(1, mp.bitmap_size);
	mp.count++;
}

void* buddy_memalloc(long n_bytes, int handle){
	int i = 0;
	// Set curr_size = to a power of 2 that is >= n_bytes
	long curr_size = 1;
	int k;
	while(curr_size < n_bytes) curr_size *= 2;
	
	// Divide by the minimum page size
	// Will use this to pass to the areaa_free detector
	curr_size /= mp.page_size;
	// Find free area
    	int bitmap_loc = area_free(curr_size);
      	if ( bitmap_loc == -1){
      		printf("Error: No space Found\n");
      		return NULL;
      	}
      	unsigned j;
	for(j = bitmap_loc; j > (bitmap_loc - cur_size); j--){
		mp.bitmap[j] = 1;
	}
      	else{
		buddy_memalloc(handle, 2^(log2(bytes)+1))   // Check again for larger space
      	}
}

int meminit(long n_bytes, unsigned int flags, int parm1, int* parm2){
	int rv;
	mp.count = 0;	
	if((flags & 0x1)==0x1){
		printf("0x1");
		rv = buddy_init(n_bytes, parm1);
		
	}
		else if((flags & 0x08)==0x08){printf("0x8\n");}
		else if ((flags & 0x10)==0x10){printf("0x10\n");}
		else if ((flags & 0x20)==0x20){printf("0x20\n");}
		else if ((flags & 0x40)==0x40){printf("0x40\n");}
		else{
			printf("Invalid bits set: %#010x\n", flags);
		}
	return rv;
}
