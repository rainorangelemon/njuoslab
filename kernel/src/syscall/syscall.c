#include "include/mmu.h"
#include "include/irq.h"
#include "include/video.h"
#include "include/x86.h"
#include "include/pcb.h"
#include "pcb_struct.h"
#include "include/semaphore.h"

//#include "include/types.h"

void serial_printc(char ch);
void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *v, int c, size_t n);
int handle_keys();

#define KEYCODE 0
#define VMEMORY 1
#define SERIAL	2
#define FORK 	3
#define SLEEP 	4
#define EXIT 	5
#define THREAD	6
#define SEM_OPEN	7
#define SEM_WAIT	8
#define SEM_POST	9
#define SEM_CLOSE	10
#define Write       16
#define Time        17
#define Getpid      18

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

extern uint32_t code;
extern uint32_t time_tick;
extern struct PCB* current;
void do_syscall(struct TrapFrame *tf) {
	switch(tf->eax) {
		case KEYCODE:
			tf->eax=handle_keys();
			break;
		case Write:
			tf->eax = fs_write(tf->ebx,(void*)tf->ecx,tf->edx);
			break;
		case VMEMORY: //绘制显存
			memcpy(VMEM_ADDR, (void *)tf->ebx, SCR_SIZE);
			break;
		case SERIAL: //串口输出
			serial_printc((char)tf->ebx);
			break;
		case Time:
			tf->eax=time_tick;
			break;
		case FORK:	//fork			
	//		tf->eax = fork();	
			fork();
			break;
		case SLEEP: //sleep
			sleep(tf->ebx);
			break;
		case EXIT:
			Exit();
			break;
		case Getpid:
			tf->eax=current->pid;
			break;
		case THREAD:
			create_thread((uint32_t *)tf->ebx);
			break;
		case SEM_OPEN:
			sem_open((sem_t *)tf->ebx, (int)tf->ecx, (bool)tf->edx);
			break;
		case SEM_WAIT:
			sem_wait((sem_t *)tf->ebx);
			break;
		case SEM_POST:
			sem_post((sem_t *)tf->ebx);
			break;
		case SEM_CLOSE:
			sem_close((sem_t *)tf->ebx);
			break;

		default: printk("Unhandled system call: id = %d", tf->eax);
	}
}
