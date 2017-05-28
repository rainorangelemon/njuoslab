#include "include/types.h"
#include "include/string.h"
#include "include/video.h"
#include "include/syscall.h"
#include "include/font.h"
/* 绘制屏幕的帧缓冲实现。
 * 在某些版本的qemu-kvm上，由于每次访问显存映射区域都会产生一次VM exit，
 * 更新屏幕的速度可能非常缓慢从而引起游戏跳帧。定义宏SLOW以只重绘屏幕变化
 * 的部分；
 * */

//uint8_t *vmem = VMEM_ADDR;
static uint8_t vbuf[SCR_SIZE];
uint8_t *vmem = vbuf;
void syscall(int id, ...);
int printf(const char *fmt, ...);

void
prepare_buffer(void) {
	memset(vmem, 0, SCR_SIZE);
}

void
display_buffer(void) {
	syscall(VMEMORY, vbuf);
}


static inline void
draw_character(char ch, int x, int y, int color) {
	int i, j;
//	assert((ch & 0x80) == 0);
	char *p = font8x8_basic[(int)ch];
	for (i = 0; i < 8; i ++)
		for (j = 0; j < 8; j ++)
			if ((p[i] >> j) & 1)
				draw_pixel(x + i, y + j, color);
}


void
draw_string(const char *str, int x, int y, int color) {
	while (*str) {
		draw_character(*str ++, x, y, color);
		if (y + 8 >= SCR_WIDTH) {
			x += 8; y = 0;
		} else {
			y += 8;
		}
	}
}

void draw_obj(int x, int y, int len, int wid)
{
	int i,j;
	for(i=0;i<=wid;i++)
	  for(j=0;j<=len;j++)
		vmem[(x+i)*320+(y+j)]=1;
}

void
init_screen() {
//	printf("vmem %x\n",vmem);
	for(int i = 0; i < 3; i ++)
	  for(int j = 0; j < 194; j ++)
		vmem[i*320+j] = 7;
	for(int i=197; i<200; i++)
	  for(int j=0; j<194; j++)
		vmem[i*320+j] = 7;
	for(int i=0; i<194; i++)
	{
		vmem[41*320+i]=7;
		vmem[80*320+i]=7;
		vmem[119*320+i]=7;
		vmem[158*320+i]=7;
	}
	for(int i=0; i<200; i++)
	{
		vmem[i*320+38]=7;
		vmem[i*320+77]=7;
		vmem[i*320+116]=7;
		vmem[i*320+155]=7;
		vmem[i*320+194]=7;
	}
	for(int i=0; i<200; i++)
	  for(int j=0; j<320; j++)
		if(vmem[i*320+j]!=7)
		  vmem[i*320+j]=9;
}

//存在游戏逻辑和写显存的交错

int
site[2][25]={{3, 3, 3,  3,  3, 42,42,42, 42, 42, 81,81,81, 81, 81, 120,120,120,120,120, 159,159,159,159,159},
			 {0,39,78,117,156,  0,39,78,117,156,  0,39,78,117,156,   0, 39, 78,117,156,   0, 39, 78,117,156}};

int state[5][5]={{0,0,0,0,0},
				 {0,0,0,0,0},
				 {0,0,0,0,0},
				 {0,0,0,0,0},
				 {0,0,0,0,0}};

void draw_success()
{
	int count=0;
	for(int i=0; i<5; i++)
	  for(int j=0; j<5; j++)
		if(state[i][j]==1)
		  count++;
	if(count==25)
	{
		draw_string("You win!", 50,220,47);
		draw_string("Press \"r\"", 60, 218, 47);
		draw_string("to restart", 70, 218, 47);
	}

}

void draw_light(int len, int wid)
{
	for(int i=0; i<5; i++)
	  for(int j=0; j<5; j++)
	  {
		  if(state[i][j]==1)
		  {
			  int x=site[0][i*5+j];
			  int y=site[1][i*5+j];
			  for(int m=0; m<len; m++)
				for(int n=0; n<wid; n++)
				  vmem[(x+m)*320+(y+n)]=12;
		  }
		  else
		  {
			  int x=site[0][i*5+j];
			  int y=site[1][i*5+j];
			  for(int m=0; m<len; m++)
				for(int n=0; n<wid; n++)
				  vmem[(x+m)*320+(y+n)]=9;
		  }
	  }
}

//游戏逻辑部分，和显存无关

void reset()
{
	for(int i=0; i<5; i++)
	  for(int j=0; j<5; j++)
		state[i][j]=0;
}

