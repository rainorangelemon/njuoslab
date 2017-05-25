#include "../include/elf.h"
#include "../include/mmu.h"
#include "../include/x86.h"
#include "../include/types.h"
#include "../include/memlayout.h"
#include "../include/stdio.h"
#include "../include/common.h"
#include "../include/string.h"
#include "../include/pmap.h"
#include "../include/irq.h"
#include "../include/trap.h"

#define SECTSIZE 512

void waitdisk(void)
{
	while((inb(0x1F7) & 0xC0) != 0x40); 
}

void readsect(void *dst, int offset)
{
	int i;
	waitdisk();
	outb(0x1F2, 1);
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0);
	outb(0x1F7, 0x20);

	waitdisk();
	for (i = 0; i < SECTSIZE / 4; i ++) 
		((int *)dst)[i] = inl(0x1F0);
}


void readseg(unsigned char *pa, int count, int offset)
{
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 201;
	for(; pa < epa; pa += SECTSIZE, offset ++)
		readsect(pa, offset);
}

