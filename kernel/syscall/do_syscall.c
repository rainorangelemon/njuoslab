#include "../include/video.h"
#include "../include/mmu.h"
#include "../include/system.h"
#include "../include/x86.h"
#include "../include/elf.h"
#include "../include/string.h"
#include "../include/timer.h"
#include "../include/keyboard.h"
#include "../include/game.h"
#include "../include/irq.h"
#include "../../memory/stdio.h"
#include "../include/device/keyboard.h"

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
		case env_fork:
			system_env_fork();
			break;
		case env_sleep:
			printk("sleep!!\n");
			system_env_sleep((uint32_t)tf->ebx);
			break;
		case env_exit:
			system_env_exit();
			break;
		case 0:
			set_timer_intr_handler((void*)tf->ebx);
			break;
		case 1:
			set_keyboard_intr_handler((void*)tf->ebx);
			break;
		case drawpixel:
		{
			int offset = tf->ebx + tf->ecx * SCR_WIDTH;
			uint32_t *position = vmembase + offset;
			(*position) = tf->edx;
		}
		break;
		case serialprint:
			while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
			outb(SERIAL_PORT,tf->ebx);
			break;
		case drawpixeloff:
		{
			uint32_t *position = vmembase + tf->ebx;
			(*position) = tf->ecx;
		}
		break;
		case clearscreen:
			memset((void*)vmembase,tf->ebx,SCR_SIZE);
			break;
		case initserial:
			init_serial();
			break;
		case inittimer:
			init_timer();
			break;
		case enableinterrupt:
			asm volatile("sti");
			break;
		case disenableinterrupt:
			asm volatile("cli");
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
