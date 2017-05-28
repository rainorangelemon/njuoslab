#include "x86.h"
#include "elf.h"
#include "common.h"
#include "string.h"
#include "mmu.h"
#include "memory.h"
#include "video.h"
#include "system.h"
#include "memlayout.h"
#include "process/pm.h"
#include "irq.h"
#include "pmap.h"
#include "game.h"
#include "device/keyboard.h"

extern pde_t entry_pgdir[];
void init_cond();
void init_segment();
void init_mem();
/*void *loader();*/

int kernel_main()
{
	page_init();
	init_cond();
	return 0;
}
extern struct PageInfo pages[];

void init_cond()
{
	add_irq_handle(1,Keyboard_event);
	init_segment();
	init_idt();
	init_intr();
	init_serial();
	init_timer();
	init_mem();
	add_irq_handle(0,kernel_timer_event);
	asm volatile("cli");
	pcb_init();
	pcb_create();
	pcb_run(&pcbs[0]);
	printk("your life ends here\n");
	while(1){
		printk("here is init process\n");	
	}
}
