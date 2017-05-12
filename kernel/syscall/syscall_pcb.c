#include "memory.h"
#include "x86.h"
#include "string.h"

int getpid(){
	return current_pid;
}

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
	int next=find_next_runnable();
	if((next!=-1)&&(next!=current_pid))
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
	int i,j,k;
	for(i=0;i<4096;i++)
		pcb[new_id].kstack[i]=pcb[current_pid].kstack[i];
	lcr3(pcb[new_id].ucr3.val);
	printk("yes we can");
	for(i=0;i<NR_PDE;i++){
		if(pcb[current_pid].updir[i].present){
			int old_uptable_index=pcb[current_pid].updir[i].page_frame<<PGSHIFT;
			int has_find=0;
			for(j=0;j<3;j++){
				if()
			}
		}
	}
	pcb[new_id].status=RUNNABLE;
	printk("fork ends here");
	return new_id;
}
