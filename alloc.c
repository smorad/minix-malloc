#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#define true 1
#define false 0
#define FREE 0
#define TAKEN 1
#define BUDDY 0
#define FIRST 1
#define NEXT 2
#define BEST 3
#define LIST 4
#define ERROR -1


typedef struct {
	unsigned count;
	void* beg;
	void* end;
	unsigned page_size;
	unsigned char *bitmap;
	unsigned bitmap_size;
} mem_ptr;

struct binary_tree{
	int taken;
	long size;
	long seg_beg;
	long seg_end;
	void* seg_start;
	struct binary_tree* lchild;
	struct binary_tree* rchild;
};

typedef struct binary_tree *btree;

btree trees[1024];

int btree_count = 0;

int power2(long x){
	return (x & (x-1))==0;
}

void btree_debug(btree parent){
	printf("parent size: %lu beg: %lu end: %lu\n", parent->size, parent->seg_beg, parent->seg_end);
	printf("lchild_ptr: %p rchild_ptr: %p\n", 
		parent->lchild->seg_start, parent->rchild->seg_start);

}

btree insert_node(long begin, long end){
	btree new = malloc(sizeof(struct binary_tree));
	assert(new!=NULL);
	new->size = end - begin;
	//if(begin==0)
	//	new->size++;
	assert(new->size!=0);
	new->lchild = NULL;
	new->rchild = NULL;
	new->taken = false;
	new->seg_start = NULL;
	new->seg_beg = begin;
	new->seg_end = end;
	assert(new!=NULL);
	printf("inserting node of size: %lu, beg: %lu end %lu\n", new->size, new->seg_beg, new->seg_end);
	return new;
}

int _buddy_init(long n_bytes, int parm1){	
	if (!power2(n_bytes)){
		printf("\n%lu: not a pow2\n", n_bytes);
		return ERROR;
	}
	assert(parm1>0);
	trees[btree_count] = insert_node(0, n_bytes); //change me for pages
	trees[btree_count]->seg_start = (void*)malloc(n_bytes);
	if (trees[btree_count] == NULL){
		printf("beg = NULL\n");
		return ERROR;
	}
	return btree_count++;
}

void* _buddy_alloc(long n_bytes, btree root){
	btree parent = root;
	btree lchild = NULL;
	btree rchild = NULL;
	//traverse in order
	while(parent!=NULL){
	lchild = parent->lchild;
	rchild = parent->rchild;
		if(parent->lchild != NULL)
			return _buddy_alloc(n_bytes, parent->lchild);
		else if(parent->rchild != NULL)
			return _buddy_alloc(n_bytes, parent->rchild);
		else{
			//if leaf
			//if block is empty and correct size
			if(parent->taken==0 && (parent->size == n_bytes) || (parent->size== n_bytes-1)){
				printf("***FOUND***\n\n\n");
				parent->taken = true;
				return parent->seg_start;
			}
			else{
				printf("creating child\n");
				//split block into children
				//these conflict, fix later
				parent->lchild = insert_node(parent->seg_beg, (parent->seg_end/2));
				parent->rchild = insert_node(parent->seg_end/2+1, parent->seg_end+1);
				//check again starting at current node
				btree_debug(parent);
				return _buddy_alloc(n_bytes, parent);
			}
		
		}
			
	}
}

mem_ptr mp;

/*
 * Global variable for keeping track of index for next fit
 */
 unsigned curr_index = 0;

/*
 * Prints the bitmap for bedugging purposes
 */
 void print_bitmap(){
	printf("\n--------------------BITMAP----------------------\n");
	unsigned j;
	unsigned col_count = 0;
	for(j = 0; j < mp.bitmap_size; j++){
		if(col_count == 16){
			col_count = 0;
			printf("\n");
		}
		col_count++;
		printf("[%d]", mp.bitmap[j]);
	}
	printf("\n--------------------ENDBMP----------------------\n");
 }

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
	printf("Marked %d spots at %d index to %d value\n", size, index, value);
	print_bitmap();
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
	return ERROR;
}

/*
 * Check the bitmap for the first free area of the target size
 * size - # of pages
 * Returns the beginning indiex in the bitmap
 * For freelist
 */
int first_area_free(int size){
	int i;
	int count = 0;
	for(i = 0; i < mp.bitmap_size; i++){
		// Check to see if there is enough free space
		if(count >= size){
			printf("Found a location of size %d	at: %d\n", size, (i - count));
			return (i - count);
		}
		if(mp.bitmap[i] == TAKEN) count = 0;
		else count++;
	}
	// There is not enough free space
	printf("Not enough room for block of size %d\n", size);
	return ERROR;
}

