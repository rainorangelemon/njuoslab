#include "types.h"
#include "mmu.h"
#include "pcb.h"
#include "pcb_struct.h"
#include "string.h"
#include "cpupa.h"
#define MAX_PCB 100

#define new 	0
#define ready	1
#define running	2
#define blocked	3
#define exit 	4

#define sleeptime 5

int printk(const char *fmt, ...);
extern struct PCB *current;
void set_tss_esp0(uint32_t);
PDE* get_updir();
void release();
void *memset(void *v, int c, size_t n);

struct PCB pcb_table[MAX_PCB];
int pid_count = 0;			//当前进程号分配到了第几个

int pid_alloc()
{
	pid_count++;
	return pid_count;
}

void pcb_init()		//将所有可用PCB资源初始化
{
	int i;
	for(i = 0; i < MAX_PCB; i ++)
	{
		pcb_table[i].status = -1;
		pcb_table[i].pid = -1;
		pcb_table[i].block_sem_index = -1;
	}
}

int pcb_alloc()	//为进程分配一个PCB，返回pid
{
	int pcb_index;
	for(int i = 0; i < MAX_PCB; i ++)
	{
		if(pcb_table[i].status == -1)
		{
			pcb_index = i;
			break;
		}
	}
	return pcb_index;		//告诉进程我给他分配的pcb的位置
}

void pcb_new(int pid, int ppid, int pcb_index)
{
	pcb_table[pcb_index].pid = pid;
	pcb_table[pcb_index].ppid = ppid;
	pcb_table[pcb_index].status = new;
	pcb_table[pcb_index].time_count = 0;
	pcb_table[pcb_index].sleep_time = sleeptime;
	pcb_table[pcb_index].pcb_index = pcb_index;
}

void pcb_cr3write(int pcb_index, uint32_t val)
{
	pcb_table[pcb_index].cr3 = val;
}

void pcb_ready(int pcb_index)
{
		pcb_table[pcb_index].status = ready;
		pcb_table[pcb_index].time_count = 0;
}

void pcb_running(int pcb_index)
{
		pcb_table[pcb_index].status = running;
		pcb_table[pcb_index].time_count += 1;
}

void schedule()
{
	int i;
	for(i = current->pcb_index + 1; i % MAX_PCB != current->pcb_index; i=(i+1)%MAX_PCB)
	{
		if(pcb_table[i].status == ready)
		{
			current = &pcb_table[i];
			current->status = running;
			set_tss_esp0((uint32_t)(pcb_table[i].kstack + 4096));
			write_cr3(pcb_table[i].cr3);
			return;
		}
	}
	current = &pcb_table[0];
}

void wakeup()
{
	for(int i = 0; i < MAX_PCB; i ++)
	{
		if(pcb_table[i].status == blocked)
		{
			pcb_table[i].sleep_time --;
			if(pcb_table[i].sleep_time == 0)
				pcb_table[i].status = ready;
			//printk("proc pid == %d, sleep time == %d \n",pcb_table[i].pid, pcb_table[i].sleep_time);
		}
	}
}

void sleep(int time)
{
	current->status = blocked;
	current->sleep_time = time;
	schedule();
	return;
}

void Exit()
{
	current->status = -1;
	current->pid = -1;
	release();
	printk("proc %d exit\n", current->pid);
	schedule();
}

int getpid()	//获取当前进程的PID
{
	return current->pid;
}

void fork()
{
	current->tf->eax = current->pid;
	current->status = ready;
			
	int pcb_index = pcb_alloc();	//new pcb
	int pid = -1; //规定fork出来的子进程的pid==-1;
	
	pcb_new(pid, getpid(), pcb_index);
	pcb_ready(pcb_index);
	copy(pcb_index);
	
	//复制陷阱帧
	memcpy(pcb_table[pcb_index].kstack, current->kstack, KSTACK_SIZE);
	
	//修改tf指针指向该PCB的内核栈
	struct TrapFrame *tf = (struct TrapFrame *)(pcb_table[pcb_index].kstack + 4096 - sizeof(struct TrapFrame));
	pcb_table[pcb_index].tf = tf;

	tf->eax = pid;
	
	schedule();
}

void create_thread(uint32_t *func_addr)
{
	int pcb_index = pcb_alloc();
	int pid = -2;		//线程

	pcb_new(pid, getpid(), pcb_index);
	pcb_ready(pcb_index);
	thread_copy(pcb_index);
	//设置陷阱帧
	struct TrapFrame *tf = (struct TrapFrame *)(pcb_table[pcb_index].kstack + 4096 - sizeof(struct TrapFrame));
	tf->ss = current->tf->ss;
	tf->esp = 0xC0000000 - 4;
	tf->eflags = 0x202;
	tf->cs = current->tf->cs;
	tf->eip = (uint32_t)func_addr;
	tf->ds = current->tf->ds;
	tf->es = current->tf->es;
	pcb_table[pcb_index].tf = tf;
	
}

void release_one_blocked(int sem_index)
{
	for(int i = 0; i < MAX_PCB; i ++)
	{
		if(pcb_table[i].block_sem_index == sem_index)
		{
			pcb_table[i].block_sem_index = -1;
			pcb_table[i].status = ready;
			pcb_table[i].time_count = 0;
			break;
		}
	}
}
