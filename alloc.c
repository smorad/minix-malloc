#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

#define true 1
#define false 0
#define FREE 0
#define TAKEN 1
#define BUDDY 0
#define FIRST 1
#define NEXT 2
#define BEST 3
#define WORST 4
#define RANDOM 5
#define LIST 6
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
	long data_size;
	int taken;
	int should_free;
	long size;
	long seg_beg;
	long seg_end;
	void* seg_start;
	struct binary_tree* lchild;
	struct binary_tree* rchild;
};

typedef struct binary_tree *btree;

btree trees[1024];
void* data[1024];	//our memory contents
int block_size[1024];
int memalloc_mode[1024];
int memalloc_mode_counter = 0;
int meminit_count = 0;


int power2(long x){
	return (x & (x-1))==0;
}
long pow2(int parm1){
	int i;
	int rv = 1;
	for(i = 0; i < parm1; i++) rv *= 2;
	return rv;
}






/*print function stolen from Narue on daniweb.com*/

void padding ( char ch, int n )
{
	  int i;

	    for ( i = 0; i < n; i++ )
			    putchar ( ch );
}

void structure ( btree root, int level )
{
	  int i;

	    if ( root == NULL ) {
			    padding ( '\t', level );
				    puts ( "~" );
					  }
		  else {
			      structure ( root->rchild, level + 1 );
				      padding ( '\t', level );
					      printf ( "%d\n", root->taken );
						      structure ( root->lchild, level + 1 );
							    }
}


/*end print function*/



void print_memtree(btree root, int start){
	structure(root, 0);
	/*start++;
	if(root->lchild!=NULL)
		print_memtree(root->lchild, start);
	int i;
	for(i=0; i<=start; i++)
		printf("   ");
	printf("%d %d\n", root->size, root->taken);
	if(root->lchild !=NULL)
		print_memtree(root->lchild, start);*/
}

btree insert_node(long begin, long end, void* data, int handle){
	btree new = malloc(sizeof(struct binary_tree));
	assert(new!=NULL);
	new->data_size = 0;
	new->seg_beg = begin;
	new->seg_end = end;
	new->size = end - begin;
	if(new->size % block_size[handle] != 0){ //uhoh, need to add a block
		long remainder = new->size/block_size[handle];
		long increment = abs(block_size[handle] - remainder);
		//printf("size: %lu, adding %lu\n", new->size + increment);
		new->size += increment+1;
		new->seg_end += increment+1;
	}

	//if(begin==0)
	//	new->size++;
	assert(new->size!=0);
	new->lchild = NULL;
	new->rchild = NULL;
	new->taken = 0;
	new->should_free = 0;
	new->seg_start = data + handle*sizeof(long) + begin;
//	printf("data: %p + handle: %d * sizeof(long): %lu + begin: %lu == **%p **\n", data, handle, sizeof(long), begin, new->seg_start);
//	printf("insert: beg: %lu end: %lu seg start: %p\n",begin,end, new->seg_start);
	assert(new!=NULL);
	//printf("inserting node of size: %lu, beg: %lu end %lu\n", new->size, new->seg_beg, new->seg_end);
	return new;
}

int _buddy_init(long n_bytes, int parm1){	
	if (!power2(n_bytes)){
		printf("\n%lu: not a pow2\n", n_bytes);
		return ERROR;
	}
	assert(parm1>0);
	block_size[meminit_count] = pow2(parm1); //block_Size_in_bytes 
	trees[meminit_count] = insert_node(0, n_bytes, NULL, meminit_count); //change me for pages
	//printf("ROOT NODE FOR HANDLE %d IS %p\n", meminit_count, trees[meminit_count]);
	//trees[meminit_count]->seg_start = (void*)malloc(n_bytes+10); //10 extra bytes just in case
	data[meminit_count] = malloc(n_bytes+10);
	if (trees[meminit_count] == NULL){
		//printf("beg = NULL\n");
		return ERROR;
	}
	return meminit_count++;
}


