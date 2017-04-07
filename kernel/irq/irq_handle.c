#include "x86/x86.h"
#include "game.h"

#define NR_IRQ_HANDLE 32
#define NR_HARD_INTR 16

struct IRQ_t{
	void (*call)(void);
	struct IRQ_t *next;
};

void do_syscall(TrapFrame *);
static struct IRQ_t handle_pool[NR_IRQ_HANDLE];
static struct IRQ_t *handles[NR_HARD_INTR];
static int handle_num=0;

void add_irq_handle(int irq,void (*func)(void)){
	assert(irq<NR_HARD_INTR);
	assert(handle_count<=NR_IRQ_HANDLE);
	struct IRQ_t *ptr;
	ptr=&handle_pool[handle_num++];
	ptr->call=func;
	ptr->next=handles[irq];
	handles[irq]=ptr;
}

/* TrapFrame的定义在include/x86/memory.h
 * 请仔细理解这段程序的含义，这些内容将在后续的实验中被反复使用。 */
void irq_handle(struct TrapFrame *tf) {
	if(tf->irq==0x80){
		do_syscall(tf);
	}else if(tf->irq < 1000) {
		if(tf->irq == -1) {
			printk("%s, %d: Unhandled exception!\n", __FUNCTION__, __LINE__);
		}
		else {
			printk("%s, %d: Unexpected exception #%d!\n", __FUNCTION__, __LINE__, tf->irq);
		}
		assert(0);
	}else{
		int handle_id=irq-1000;
		assert(handle_id<NR_HARD_INTR);
		struct IRQ_t *irq_func=handles[handle_id];
		while(irq_func!=NULL){
			irq_func->call();
			irq_func=irq_func->next;
		}
	}
}

