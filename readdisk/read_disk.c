#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "disk.h"

struct map bitmap;
struct dir direct;

int main(int argc, char *argv[])
{
	FILE *fp = fopen(argv[1], "rb");
    if(fp == NULL)
    {
	    printf("open failed\n");
        exit(0);
    }
    fread(&bitmap, 1, sizeof(bitmap), fp);
	int k;
	for(k = 0; k < 40; k ++)
		printf("%02x", bitmap.mask[k].byte);
	printf("\n");

    fread(&direct, 1, sizeof(direct), fp);

    int i;
    char buf[513];
    for(i = 0; i < NR_DIR_ENTRY; i ++)
    {
	    if(direct.entry[i].inode_offset != -1)
        {
	        struct iNode inode;
            int j;
            for(j = 0; j < NR_INODE_ENTRY; j ++)
            {
	            if(inode.data_block_offset[j] != -1)
                {
	                memset(buf, '\0', sizeof(buf));
                    fseek(fp, inode.data_block_offset[j] * 512, SEEK_SET);
					fread(buf, 1, 512, fp);
					printf("%s", buf);
				}
			}
		}
	}
}