btree found_node = NULL;
/*if mode == 1. will only coalesce, not free*/
btree find_by_region(btree root, void* region, int mode){	//will return node with segment, also will coalesce empty blocks
	if(root==NULL) return NULL;
	/*mark node as freeable, coalesce block*/
	if(root->lchild==NULL && root->rchild==NULL && root->taken==0) {//leaf
		//printf("marking node %p as should_free\n", root);
		root->should_free = 1;
	}
	if(root->lchild!=NULL && root->rchild!=NULL)
		if(root->lchild->should_free && root->rchild->should_free){
			//printf("freeing node: %p segment: %p\n", root, root->seg_start);
			free(root->lchild);
			free(root->rchild);
			root->lchild = root->rchild = NULL;
			root->should_free = 1; //do i want to do this?
		}
	/*search block*/

	//printf("root->seg_start: %p region: %p\n", root->seg_start, region);
	if(root->taken && root->seg_start == region && mode!=1){
		//printf("FOUND FREE\n");
		root->taken = 0;
		root->should_free = 1;
		found_node = root;
		return root;
	}
	else{
		if(root->lchild!=NULL){
			//printf("moving left to %p\n", root->lchild);
			find_by_region(root->lchild, region, mode);
		}
	
		if(root->rchild!=NULL){
			//printf("moving right to %p\n", root->lchild);
			find_by_region(root->rchild, region, mode);
		}
		else
			return NULL;
	}
}


int _free_buddy(void* region, int mode){
	int i;
	for(i=0; i<meminit_count; i++){
		found_node = NULL;
		//printf("attempting to find %p in tree %d\n", region, i);
		find_by_region(trees[i], region, mode);
		if(found_node!=NULL){
			found_node->taken = 0;
			//printf("\n\nreturn 0\n\n");
			return 0;
		}
	}
	//printf("\n\nreturn 1\n\n");
	return ERROR;
}

void* result_ptr = NULL; //in case we return null after finding value

