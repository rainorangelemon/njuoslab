#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "common.h"

#define SCR_WIDTH		320
#define SCR_HEIGHT		200
#define SCR_DEPTH		1
#define SCR_WIDTH_SIZE	(SCR_WIDTH * SCR_DEPTH)
#define SCR_SIZE		((SCR_WIDTH) * (SCR_HEIGHT) * (SCR_DEPTH))
//#define VMEM_ADDR		0xFD000000
//uint32_t VMEM_ADDR;

extern uint8_t *vmem;

void init_vmem_addr();
void init_vmem();
int load_vmem(uint8_t*);

/*
void clear_screen();
void clear_buffer();
void display_buffer();
void drawRect_buffer(const uint8_t*,int,int,int,int);
void flyying_logo();
void show_police();
*/

/*
static inline void
draw_pixel(int x, int y, int color) {
	assert(x >= 0 && y >= 0 && x < SCR_HEIGHT && y < SCR_WIDTH);
	vmem[(x << 9) + (x << 8) + (x << 5) + y] = color;
} */

#endif
