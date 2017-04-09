#include "common.h"
#include "device/video.h"
#include "x86.h"
#include "elf.h"
#include "string.h"
#include "memory.h"
#include "irq.h"
#include "x86/memory.h"


#define SECTSIZE 512
#define GAME_OFFSET_IN_DISK (10 * 1024 * 1024)
void readseg(unsigned char*,int,int);

void init_page();
void init_segment();
void init_vmem_addr();
void init_serial();
void init_i8259();
void init_idt();
void init_timer();
void add_irq_handle(int,void (*)(void));
void timer_event();
void keyboard_event();

void printk_test();
void serial_output_test();

PCB* create_process(uint32_t disk_offset);
void set_trapframe(TrapFrame4p*,uint32_t);

int main(void);
void init(){
	init_page();
	asm volatile("addl %0,%%esp" : : "i"(KOFFSET));
	asm volatile("jmp *%0" : : "r"(main));
	panic("wrong in the initi()");
}

void INIT_WORK(){
	init_segment();
	init_vmem_addr();
	init_vmem();
	init_pcb();
	init_pte_info();
	init_serial();
	init_i8259();
	init_idt();
	init_timer();
	add_irq_handle(0, timer_event);
	add_irq_handle(1, keyboard_event);
}

void TEST_WORK(){
	serial_output_test();
	printk_test();
}

int main(void) {

	INIT_WORK(); //while(1);
	TEST_WORK();

	printk("Here is main()\n");

	//sti(); hlt(); cli(); while(1);
	/*
	struct ELFHeader *elf;
	struct ProgramHeader *ph, *eph;
	unsigned char *pa, *i;

	//elf = (struct ElfHeader*)0x1f00000;
	uint8_t buf[4096];
	elf = (struct ELFHeader*)buf;
	printk("addr of buf: 0x%x\n", (uint32_t)buf);

	readseg((unsigned char*)elf, 4096, GAME_OFFSET_IN_DISK);

	ph = (struct ProgramHeader*)((char *)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(; ph < eph; ph ++) {
		pa = (unsigned char*)ph->paddr;
		printk("0x%x, 0x%x, 0x%x\n", pa, ph->filesz, ph->memsz);
		readseg(pa, ph->filesz, GAME_OFFSET_IN_DISK + ph->off);
		for(i = pa + ph->filesz; i < pa + ph->memsz; *i ++ = 0);
	}
	*/
	PCB *pcb_p=create_process(GAME_OFFSET_IN_DISK);
	set_trapframe((void*)pcb_p->kstack,pcb_p->entry);
	printk("here we would go!\n");

	asm volatile("movl %0, %%esp" : :"a"(pcb_p->kstack));
	asm volatile("popal;\
			pushl %eax;\
			movw 4(%esp),%ax;\
			movw %ax,%gs;\
			movw %ax,%fs;\
			movw %ax,%es;\
			movw %ax,%ds;\
			popl %eax;\
			addl $0x18,%esp;\
			iret");

	//sti(); hlt(); cli(); while(1);

	panic("YOU shouldn't get here!\n");
	return 0;
}

/*
void waitdisk(void) {
	while((inb(0x1f7) & 0xc0) != 0x40);
}

void readsect(void *dst, int offset) {
	waitdisk();

	outb(0x1f2, 1);		// count = 1
	outb(0x1f3, offset);
	outb(0x1f4, offset >> 8);
	outb(0x1f5, offset >> 16);
	outb(0x1f6, (offset >> 24) | 0xe0);
	outb(0x1f7, 0x20);	// cmd 0x20 - read sectors

	waitdisk();

	insl(0x1f0, dst, SECTSIZE/4);	//read a sector
}

void readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
		readsect(pa, offset);
}
*/
