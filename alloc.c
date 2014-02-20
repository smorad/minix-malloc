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
void* data[1024];	//our memory contents

int btree_count = 0;

int power2(long x){
	return (x & (x-1))==0;
}

void btree_debug(btree parent){
	//printf("parent size: %lu beg: %lu end: %lu\n", parent->size, parent->seg_beg, parent->seg_end);
	//printf("lchild_ptr: %p rchild_ptr: %p\n", 
	//	parent->lchild->seg_start, parent->rchild->seg_start);

}

void print_memtree(btree root, int start){
	start++;
	if(root->rchild!=NULL)
		print_memtree(root->rchild, start);
	int i;
	for(i=0; i<=start; i++)
		printf("     ");
	printf("%lu\n", root->size);
	if(root->lchild !=NULL)
		print_memtree(root->lchild, start);
}

btree insert_node(long begin, long end, void* data){
	btree new = malloc(sizeof(struct binary_tree));
	assert(new!=NULL);
	new->size = end - begin;
	//if(begin==0)
	//	new->size++;
	assert(new->size!=0);
	new->lchild = NULL;
	new->rchild = NULL;
	new->taken = false;
	new->seg_start = data + begin;
	new->seg_beg = begin;
	new->seg_end = end;
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
	trees[btree_count] = insert_node(0, n_bytes, NULL); //change me for pages
	printf("ROOT NODE FOR HANDLE %d IS %p\n", btree_count, trees[btree_count]);
	//trees[btree_count]->seg_start = (void*)malloc(n_bytes+10); //10 extra bytes just in case
	data[btree_count] = malloc(n_bytes+10);
	if (trees[btree_count] == NULL){
		printf("beg = NULL\n");
		return ERROR;
	}
	return btree_count++;
}

btree find_by_region(btree root, void* region){	//will return node with segment, also will coalesce empty blocks
	if((root-> lchild !=NULL && root->lchild->taken == 0 )&&( root->rchild !=NULL && root->rchild->taken==0)){//coalesce
		if(root->rchild->rchild == NULL && root->rchild->lchild == NULL && root->lchild->rchild == NULL && root->lchild->lchild == NULL){//make sure rchild and lchild are leaves
			free(root->lchild);//might need to free all children's children recursively too
			free(root->rchild);
			root->rchild = NULL;
			root->lchild = NULL;
		}

	}	
	if(root->lchild!=NULL)
		find_by_region(root->lchild, region);

	if(root->rchild!=NULL)
		find_by_region(root->rchild, region);
	
	//if leaf
	if(root->seg_start == region)
		return root;
}


void _free_buddy(void* region){
	int i;
	btree found_node = NULL;
	for(i=0; i<btree_count; i++){
		found_node = find_by_region(trees[btree_count], region);
	}
	if(found_node!=NULL){
		found_node->taken = 0;
	}
}

btree result_ptr = NULL; //in case we return null after finding value

void* _buddy_alloc(long n_bytes, btree root, void* data){
	//traverse in order
	if (root==NULL) return;
	if (result_ptr != NULL) return; //found our result, don't want to overwrite
  if(root->size < n_bytes){//too deep into the tree
            //printf("--PARENT TOO SMALL-- ptr: %p n_bytes: %lu size: %lu\n", root, n_bytes, root->size);
    return result_ptr;
  }		
		if(root->lchild != NULL)
			_buddy_alloc(n_bytes, root->lchild, data);
		if(root->rchild != NULL)
			_buddy_alloc(n_bytes, root->rchild, data);
		if(root->lchild == NULL && root->rchild == NULL){ 	//if leaf
			if(root->taken==0 && (root->size == n_bytes)){		//if block is empty and correct size
				printf("***FOUND***");
				printf("beg: %lu end: %lu\n", root->seg_beg, root->seg_end);
				printf("psize: %lu n_bytes: %lu ptr: %lu mem_seg: %p\n\n\n ", root->size, n_bytes, root, root->seg_start);
				root->taken = true;
				result_ptr = root->seg_start;
				return root->seg_start;
			}
			else{
				//printf("creating child\n");
				//split block into children
				//these conflict, fix later
/*				if(((root->seg_end/2)-root->seg_beg) < n_bytes){ 
					//printf("child would be too small\n");
					return result_ptr;	//just in case so we don't get stuck in inf loop
				}*/
				root->lchild = insert_node(root->seg_beg, ((root->seg_beg + root->seg_end)/2), data);
				root->rchild = insert_node(((root->seg_beg + root->seg_end)/2)+1, root->seg_end+1, data);
				//check again starting at current node
				btree_debug(root);
				_buddy_alloc(n_bytes, root, data);
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
		// Check to see if there is enough free space
		if(count >= size && count >= worst){
			worst_index = i;
			worst = count;
		}
		if(mp.bitmap[i] == TAKEN) count = 0;
		else count++;
	}
	if(worst_index != ERROR){
		printf("Found location for worst here: %d\n", (worst_index - worst));
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
	switch(handle){
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
    			bitmap_loc = worst_area_free(curr_size);
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
	if(flags==0x1){
		//printf("0x1\n");
		rv = _buddy_init(n_bytes, parm1);
		
	}
	else if(flags==(0x00 | 0x4)){
		printf("FIRST FIT\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) rv = FIRST;
	}
	else if (flags==(0x08| 0x4)){
		printf("NEXT FIT\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) rv = NEXT;
	}
	else if (flags==(0x10| 0x4)){
		printf("BEST FIT\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) rv = BEST;
	}
	else if (flags==(0x20| 0x4)){
		printf("WORST FIT\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) rv = WORST;
	}
	else if (flags==(0x40| 0x4)){
		printf("RANDOM FIT\n");
		rv = list_init(n_bytes, parm1, parm2);
		if(rv != ERROR) rv = RANDOM;
	}
	else{
		printf("Invalid bits set: %#010x\n", flags);
	}
	return rv;
}

void* memalloc(long n_bytes, int handle){
	switch(handle){
		case BUDDY:
			result_ptr = NULL;
			return _buddy_alloc(n_bytes, trees[handle], data[handle]);
		case ERROR:
			printf("Memory not initialized correctly\n");
			return NULL;
		default:
			printf("n_bytes: %lu\n", n_bytes);
			return list_memalloc(n_bytes, handle);
	}
	return NULL;
}

void memfree(void *region){
	void *find_len;
	unsigned mem_index, bitmap_index;
	long free_size;
	mem_index = (unsigned)(region - mp.beg);
	bitmap_index = (mem_index/mp.page_size);
	find_len = region - (sizeof(long));
	free_size = *((long *)find_len);
	printf("Bitmap index freed: %u		of size: %d\n", bitmap_index, free_size );
	mark_mem(bitmap_index, free_size, FREE);
}
