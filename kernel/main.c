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
void *loader();

int kernel_main()
{
	page_init();
	init_cond();
	return 0;
}
extern struct PageInfo pages[];

void init_cond()
{
	set_keyboard_intr_handler(Keyboard_event);
	init_segment();
	init_idt();
	init_intr();
	init_serial();
	init_timer();
	init_mem();
	set_timer_intr_handler(kernel_timer_event);
	asm volatile("cli");
	pcb_init();
	pcb_create();
	pcb_run(&pcbs[0]);
}