/*
 * Check the bitmap for the next free area of the target size
 * size - # of pages
 * Returns the beginning indiex in the bitmap
 * For freelist
 */
int next_area_free(int size){
	int count = 0;
	unsigned full_loop;
	for(full_loop = 0; full_loop < mp.bitmap_size; full_loop++){
		// Loop back around
		if( curr_index >= mp.bitmap_size){
			curr_index = 0;
			count = 0;
		}
		// Check to see if there is enough free space
		if(count >= size){
			printf("Found a location of size %d	at: %d\n", size, (curr_index - count));
			return (curr_index - count);
		}
		if(mp.bitmap[curr_index] == TAKEN) count = 0;
		else count++;
		
		// Next value in bitmap
		curr_index++;
	}
	// There is not enough free space
	printf("Not enough room for block of size %d\n", size);
	return ERROR;
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
/*int buddy_init(long n_bytes, int parm1){	
	if (!power2(n_bytes)){
		printf("\n%lu: not a pow2\n", n_bytes);
		return ERROR;
	}
	mp.beg = malloc(n_bytes);
	if (mp.beg == NULL){
		printf("beg = NULL\n");
		return ERROR;
	}
	mp.page_size = pow2(parm1);
	mp.bitmap_size = n_bytes/mp.page_size;
	mp.bitmap = calloc(1, mp.bitmap_size);
	mp.count++;
	printf("parm1: %d\nbeg: %p\npage_size: %lu\nbitmap_size: %u\n", parm1, mp.beg, mp.page_size, mp.bitmap_size);
	return BUDDY;
}*/

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
      	if ( bitmap_loc == ERROR ){
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
void* list_memalloc(long n_bytes, int handle){
	// Return pointer in memory
	void *mem_ptr;
	// Find free area
	long curr_size;
	curr_size = (n_bytes/mp.page_size);
	int bitmap_loc;
	/* Switch for different list cases */
	switch(handle){
		case FIRST: 
    			bitmap_loc = first_area_free(curr_size);
    			break;
    		case NEXT: 
    			bitmap_loc = next_area_free(curr_size);
    			break;
	}
      	if ( bitmap_loc == ERROR ){
      		printf("Error: No space Found\n");
      		return NULL;
      	}
      	else{
      		// Marks all as taken
      		mark_mem(bitmap_loc, curr_size, TAKEN);
      		mem_ptr = (mp.beg + (bitmap_loc * mp.page_size));
      		printf("mem_ptr: %p\n", mem_ptr);
      		return mem_ptr;
      	}
}


/*
 * Initializes a list based memalloc
 */
 int list_init(long n_bytes, int parm1, int* parm2){
	mp.beg = malloc(n_bytes);
	if (mp.beg == NULL){
		printf("beg = NULL\n");
		return ERROR;
	}
	mp.page_size = pow2(parm1);
	mp.bitmap_size = n_bytes/mp.page_size;
	mp.bitmap = calloc(1, mp.bitmap_size);
	mp.count++;
	printf("parm1: %d\nbeg: %p\npage_size: %u\nbitmap_size: %u\n", parm1, mp.beg, mp.page_size, mp.bitmap_size);
	return LIST;
}

int meminit(long n_bytes, unsigned int flags, int parm1, int* parm2){
	int rv;
	mp.count = 0;	
	if((flags & 0x1)==0x1){
		//printf("0x1\n");
		rv = _buddy_init(n_bytes, parm1);
		
	}
	else if((flags & 0x08)==0x08){
		printf("FIRST FIT\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) rv = FIRST;
	}
	else if ((flags & 0x10)==0x10){
		printf("NEXT FIT\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) rv = NEXT;
	}
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
			return _buddy_alloc(n_bytes, trees[handle]);
		case ERROR:
			printf("Memory not initialized correctly\n");
			return NULL;
		default:
			return list_memalloc(n_bytes, handle);
	}
	return NULL;
}

void memfree(void *region){
	unsigned mem_index, bitmap_index, free_size;
	mem_index = (unsigned)(region - mp.beg);
	bitmap_index = (mem_index/mp.page_size);
	free_size = 8; // dummy value until i figure out how to actually do it
	printf("Bitmap index freed: %u		of size: %d\n", bitmap_index, free_size );
	mark_mem(bitmap_index, free_size, FREE);
}
