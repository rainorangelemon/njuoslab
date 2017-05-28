#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
#include <string.h>

struct map bitmap;

struct dir direct;

void bit_alloc(int *bitoffset)
{
	int i;
	for(i = BITMAP_USED_BYTE; i < BITMAP_SIZE; i ++)
	{
		if(bitmap.mask[i].byte != 0xff)
		{
			if(bitmap.mask[i].bit7 == 0)
			{
				bitmap.mask[i].bit7 = 1;
				*bitoffset = 0;
				break;
			}
			else if(bitmap.mask[i].bit6 == 0)
			{
				bitmap.mask[i].bit6 = 1;
				*bitoffset = 1;
				break;
			}
			else if(bitmap.mask[i].bit5 == 0)
			{
				bitmap.mask[i].bit5 = 1;
				*bitoffset = 2;
				break;
			}	
			else if(bitmap.mask[i].bit4 == 0)
			{
				bitmap.mask[i].bit4 = 1;
				*bitoffset = 3;
				break;
			}
			else if(bitmap.mask[i].bit3 == 0)
			{
				bitmap.mask[i].bit3 = 1;
				*bitoffset = 4;
				break;
			}
			else if(bitmap.mask[i].bit2 == 0)
			{
				bitmap.mask[i].bit2 = 1;
				*bitoffset = 5;
				break;
			}
			else if(bitmap.mask[i].bit1 == 0)
			{
				bitmap.mask[i].bit1 = 1;
				*bitoffset = 6;
				break;
			}
			else if(bitmap.mask[i].bit0 == 0)
			{
				bitmap.mask[i].bit0 = 1;
				*bitoffset = 7;
				break;
			}
		}
	}
	*bitoffset += i * 8;
}

int main(int argc, char *argv[])
{
	FILE *diskfp = fopen(argv[1], "wb");
	if(diskfp == NULL)
	{
		printf("create disk error!\n");
		exit(0);
	}
//-----------------init-------------------
	int i, j;
	for(i = 0; i < BITMAP_USED_BYTE; i ++)
	{
		bitmap.mask[i].byte = 0xff;
	}
	bitmap.mask[BITMAP_USED_BYTE].bit7 = 1;

	int dir_bitoffset = BITMAP_USED_BYTE * 8;
	for(j = 0; j < NR_DIR_ENTRY; j ++)
	{
		direct.entry[j].filename[0] = '\0';
		direct.entry[j].file_size = 0;
		direct.entry[j].inode_offset = -1;
	}
//----------------------------------------
	int k;
	int filesz;
	int bitoffset;
	for(k = 2; k < argc; k ++)
	{
		FILE *fp = fopen(argv[k], "rb");
		if(fp == NULL)
		{
			printf("open failed!\n");
			exit(1);
		}
	//	printf("open success\n");
		fseek(fp, 0, SEEK_END);
		filesz = ftell(fp);
		fseek(fp, 0, SEEK_SET);
//printf("file %s's size is %d\n", argv[k], filesz);
//----------------fill bitmap----------------
		bit_alloc(&bitoffset);
		//this number is the file's inode bit index, counting from 0
printf("this file's inode bit index is %d\n", bitoffset);
//--------------fill a direct entry--------------
		int index;
		for(index = 0; index < NR_DIR_ENTRY; index ++)
		{
			if(direct.entry[index].inode_offset == -1)
			{
				memcpy(direct.entry[index].filename, argv[k], strlen(argv[k]));
				direct.entry[index].file_size = filesz;
				direct.entry[index].inode_offset = bitoffset;
				break;
			}
		}
//printf("now, the directory is:\n");
//for(i = 0; i < 16; i ++)
//{
//	printf("%s %d %d\n",direct.entry[i].filename, direct.entry[i].file_size, direct.entry[i].inode_offset);
//}
	
//-------------write the data to disk block by block-------------
		struct iNode inode;
		//------init inode-------
		int inode_entry_index;
		for(inode_entry_index = 0; inode_entry_index <= NR_INODE_ENTRY; inode_entry_index ++)
			inode.data_block_offset[inode_entry_index] = -1;

		char buf[513] = "\0";
		int entry_index = 0;
		while(!feof(fp))
		{
		memset(buf, '\0', sizeof(buf));

			fread(buf, 1, 512, fp);
//printf("the buf is\n\t%s\n\n", buf);
			int inode_entry = 0;
			bit_alloc(&inode_entry);
//printf("the block's bit index for this buf is %d\n", inode_entry);
			if(entry_index < NR_INODE_ENTRY)
				inode.data_block_offset[entry_index]=inode_entry;
			else
			{
				printf("outof inode entry num!\n");
				exit(1);
			}
//----------------write data------------------
			fseek(diskfp, inode_entry * 512, SEEK_SET);
			fwrite(buf, 1, 512, diskfp);
			entry_index ++;
		}

//----------------write inode----------------
		fseek(diskfp, direct.entry[index].inode_offset * 512, SEEK_SET);
		fwrite(&inode, 1, 512, diskfp);		
	}

//----------------write root direct----------------
	fseek(diskfp, dir_bitoffset * 512, SEEK_SET);
	fwrite(&direct, 1, 512, diskfp);

//----------------write bitmap----------------
	fseek(diskfp, 0, SEEK_SET);
	fwrite(&bitmap, 256, 512, diskfp);
}
