#include "x86.h"
#include "elf.h"
//#include "boot.h"

#define SECTSIZE 512
#define KERNEL_OFFSET_IN_DISK 0

void readseg(unsigned char*,int,int);

int bootmain(void)
{
	struct Elf *elf;
	struct Proghdr *phr, *eph;
	unsigned char *pa, *i;

	elf = (struct Elf*)0x8000;

	readseg((unsigned char*)elf, 4096, KERNEL_OFFSET_IN_DISK);

	phr = (struct Proghdr*)((char *)elf + elf->e_phoff);
	eph = phr + elf->e_phnum;
	for(; phr < eph; phr ++) {
		pa = (unsigned char*)phr->p_pa;
		readseg(pa, phr->p_filesz, KERNEL_OFFSET_IN_DISK + phr->p_offset);
		for(i = pa + phr->p_filesz; i < pa + phr->p_memsz; *i ++ = 0);
	}

	((void(*)(void))elf->e_entry)(); /* The real program starts here */

	while(1);
}




void waitdisk(void) {
	while((inb(0x1f7) & 0xc0) != 0x40);
}

void readsect(void *dst, int offset) {
	waitdisk();

	outb(0x1f2, 1);		
	outb(0x1f3, offset);
	outb(0x1f4, offset >> 8);
	outb(0x1f5, offset >> 16);
	outb(0x1f6, (offset >> 24) | 0xe0);
	outb(0x1f7, 0x20);

	waitdisk();

	insl(0x1f0, dst, SECTSIZE/4);	//read one part of sectors
}

void readseg(unsigned char *pa, int count, int off) {
	unsigned char *epa;
	epa = pa + count;
	pa -= off % SECTSIZE;
	off = (off / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, off ++)
		readsect(pa, off);
}
