#include "memory.h"
#include "x86.h"
#include "string.h"

int find_next_runnable(){
	int now=current_pid;
	if(pcb[now].status==RUNNING)
		pcb[now].status=RUNNABLE;
	int i;
	for(i=0;i<NR_PCB;i++){
		if(i==now)
			continue;
		else if(pcb[i].status==RUNNABLE)
			return i;
	}
	return now;
	printk("no process\n");
	return -1;
}

void schedule(){
	int i;
	for(i=0;i<NR_PCB;i++){
		if(pcb[i].status==SLEEP){
			pcb[i].sleep_time-=1;
			if(pcb[i].sleep_time==0)
				pcb[i].status=RUNNABLE;
		}
	}
	run_process(find_next_runnable());
}



void system_sleep(uint32_t time){
	pcb[current_pid].status=SLEEP;
	pcb[current_pid].sleep_time=time;
	int new_pid=find_next_runnable();
	run_process(new_pid);
}

void system_exit(){
	pcb[current_pid].status=FREE;
	destroy_process(current_pid);
	int new_pid=find_next_runnable();
	run_process(new_pid);
}

int system_fork(){
	int new_id=get_pcb();
	pcb[new_id].entry=pcb[current_pid].entry;
	pcb[new_id]._free_pte=pcb[current_pid]._free_pte;
	int i;
	for(i=0;i<4096;i++)
		pcb[new_id].kstack[i]=pcb[current_pid].kstack[i];
	lcr3(pcb[new_id].ucr3.val);
	for(i=0;i<NR_PDE;i++){
		if(pcb[current_pid].updir[i].present){
			PTE *old_uptable_p;
			old_uptable_p=(void*)pa_to_va(((pcb[current_pid].updir[i].val)>>12)<<12);	
			int index;
			for(index=0;index<3;index++){
				if(old_uptable_p==pcb[current_pid].uptable[index])
					break;
			}
			PTE *uptable_p=pcb[new_id].uptable[index];
			int physbase=get_pte();
			pcb[new_id].updir[i].val=va_to_pa(uptable_p)|PTE_P|PTE_W|PTE_U;
			int k;
			for(k=0;k<NR_PTE;k++){
				uptable_p->val=physbase|PTE_P|PTE_W|PTE_U;
				memcpy((void*)physbase,(void*)(((old_uptable_p->val)>>12)<<12),PGSIZE);
				old_uptable_p++;
				uptable_p++;
				physbase=physbase+PGSIZE;
			}
		}
	}
	pcb[new_id].status=RUNNABLE;
	return new_id;
}
