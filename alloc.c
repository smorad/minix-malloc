#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define true 1
#define false 0
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
 * Check the bitmap for a free area of the target size
 * size - # of pages
 * Returns index to the start of the free area
 * For Buddy
 */
int buddy_area_free(int size){
	int i;
	int taken = 0;
	int free = 0;
	for(i = 0; i < mp.bitmap_size; i++){
		printf("free: %d	taken: %d\n", free, taken);
		if(mp.bitmap[i]){
			taken++;
			free = 0;
		}
		else{
			taken = 0;
			free++;
		}
		// Found size in a row that are taken
		if(taken == size){
			printf("Found a location of size taken\n");
			free = 0;
			int j;
			for(j = i; j < i + size; j++){
				// Success!
				// [X] [X] [0] [0]
				if(free == size){
					// Correct for off by 1
					return (i + size - 1);
				}
				if(mp.bitmap[j]){
					taken = 0;
					free = 0;
					break;
				}
				else{
					free++;
				}
			}
		}
		else if(free == size){
			printf("Found a location of size %d	at: %d\n", free, i);
			taken = 0;
			int j;
			for(j = i; j < i + size; j++){
				// Success!
				// [0] [0] [X] [X]
				if(taken == size){
					printf("Found an equal block taken\n");
					// Correct for off by 1
					return (i - size + 1);
				}
				if(mp.bitmap[j]){
					taken++;
				}
				else{
					taken = 0;
					free = 0;
					break;
				}
			}
			// Check to see if memeory is completely empty
			if(taken == 0){
				printf("(i - size + 1): %d\n", (i - size + 1));
				return (i - size + 1);
			}
		}
	}
	// There is not enough free space
	return -1;
}

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

long pow2(int parm1){
	int i;
	int rv = 1;
	for(i = 0; i < parm1; i++) rv *= 2;
	return rv;
}

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
}

void* buddy_memalloc(long n_bytes, int handle){
	// Set curr_size = to a power of 2 that is >= n_bytes
	long curr_size = 1;
	while(curr_size <= n_bytes) curr_size *= 2;
	
	// Divide by the minimum page size
	// Will use this to pass to the areaa_free detector
	// This is in blocks
	curr_size /= mp.page_size;
	printf("Curr_size: %lu\n", curr_size);
	// Find free area
    	int bitmap_loc = buddy_area_free(curr_size);
      	if ( bitmap_loc == -1 ){
      		printf("Error: No space Found\n");
      		return NULL;
      		// Marks all as taken
	      	unsigned j;
		for(j = bitmap_loc; j > (bitmap_loc - curr_size); j--){
			mp.bitmap[j] = 1;
		}
      	}
      	else{
		buddy_memalloc(handle, (2^(curr_size +1)) );   // Check again for larger space
      	}
}

int meminit(long n_bytes, unsigned int flags, int parm1, int* parm2){
	int rv;
	mp.count = 0;	
	if((flags & 0x1)==0x1){
		//printf("0x1\n");
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

void* memalloc(long n_bytes, int handle){
	return buddy_memalloc(n_bytes, handle);
}
