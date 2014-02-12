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

} mem_ptr;

mem_ptr mp;




int power2(long x){
	return (x & (x-1))==0;
}

int buddy_init(long n_bytes, int parm1, mem_ptr mp){	
	if (power2(n_bytes)!=0)
		return -1;
	mp.beg = malloc(n_bytes);
	if (mp.beg == NULL)
		return -1;
	mp.page_size = pow(2, parm1);
}

int meminit(long n_bytes, unsigned int flags, int parm1, int* parm2){
	int rv;
	mp.count = 0;	
	if((flags & 0x1)==0x1){
		printf("0x1");
		rv = buddy_init(n_bytes, parm1, mp);
		
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
