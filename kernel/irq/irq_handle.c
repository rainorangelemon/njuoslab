#include "common.h"
#include "irq.h"
#include "x86/memory.h"
#include "memory.h"

#define NR_IRQ_HANDLE 32
#define NR_HARD_INTR 16 /* At most 16 kinds of hardware interrupts. */

struct IRQ_t {
	void (*routine)(void);
	struct IRQ_t *next;
};

static struct IRQ_t handle_pool[NR_IRQ_HANDLE];
static struct IRQ_t *handles[NR_HARD_INTR]; // handles is an array of lists
static int handle_count = 0;

void do_syscall(struct TrapFrame4p *);

void add_irq_handle(int irq, void (*func)(void) ) {
	assert(irq < NR_HARD_INTR);
	assert(handle_count <= NR_IRQ_HANDLE);

	struct IRQ_t *ptr;
	ptr = &handle_pool[handle_count ++]; // get a free handler
	ptr->routine = func;
	ptr->next = handles[irq]; // insert into the linked list
	handles[irq] = ptr;
}

void irq_handle(struct TrapFrame4p *tf) {
	//printk("irq_handle(), irq=%d, eip=0x%x\n", tf->irq, tf->eip);


	int irq = tf->irq;
	if(current_pid!=-1){
		TrapFrame4p *temp=(void*)pcb[current_pid].kstack;
		*temp=*tf;
	}

	if(irq == 0x80) do_syscall(tf);
	else if(irq < 1000){
		switch(irq){ 
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
						 if((tf->err & 0x1) == 0) 
							printk("The page does not present!\n");
						 else{
							 if(tf->err & 0x2) printk("The access causing the fault was a write.\n");
							 else printk("The access causing the fault was a read.\n");
							 if(tf->err & 0x4) printk("The processor was executing in user mode.\n");
							 else printk("The processor was executing in supervisor mode.\n");
						 }
						 break;
			}
			default: printk("Unexisted exception!\n"); break;
		}
		panic("Unhandled exception! irq==%d,error_code=0x%x\n", irq, tf->err);
	}else {
		int irq_id = irq - 1000;
		assert(irq_id < NR_HARD_INTR);
		struct IRQ_t *f = handles[irq_id];

		while(f != NULL) {
			f->routine();
			f = f->next;
		}
	}
}



