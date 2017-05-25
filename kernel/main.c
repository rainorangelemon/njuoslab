#include "include/x86.h"
#include "include/elf.h"
#include "include/common.h"
#include "include/string.h"
#include "include/mmu.h"
#include "include/memory.h"
#include "include/video.h"
#include "include/system.h"
#include "include/memlayout.h"
#include "include/process/env.h"
#include "include/irq.h"
#include "include/pmap.h"
#include "include/game.h"
#include "include/device/keyboard.h"

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
	env_init();
	env_create();
	env_run(&envs[0]);
}
