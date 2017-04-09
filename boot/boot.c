#include "x86.h"
#include "elf.h"
//#include "boot.h"

#define SECTSIZE 512
#define KERNEL_OFFSET_IN_DISK 0

void readseg(unsigned char*,int,int);

int bootmain(void)
{
	struct Elf *elf;
	struct Proghdr *ph, *eph;
	unsigned char *pa, *i;

	elf = (struct Elf*)0x8000;

	readseg((unsigned char*)elf, 4096, KERNEL_OFFSET_IN_DISK);

	ph = (struct Proghdr*)((char *)elf + elf->e_phoff);
	eph = ph + elf->e_phnum;
	for(; ph < eph; ph ++) {
		pa = (unsigned char*)ph->p_pa;
		readseg(pa, ph->p_filesz, KERNEL_OFFSET_IN_DISK + ph->p_offset);
		for(i = pa + ph->p_filesz; i < pa + ph->p_memsz; *i ++ = 0);
	}

	((void(*)(void))elf->e_entry)(); /* Here we go! */

	while(1);
}




void waitdisk(void) {
	while((inb(0x1f7) & 0xc0) != 0x40);
}

void readsect(void *dst, int offset) {
	/* int i; */
	waitdisk();

	outb(0x1f2, 1);		// count = 1
	outb(0x1f3, offset);
	outb(0x1f4, offset >> 8);
	outb(0x1f5, offset >> 16);
	outb(0x1f6, (offset >> 24) | 0xe0);
	outb(0x1f7, 0x20);	// cmd 0x20 - read sectors

	waitdisk();

	insl(0x1f0, dst, SECTSIZE/4);	//read a sector
	/*	this part does the same thing
	for(i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = in_long(0x1f0);
	} */
}

void readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
		readsect(pa, offset);
}
