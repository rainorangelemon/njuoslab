#include "common.h"
#include "irq.h"
#include "assert.h"
#include "mmu.h"
#include "x86.h"
#include "pm.h"
#include "types.h"
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

#define NR_IRQ_HANDLE 32
#define NR_HARD_INTR 16

void do_syscall(TrapFrame *tf);

struct IRQ_t{
	void (*routine)(void);
	struct IRQ_t *next;
};

struct IRQ_t handle_pool[NR_IRQ_HANDLE];
struct IRQ_t *handles[NR_HARD_INTR];
static int handle_count=0;


void add_irq_handle(int irq, void (*func)(void) ){
	assert(irq<NR_HARD_INTR);
	assert(handle_count<=NR_IRQ_HANDLE);
	struct IRQ_t *ptr;
	ptr=&handle_pool[handle_count++];
	ptr->routine=func;
	ptr->next=handles[irq];
	handles[irq]=ptr;
}

void irq_handle(struct TrapFrame *tf)
{
	asm volatile("cli");
	current_pcb->tf=*tf;
	if (tf->irq == 0x80)
		do_syscall(tf);
	else 
	if (tf->irq < 1000)
	{
		if(tf->irq == -1)
		{
			printk("%s, %d: Unhandled exception!\n", __FUNCTION__, __LINE__);
		}
		else
		{
			switch(tf->irq){ 
				case 0: printk("Divide Error!\n"); break;
				case 1: printk("Debug Exceptions!\n"); break;
				case 3: printk("Breakpoint Error!\n"); break;
				case 4: printk("Overflow Error!\n"); break;
				case 5: printk("Bounds Check Error!\n"); break;
				case 6: printk("Invalid Opcode Error!\n"); break;
				case 7: printk("Coprocessor Not Available!\n"); break;
				case 8: printk("Double Fault!\n"); break;
				case 9: printk("Coprocessor Segment Overrun!\n"); break;
				case 10: printk("Invalid TSS!\n"); break;
				case 11: printk("Segment Not Present!\n"); break;
				case 12: printk("Stack Exception!\n"); break;
				case 13: printk("General Protection Exception!\n"); break;
				case 14: {
						 printk("Fault in Page!\n");
						 if((tf->error_code & 0x1) == 0) 
							printk("The page does not present!\n");
						 else{
							 if(tf->error_code & 0x2) printk("The access causing the fault was a write.\n");
							 else printk("The access causing the fault was a read.\n");
							 if(tf->error_code & 0x4) printk("The processor was executing in user mode.\n");
							 else printk("The processor was executing in supervisor mode.\n");
						 }
						 break;
				}
				default: printk("Unexisted exception!\n"); break;
			}
			printk("%s, %d: Unexpected exception #%d!\n", __FUNCTION__, __LINE__, tf->irq);
		}
		assert(0);
	}
	else
	{
		int irq_id=tf->irq-1000;
		assert(irq_id<NR_HARD_INTR);
		struct IRQ_t *f=handles[irq_id];
		while(f!=NULL){
			f->routine();
			f=f->next;
		}
	}
	asm volatile("sti");
}
