#include "common.h"
#include "irq.h"
#include "x86/memory.h"

enum {SYS_write,SYS_time,SYS_kbd,SYS_video};

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
		default:
			panic("No such call: id=%d",tf->eax);
	}
}
