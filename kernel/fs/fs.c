#include "common.h"

void serial_printc(char);

int fs_write(int fd,void *buf,int len){
	assert(fd==1);
	int ret=-1;
	if(fd==1){
		ret=len;
		int i;
		for(i=0;i<ret;++i){
			serial_printc(*(char*)(buf+i));
		}
	}else
		assert(0);
	return ret;
}
