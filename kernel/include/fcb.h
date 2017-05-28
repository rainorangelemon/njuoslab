#ifndef __FCB_H__
#define __FCB_H__

#include "disk.h"

#define s_nonexist	-1
#define s_r		0
#define s_w  	1
#define s_rw	2
#define s_close 3

struct FCB {
	int f_state;			//文件状态
	int act_inode_index;
	int inode_bitoffset;	//inode位置
	int ch_offset;			//文件指针的字节偏移
	int buf_offset;			//缓冲区对应的块偏移
	char buf[512 * 4];
};

struct map bitmap;
struct dir direct;			//这两项必须在加载文件之前就从磁盘读出

//#define ACTIVE_INODE 	256
//struct iNode active_inode[ACTIVE_INODE];

void fcb_init();
int fcb_alloc();
void fcb_release();
void inode_init();
int inode_alloc();

void open(char*, int);
void read(int, char*, int);
void write(int, char*, int);
void lseek(int, int, int);
void close(int);

#endif
