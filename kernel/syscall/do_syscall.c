#include "common.h"
#include "irq.h"
#include "x86/memory.h"
#include "process.h"

enum {SYS_write,SYS_time,SYS_kbd,SYS_video,SYS_fork,SYS_sleep,SYS_exit,SYS_getpid};

extern uint32_t time_tick;
int fs_write(int,void*,int);
int handle_keys();
int load_vmem(uint8_t*);

void do_syscall(struct TrapFrame4p *tf){
	switch(tf->eax){
		case SYS_write:
			tf->eax=fs_write(tf->ebx,(void*)tf->ecx,tf->edx);
			break;
		case SYS_time: 
			tf->eax=time_tick;
			break;
		case SYS_kbd:  
			tf->eax=handle_keys();
			break;
		case SYS_video:
			tf->eax=load_vmem((uint8_t*)tf->ebx);
			break;
		case SYS_fork:
			tf->eax=system_fork();
			break;
		case SYS_sleep:
			printk("it is going into sleep for %d",(uint32_t)tf->ebx);
			system_sleep((uint32_t)tf->ebx);
			break;
		case SYS_exit:
			system_exit();
			break;
		case SYS_getpid:
			tf->eax=getpid();
			break;
		default:
			panic("No such call: id=%d",tf->eax);
	}
}