void light_state(int x, int y)
{
	if(x==site[0][0] && y==site[1][0])
	{
		state[0][0]=!state[0][0];
		state[0][1]=!state[0][1];
		state[1][0]=!state[1][0];
	}
	else if(x==site[0][1] && y==site[1][1])
	{
		state[0][0]=!state[0][0];
		state[0][1]=!state[0][1];
		state[0][2]=!state[0][2];
		state[1][1]=!state[1][1];
	}
	else if(x==site[0][2] && y==site[1][2])
	{
		state[0][1]=!state[0][1];
		state[0][2]=!state[0][2];
		state[0][3]=!state[0][3];
		state[1][2]=!state[1][2];
	}
	else if(x==site[0][3] && y==site[1][3])
	{
		state[0][2]=!state[0][2];
		state[0][3]=!state[0][3];
		state[0][4]=!state[0][4];
		state[1][3]=!state[1][3];
	}
	else if(x==site[0][4] && y==site[1][4])
	{
		state[0][4]=!state[0][4];
		state[0][3]=!state[0][3];
		state[1][4]=!state[1][4];
	}
	else if(x==site[0][5] && y==site[1][5])
	{
		state[1][0]=!state[1][0];
		state[0][0]=!state[0][0];
		state[1][1]=!state[1][1];
		state[2][0]=!state[2][0];
	}
	else if(x==site[0][6] && y==site[1][6])
	{
		state[1][1]=!state[1][1];
		state[0][1]=!state[0][1];
		state[1][0]=!state[1][0];
		state[1][2]=!state[1][2];
		state[2][1]=!state[2][1];
	}
	else if(x==site[0][7] && y==site[1][7])
	{
		state[1][2]=!state[1][2];
		state[0][2]=!state[0][2];
		state[1][1]=!state[1][1];
		state[1][3]=!state[1][3];
		state[2][2]=!state[2][2];
	}
	else if(x==site[0][8] && y==site[1][8])
	{
		state[1][3]=!state[1][3];
		state[0][3]=!state[0][3];
		state[1][2]=!state[1][2];
		state[1][4]=!state[1][4];
		state[2][3]=!state[2][3];
	}
	else if(x==site[0][9] && y==site[1][9])
	{
		state[1][4]=!state[1][4];
		state[0][4]=!state[0][4];
		state[1][3]=!state[1][3];
		state[2][4]=!state[2][4];
	}
	else if(x==site[0][10] && y==site[1][10])
	{
		state[2][0]=!state[2][0];
		state[1][0]=!state[1][0];
		state[2][1]=!state[2][1];
		state[3][0]=!state[3][0];
	}
	else if(x==site[0][11] && y==site[1][11])
	{
		state[2][1]=!state[2][1];
		state[1][1]=!state[1][1];
		state[2][0]=!state[2][0];
		state[2][2]=!state[2][2];
		state[3][1]=!state[3][1];
	}
	else if(x==site[0][12] && y==site[1][12])
	{
		state[2][2]=!state[2][2];
		state[1][2]=!state[1][2];
		state[2][1]=!state[2][1];
		state[2][3]=!state[2][3];
		state[3][2]=!state[3][2];
	}
	else if(x==site[0][13] && y==site[1][13])
	{
		state[2][3]=!state[2][3];
		state[1][3]=!state[1][3];
		state[2][4]=!state[2][4];
		state[2][2]=!state[2][2];
		state[3][3]=!state[3][3];
	}
	else if(x==site[0][14] && y==site[1][14])
	{
		state[2][4]=!state[2][4];
		state[1][4]=!state[1][4];
		state[2][3]=!state[2][3];
		state[3][4]=!state[3][4];
	}
	else if(x==site[0][15] && y==site[1][15])
	{
		state[3][0]=!state[3][0];
		state[2][0]=!state[2][0];
		state[3][1]=!state[3][1];
		state[4][0]=!state[4][0];
	}
	else if(x==site[0][16] && y==site[1][16])
	{
		state[3][1]=!state[3][1];
		state[2][1]=!state[2][1];
		state[3][0]=!state[3][0];
		state[3][2]=!state[3][2];
		state[4][1]=!state[4][1];
	}
	else if(x==site[0][17] && y==site[1][17])
	{
		state[3][2]=!state[3][2];
		state[2][2]=!state[2][2];
		state[3][1]=!state[3][1];
		state[3][3]=!state[3][3];
		state[4][2]=!state[4][2];
	}
	else if(x==site[0][18] && y==site[1][18])
	{
		state[3][3]=!state[3][3];
		state[2][3]=!state[2][3];
		state[3][2]=!state[3][2];
		state[3][4]=!state[3][4];
		state[4][3]=!state[4][3];
	}
	else if(x==site[0][19] && y==site[1][19])
	{
		state[3][4]=!state[3][4];
		state[2][4]=!state[2][4];
		state[3][3]=!state[3][3];
		state[4][4]=!state[4][4];
	}
	else if(x==site[0][20] && y==site[1][20])
	{
		state[4][0]=!state[4][0];
		state[3][0]=!state[3][0];
		state[4][1]=!state[4][1];
	}
	else if(x==site[0][21] && y==site[1][21])
	{
		state[4][1]=!state[4][1];
		state[3][1]=!state[3][1];
		state[4][0]=!state[4][0];
		state[4][2]=!state[4][2];
	}
	else if(x==site[0][22] && y==site[1][22])
	{
		state[4][2]=!state[4][2];
		state[3][2]=!state[3][2];
		state[4][1]=!state[4][1];
		state[4][3]=!state[4][3];
	}
	else if(x==site[0][23] && y==site[1][23])
	{
		state[4][3]=!state[4][3];
		state[3][3]=!state[3][3];
		state[4][2]=!state[4][2];
		state[4][4]=!state[4][4];
	}
	else if(x==site[0][24] && y==site[1][24])
	{
		state[4][4]=!state[4][4];
		state[3][4]=!state[3][4];
		state[4][3]=!state[4][3];
	}
}
