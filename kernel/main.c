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
	add_irq_handle(0, schedule);
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

	current_pid=-1;
	PCB *pcb_p=create_process(GAME_OFFSET_IN_DISK);
	set_trapframe((void*)pcb_p->kstack,pcb_p->entry);
	printk("here we would go!\n");

	pop_tf_process((void*)pcb_p->kstack);
	

	//sti(); hlt(); cli(); while(1);

	printk("this is init process");
	while(1);

	panic("YOU shouldn't get here!\n");
	return 0;
}

