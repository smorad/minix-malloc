#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define true 1
#define false 0
#define FREE 0
#define TAKEN 1
#define BUDDY 0
#define FIRST 1
#define NEXT 2
#define BEST 3
#define LIST 4


typedef struct {
	unsigned count;
	void* beg;
	void* end;
	unsigned page_size;
	unsigned char *bitmap;
	unsigned bitmap_size;
} mem_ptr;

mem_ptr mp;

/*
 * Marks memory with the given value
 * index  - location to start in memory
 * size   - amount to mark
 * value  - value to mark memory with ( 0 = free | 1 = taken )
 */
 void mark_mem(int index, int size, int value){
 	unsigned j;
	for(j = index; j < (index + size); j++){
		mp.bitmap[j] = value;
	}
 }

/*
 * Check the bitmap for a free area of the target size
 * size - # of pages
 * Returns index to the start of the free area
 * For Buddy
 * BROKEN RIGHT NOW
 */
int buddy_area_free(int size){
	int i;
	int taken = 0;
	int free = 0;
	for(i = 0; i < mp.bitmap_size;){
		printf("free: %d	taken: %d\n", free, taken);
		// Go until a taken space is found
		// Record size
		while(!mp.bitmap[i++]){
			if( i >= (mp.bitmap_size - 1) ) break;
			free++;
		}
		// Free block of exact size found
		if(free == size){
			
		}
	}
	// There is not enough free space
	return -1;
}

/*
 * Check the bitmap for a free area of the target size
 * size - # of pages
 * Returns the beginning indiex in the bitmap
 * For freelist
 */
int area_free(int size){
	int i;
	int count = 0;
	for(i = 0; i < mp.bitmap_size; i++){
		// Check to see if there is enough free space
		if(count >= size) return (i - size + 1);	
		if(mp.bitmap[i]) count = 0;
		else count++;
	}
	// There is not enough free space
	return -1;
}


int power2(long x){
	return (x & (x-1))==0;
}

long pow2(int parm1){
	int i;
	int rv = 1;
	for(i = 0; i < parm1; i++) rv *= 2;
	return rv;
}
/*
 * Allocates a size of memory n_bytes long.
 * Splits it into page sizes (2^parm1)
 */
int buddy_init(long n_bytes, int parm1){	
	if (!power2(n_bytes)){
		printf("\n%lu: not a pow2\n", n_bytes);
		return -1;
	}
	mp.beg = malloc(n_bytes);
	if (mp.beg == NULL){
		printf("beg = NULL\n");
		return -1;
	}
	mp.page_size = pow2(parm1);
	mp.bitmap_size = n_bytes/mp.page_size;
	mp.bitmap = calloc(1, mp.bitmap_size);
	mp.count++;
	printf("parm1: %d\nbeg: %p\npage_size: %lu\nbitmap_size: %u\n", parm1, mp.beg, mp.page_size, mp.bitmap_size);
	return BUDDY;
}

void* buddy_memalloc(long n_bytes, int handle){
	// Set curr_size = to a power of 2 that is >= n_bytes
	long curr_size = 1;
	while(curr_size < n_bytes) curr_size *= 2;
	printf("Curr_size first: %lu\n", curr_size);
	// Divide by the minimum page size
	// Will use this to pass to the areaa_free detector
	// This is in blocks
	curr_size /= mp.page_size;
	printf("Curr_size: %lu\n", curr_size);
	// Find free area
    	int bitmap_loc = buddy_area_free(curr_size);
      	if ( bitmap_loc == -1 ){
      		printf("Error: No space Found\n");
      		return buddy_memalloc(handle, (pow2(curr_size +1)) );   // Check again for larger space
      	}
      	else{
      		// Marks all as taken
      		mark_mem(bitmap_loc, curr_size, TAKEN);
      	}
}

/*
 * First fit memalloc
 */
void* first_memalloc(long n_bytes, int handle){
	// Find free area
	long curr_size;
	curr_size = (n_bytes/mp.page_size);
    	int bitmap_loc = area_free(curr_size);
      	if ( bitmap_loc == -1 ){
      		printf("Error: No space Found\n");
      		return NULL;
      	}
      	else{
      		// Marks all as taken
      		mark_mem(bitmap_loc, curr_size, TAKEN);
      	}
}


/*
 * Initializes a list based memalloc
 */
 int list_init(long n_bytes, int parm1, int* parm2){
	mp.beg = malloc(n_bytes);
	if (mp.beg == NULL){
		printf("beg = NULL\n");
		return -1;
	}
	mp.page_size = pow2(parm1);
	mp.bitmap_size = n_bytes/mp.page_size;
	mp.bitmap = calloc(1, mp.bitmap_size);
	mp.count++;
	printf("LIST\n\nparm1: %d\nbeg: %p\npage_size: %lu\nbitmap_size: %u\n", parm1, mp.beg, mp.page_size, mp.bitmap_size);
	return LIST;
}

int meminit(long n_bytes, unsigned int flags, int parm1, int* parm2){
	int rv;
	mp.count = 0;	
	if((flags & 0x1)==0x1){
		//printf("0x1\n");
		rv = buddy_init(n_bytes, parm1);
		
	}
	else if((flags & 0x08)==0x08){
		printf("FIRST FIT\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != -1) rv = FIRST;
	}
	else if ((flags & 0x10)==0x10){printf("0x10\n");}
	else if ((flags & 0x20)==0x20){printf("0x20\n");}
	else if ((flags & 0x40)==0x40){printf("0x40\n");}
	else{
		printf("Invalid bits set: %#010x\n", flags);
	}
	return rv;
}

void* memalloc(long n_bytes, int handle){
	switch(handle){
		case BUDDY:
			return buddy_memalloc(n_bytes, handle);
		case FIRST:
			return first_memalloc(n_bytes, handle);
	}
	return NULL;
}
