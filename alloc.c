#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define true 1
#define false 0
typedef struct {
	unsigned count;
	long beg;
	long end;

} mem_ptr;

mem_ptr mp;




int power2(long x){
	return (x & (x-1))==0;
}

int buddy_init(long n_bytes, int parm1, mem_ptr mp){	
	if (power2(n_bytes)!=0)
		return -1;
	mp.beg = malloc(n_bytes);
	mp.count++;
}

int meminit(long n_bytes, unsigned int flags, int parm1){
	int rv;
	if((flags & 0x1)==0x1){
		mp.count = 0;	
		rv = buddy_init(n_bytes, parm1, mp);
		
	}
	else if((flags & 0x4)==0x4){
		if((flags & 0x08)==0x08){}
		else if ((flags & 0x10)==0x10){}
		else if ((flags & 0x20)==0x20){}
		else if ((flags & 0x40)==0x40){}
		else{
			printf("Invalid bits set: %b\n", flags);
		}
	}
	return rv;
}
