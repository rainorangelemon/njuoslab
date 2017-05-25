#include "video.h"
#include "mmu.h"
#include "system.h"
#include "x86.h"
#include "elf.h"
#include "string.h"
#include "timer.h"
#include "keyboard.h"
#include "game.h"
#include "irq.h"
#include "../../memory/stdio.h"
#include "device/keyboard.h"

#ifndef SERIAL_PORT
#define SERIAL_PORT 0x3F8

static uint32_t *vmembase = (uint32_t*)VMEM_ADDR;

extern uint32_t time_tick;

int fs_write(int fd, void *buf, int len){
	int ret=-1;
	if(fd==1){
		ret=len;
		int i;
		for(i=0;i<ret;++i){
			serial_printc(*(char*)(buf+i));
		}
	}
	return ret;
}

void do_syscall(TrapFrame* tf)
{
	switch (tf->eax)
	{
		case SYS_fork:
			system_pcb_fork();
			break;
		case SYS_sleep:
			printk("sleep!!\n");
			system_pcb_sleep((uint32_t)tf->ebx);
			break;
		case SYS_exit:
			system_pcb_exit();
			break;
		case 0:
			set_timer_intr_handler((void*)tf->ebx);
			break;
		case 1:
			set_keyboard_intr_handler((void*)tf->ebx);
			break;
		case SYS_video:
		{	uint32_t *vbuf;
			int i;
			vbuf=(uint32_t*)tf->ebx;
			for(i=0;i<SCR_SIZE/4;i++){
				uint32_t *position=vmembase+i;
				(*position)=vbuf[i];
			}
		}
			break;
		case SYS_time:
			tf->eax=time_tick;
			break;
		case SYS_write:
			tf->eax=fs_write(tf->ebx,(void*)tf->ecx,tf->edx);
			break;
		case SYS_kbd:
			tf->eax=handle_keys();
			break;
		case ReKbdbuf:
			Refresh_Kbdbuf();
			break;	
 	}
}

#endif