void* _buddy_alloc(long n_bytes, btree root, void* data, int handle){
	//traverse in order
	if(root==NULL) return NULL;
	if(result_ptr != NULL) return; //found our result, don't want to overwrite
  	if(root->size < n_bytes){//too deep into the tree
            	//printf("--PARENT TOO SMALL-- ptr: %p n_bytes: %lu size: %lu\n", root, n_bytes, root->size);
		 return result_ptr;
  	}		
	if(root->lchild != NULL)
		_buddy_alloc(n_bytes, root->lchild, data, handle);
	if(root->rchild != NULL)
		_buddy_alloc(n_bytes, root->rchild, data, handle);
	if(root->lchild == NULL && root->rchild == NULL){ 	//if leaf
		if(root->taken==0 && (root->size == n_bytes)){		//if block is empty and correct size
			root->taken = 1;
			//printf("root->taken add: %p\n", &root->taken);
			result_ptr = root->seg_start;
			return;
		}
		else{
			if(root->size/2 < n_bytes) return;
			//split block into children
			root->lchild = insert_node(root->seg_beg, ((root->seg_beg + root->seg_end)/2), data, handle);
			root->rchild = insert_node(((root->seg_beg + root->seg_end)/2)+1, root->seg_end+1, data, handle);

			root->lchild->data_size = n_bytes;
			root->rchild->data_size = n_bytes;
			//check again starting at current node
			_buddy_alloc(n_bytes, root, data, handle);
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
  * Finds the holes in the linear bitmap
  */
 void find_holes(){
 	
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
	//printf("Marked %d spots at %d index to %d value\n", size, index, value);
	//print_bitmap();
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
 * Check the bitmap for the best free area of the target size
 * size - # of pages
 * Returns the beginning indiex in the bitmap
 * For freelist
 */
int best_area_free(int size){
	int i;
	int best_index = -1;
	int best = INT_MAX;
	int count = 0;
	for(i = 0; i < mp.bitmap_size; i++){
		// Check edge case for end of free list
		if( mp.bitmap[i] == TAKEN || (i == (mp.bitmap_size - 1)) ){
			if(count >= size && count <= best){
				best_index = i;
				best = count;
			}
			count = 0;
		}
		else count++;
	}
	if(best_index != ERROR){
		return (best_index - best);
	}
	// There is not enough free space
	printf("Not enough room for block of size %d\n", size);
	return ERROR;
}

/*
 * Check the bitmap for the worst free area of the target size
 * size - # of pages
 * Returns the beginning indiex in the bitmap
 * For freelist
 */
int worst_area_free(int size){
	int i;
	int worst_index = -1;
	int worst = 0;
	int count = 0;
	for(i = 0; i < mp.bitmap_size; i++){
		// Check edge case for end of free list
		if( mp.bitmap[i] == TAKEN || (i == (mp.bitmap_size - 1)) ){
			if(count >= size && count >= worst){
				worst_index = i;
				worst = count;
			}
			count = 0;
		}
		else count++;
	}
	if(worst_index != ERROR){
		return (worst_index - worst);
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

/*
 * Check the bitmap for a random free area of the target size
 * size - # of pages
 * Returns the beginning index in the bitmap
 * For freelist
 */
int random_area_free(int size){
	unsigned int i;
	int random_index = rand() % mp.bitmap_size;
	int count = 0;
	unsigned int full_circle = 0;
	for(i = random_index; full_circle < mp.bitmap_size; i++){
		// Check to see if there is enough free space
		if(count >= size){
			return (i - count);
		}
		if(mp.bitmap[i] == TAKEN) count = 0;
		else count++;
		
		if(i == mp.bitmap_size - 1){
			i = 0;
		}
		full_circle++;
	}
	// There is not enough free space
	printf("Not enough room for block of size %d\n", size);
	return ERROR;
}




/*
 * First fit memalloc
 */
void* list_memalloc(long n_bytes, int handle){
	// Return pointer in memory
	void *mem_ptr;
	void *header;
	// Find free area
	long curr_size;
	curr_size = (n_bytes/mp.page_size);
	/* Fixes values smaller than one page */
	if(curr_size == 0){
		curr_size = 1;
	}
	int bitmap_loc;
	/* Switch for different list cases */
	switch(memalloc_mode[handle]){
		case FIRST: 
    			bitmap_loc = first_area_free(curr_size);
    			break;
    		case NEXT: 
    			bitmap_loc = next_area_free(curr_size);
    			break;
    		case WORST: 
    			bitmap_loc = worst_area_free(curr_size);
    			break;
    		case BEST: 
    			bitmap_loc = best_area_free(curr_size);
    			break;
    		case RANDOM: 
    			bitmap_loc = random_area_free(curr_size);
    			break;
	}
      	if ( bitmap_loc == ERROR ){
      		printf("Error: No space Found\n");
      		return NULL;
      	}
      	else{
      		// Marks all as taken
      		mark_mem(bitmap_loc, curr_size, TAKEN);
      		header = mp.beg + (bitmap_loc * mp.page_size);
      		*((long*)header) = n_bytes;
      		mem_ptr = mp.beg + (bitmap_loc * mp.page_size) + (sizeof(long));
      		//printf("mem_ptr: %p\n", mem_ptr);
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
	return meminit_count++;
}

int meminit(long n_bytes, unsigned int flags, int parm1, int* parm2){
	int rv;
	mp.count = 0;	
	if(flags==0x1){
		printf("\n\n---------------------BUDDY---------------------\n\n");
		rv = _buddy_init(n_bytes, parm1);
		memalloc_mode[memalloc_mode_counter] = BUDDY;
		
	}
	else if(flags==(0x00 | 0x4)){
		printf("\n\n-------------------FIRST FIT--------------------\n\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) memalloc_mode[memalloc_mode_counter] = FIRST;
	}
	else if (flags==(0x08| 0x4)){
		printf("\n\n--------------------NEXT FIT--------------------\n\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) memalloc_mode[memalloc_mode_counter] = NEXT;
	}
	else if (flags==(0x10| 0x4)){
		printf("\n\n--------------------BEST FIT--------------------\n\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) memalloc_mode[memalloc_mode_counter] = BEST;
	}
	else if (flags==(0x20| 0x4)){
		printf("\n\n-------------------WORST FIT--------------------\n\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) memalloc_mode[memalloc_mode_counter] = WORST;
	}
	else if (flags==(0x40| 0x4)){
		printf("\n\n-------------------RANDOM FIT-------------------\n\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) memalloc_mode[memalloc_mode_counter] = RANDOM;
	}
	else{
		printf("Invalid bits set: %#010x\n", flags);
		exit(1);
	}
	memalloc_mode_counter++;
	return rv;
}

void* memalloc(long n_bytes, int handle){
	switch(memalloc_mode[handle]){
		case BUDDY:
			result_ptr = NULL;
			_buddy_alloc(n_bytes, trees[handle], data[handle], handle);
			return result_ptr;
		case ERROR:
			printf("Memory not initialized correctly\n");
			return NULL;
		default:
			//printf("n_bytes: %lu\n", n_bytes);
			return list_memalloc(n_bytes, handle);
	}
	return NULL;
}

void memfree(void *region){
	if(_free_buddy(region, 0)==0)
		return;
	if(mp.page_size==0) return;
	void *find_len;
	unsigned mem_index, bitmap_index;
	long free_size;
	mem_index = (unsigned)(region - mp.beg);
	bitmap_index = (mem_index/mp.page_size);
	find_len = region - (sizeof(long));
	free_size = *((long *)find_len);
	free_size = (free_size/mp.page_size);
	/* Fixes values smaller than one page */
	if(free_size == 0){
		free_size = 1;
	}
	/* Convert free_size to blocks */
	//printf("Bitmap index freed: %u		of size: %li\n", bitmap_index, free_size );
	mark_mem(bitmap_index, free_size, FREE);
}

/*
 * Struct to keep track of holes we find in the memory
 */
typedef struct {
	int num_free, num_taken;
   	unsigned long size_free, size_taken;
} metrics;


void count_holes_buddy(btree root, metrics *m){
	if(root == NULL) return;
	//find_by_region(root, NULL, 1);
	if(root->lchild!=NULL)
		count_holes_buddy(root->lchild, m);
	if(root->rchild!=NULL)
		count_holes_buddy(root->rchild, m);
	if(root->lchild == NULL && root->rchild == NULL){
		//printf("ptr: %p\n", &root->taken);
		if(root->taken == 1){
			m->num_taken++;
			m->size_taken += root->size;
		}
		else{
			m->num_free++;
			m->size_free += root->size;
		}
	}
}
/* 
 * Below are all functions for calculating statistics
 */

void print_min(unsigned int *free_holes, unsigned int *taken_holes, metrics *m){
 	unsigned int i;
 	unsigned int min = INT_MAX;
 	for(i = 0; i < m->num_free; i++){
 		if(free_holes[i] < min) min = free_holes[i];
 	}
 	printf("Minimum Size Block Free:	%lu	%d\n", min, m->num_free);
 	
 	min = INT_MAX;
 	for(i = 0; i < m->num_taken; i++){
 		if(taken_holes[i] < min) min = taken_holes[i];
 	}
 	printf("Minimum Size Block Taken:	%lu\n", min);
 }
 
void print_max(unsigned int *free_holes, unsigned int *taken_holes, metrics *m){
 	unsigned int i;
 	unsigned int max = 0;
 	for(i = 0; i < m->num_free; i++){
 		if(free_holes[i] > max) max = free_holes[i];
 	}
 	printf("Maximum Size Block Free:	%lu	%d\n", max, m->num_free);
 	
 	max = 0;
 	for(i = 0; i < m->num_taken; i++){
 		if(taken_holes[i] > max) max = taken_holes[i];
 	}
 	printf("Maximum Size Block Taken:	%lu\n", max);
 }
 

 
 void print_median(unsigned int *free_holes, unsigned int *taken_holes, metrics *m){
	printf("Median Size Block Free:\t%lu\n", free_holes[m->num_free/2]);
    printf("Median Size Block Taken:\t%lu\n", taken_holes[m->num_free/2]);
}

void print_median(unsigned int *free_holes, unsigned int *taken_holes, metrics *m){
unsigned long long mean = 0;
unsigned long count;
for(count=0; count<m->num_free;count++)
	mean+=free_holes[i];
mean = mean/m->num_free;
printf("Mean Size Block Free:\t%llu\n, mean);
mean = 0;
for(count=0; count<m->num_taken;count++)
	mean+=taken_holes[i];
mean = mean/m->num_taken;
printf("Mean Size Block Free:\t%llu\n, mean);

}

/*
 * Counts the holes in a list format
 */
void count_holes_list(metrics *m){
	printf("\n\n----------Free and Taken Block Sizes------------\n\n");
	// Array to keep track of free and taken holes
	unsigned int *free_holes = calloc(mp.bitmap_size, (sizeof (unsigned long)));
	unsigned int *taken_holes = calloc(mp.bitmap_size, (sizeof (unsigned long)));
	
	unsigned long curr_free_block = 0;
	unsigned long curr_taken_block = 0;
	unsigned i;
	for(i = 0; i < mp.bitmap_size; i++){
		if(mp.bitmap[i] == TAKEN){
			if(curr_free_block != 0){
				m->size_free += curr_free_block;
				free_holes[m->num_free++] = curr_free_block;
				printf("Free Block of Size:	%lu\n", curr_free_block);
				curr_free_block = 0;
			}
			curr_taken_block++;
		}
		else{
			if(curr_taken_block != 0){
				m->size_taken += curr_taken_block;
				taken_holes[m->num_taken++] = curr_taken_block;
				printf("Taken Block of Size:	%lu\n", curr_taken_block);
				curr_taken_block = 0;
			}
			curr_free_block++;	
		}
	}
	
	// Conduct experiments with our arrays
	printf("\n\n---------------------DATA----------------------\n\n");
	//for(i = 0; i < m->num_free; i++) printf("%d\n",free_holes[i]);
	//printf("\n------------------------------------------------\n\n");
	//for(i = 0; i < m->num_free; i++) printf("%d\n",free_holes[i]);
	print_min(free_holes, taken_holes, m);
	print_max(free_holes, taken_holes, m);
	free(free_holes);
	free(taken_holes);
	printf("\n------------------------------------------------\n\n");
	//print_bitmap();
	return;
}

void count_holes(int handle){
	metrics *m = malloc(sizeof(metrics));
	*m = (metrics){0};
	switch(memalloc_mode[handle]){
		case BUDDY:
			//_free_buddy(NULL, 1); //force coalesce blocks
			count_holes_buddy(trees[handle], m);
			break;
		default:
			count_holes_list(m);
			break;
	}
	//print_memtree(trees[handle], 0);
	if(m->num_free == 0){printf("0 wasted bytes, incrementing to 1 to avoid division by 0 error during print\n"); m->num_free = 1;}
	if(m->num_taken == 0) m->num_taken = 1;
	printf("  number of wasted bytes: %d\n  average size of holes: %lu\n  number of bytes in use: %d\n  average size of in use block: %lu\n", 
			m->num_free, m->size_free/m->num_free, m->num_taken, m->size_taken/m->num_taken);

}
