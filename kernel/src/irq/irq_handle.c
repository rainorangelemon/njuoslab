#include "x86.h"
#include "stdio.h"
#include "assert.h"
#include "pcb.h"
#include "pcb_struct.h"

void do_syscall(struct TrapFrame *tf);
static void (*do_timer)(void);
static void (*do_keyboard)(int);

struct PCB *current;

void
set_keyboard_intr_handler( void (*ptr)(int) ) {
	do_keyboard = ptr;
}

void 
set_timer_intr_handler( void (*ptr)(void) ) {
	do_timer = ptr;
}

uint32_t time_tick;

uint32_t code = 19; //global var
void schedule();	//调度函数

void Keyboard_event();

/* TrapFrame的定义在include/memory.h
 * 请仔细理解这段程序的含义，这些内容将在后续的实验中被反复使用。 */
void
irq_handle(struct TrapFrame *tf) {
//	printk("irq %d\n", tf->irq);
	if(tf->irq == 0x80)
	{
			current->tf = tf;
			do_syscall(tf);
	}
	else if(tf->irq < 1000) {
		if(tf->irq == 14)
		{
			printk("page fault\n");
			while(1);
		}
		if(tf->irq == -1) {
			printk("%s, %d: Unhandled exception! error_code %d\n", __FUNCTION__, __LINE__, tf->error_code);
			while(1);
		}
		else {
			printk("%s, %d: Unexpected exception #%d! error_code %d\n", __FUNCTION__, __LINE__, tf->irq, tf->error_code);
			while(1);
		}
	}
	else if (tf->irq == 1000) {
		
		time_tick++;
		wakeup();		//所有block状态的进程的sleep--
		if(current->time_count == 10)//进程执行了一定次数的时间片
		{
//	printk("time over, schedule now\n");
			current->tf = tf;
			pcb_ready(current->pcb_index);
			schedule();
		}
		else
		{
			current->tf = tf;
			current->time_count++;
		}
	} else if (tf->irq == 1001) {
		current->tf = tf;
		Keyboard_event();
	} else if(tf->irq == 1014) {
		current->tf = tf;
	}
	else {
		printk("wrong\n");
	}
}

