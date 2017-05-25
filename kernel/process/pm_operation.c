#include "stdio.h"
#include "pmap.h"
#include "process/pm.h"
#include "pm.h"
#include "memlayout.h"
#include "error.h"
#include "string.h"
#include "common.h"

uint32_t time_tick=0;

struct PCB*seek_next_runnable()
{
	struct PCB*pcb;
	pcb = current_pcb;
	if (current_pcb->status == RUNNING)
		current_pcb->status = RUNNABLE;
	int i;
	for (i = 0;i <= NPCB + 1;i++)
	{
		if (pcb == &pcbs[NPCB - 1]) 
			pcb = pcbs;
		else
			pcb++;
		if (pcb->status == RUNNABLE) return pcb;
	}
	printk("no process\n");
	return NULL;
}

void kernel_timer_event()
{
	time_tick++;
	int i;
	for (i = 0;i < NPCB;i++)
	{
		if (pcbs[i].status == SLEEP)
		{
			pcbs[i].sleep_time -= 1;
			if (pcbs[i].sleep_time == 0) pcbs[i].status = RUNNABLE;
		}
	}
	struct PCB* pcb = seek_next_runnable();
	pcb_run(pcb);
}

int system_pcb_fork()
{
	int i,j;
	pid_t p_id = current_pcb->pid;
	struct PCB *pcb = NULL;
	int judge = pcb_alloc(&pcb,p_id);
	if (judge != 0)
	{
		printk("pcb_alloc error!");
		return judge;
	}
	for (i = 0;i < 1024;i++)
	{
		if (!(pcb->pcb_pgdir[i] & PTE_P) && (current_pcb->pcb_pgdir[i] & PTE_P))
		{
			pte_t* pte = (pte_t*)(page2kva)((pa2page)(current_pcb->pcb_pgdir[i]));
			for (j = 0;j < 1024;j++)
			{
				if (pte[j])
				{
					struct PageInfo* p = NULL;
					if (!(p = page_alloc(1))) return -E_NO_MEM;
					p->pp_ref++;
					page_insert(pcb->pcb_pgdir,p,(void*)(i << 22 | j << 12),PTE_U | PTE_W);
					memcpy((void *)page2kva(p),(void*)page2kva(pa2page(PTE_ADDR(pte[j]))),4096);	
				}
			}
		}
	}
	pcb->tf = current_pcb->tf;
	pcb->tf.eax = 0;
	return pcb->pid;
}

void system_pcb_sleep(uint32_t time)
{
	current_pcb->status = SLEEP;
	current_pcb->sleep_time = time;
	struct PCB* pcb = seek_next_runnable();
	pcb_run(pcb);
}

void system_pcb_exit()
{
	current_pcb->status = DYING;
	pcb_destroy(current_pcb);
	struct PCB* pcb = seek_next_runnable();
	pcb_run(pcb);
}

