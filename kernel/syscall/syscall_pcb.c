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
	int i,j;
	for(i=0;i<4096;i++)
		pcb[new_id].kstack[i]=pcb[current_pid].kstack[i];
	lcr3(pcb[new_id].ucr3.val);
	printk("yes we can\n");
	for(i=0;i<NR_PDE;i++){
		if(pcb[current_pid].updir[i].present){
			int old_uptable_index=pa_to_va(pcb[current_pid].updir[i].page_frame<<PGSHIFT);
			int has_find=-1;
			for(j=0;j<3;j++){
				if(old_uptable_index==(int)pcb[current_pid].uptable[j]){
					has_find=j;
					break;
				}
			}
			if(has_find==-1){
				continue;
			}
			uint32_t physbase=get_pte();
			uint32_t temp=physbase;
			PTE* uptable_p=pcb[new_id].uptable[has_find];
			pcb[new_id].updir[i].val=va_to_pa(uptable_p)|(pcb[current_pid].updir[i].val&0xFFF);
			for(j=0;j<NR_PTE;j++){
				uptable_p->val=physbase|(pcb[current_pid].uptable[has_find][0].val&0xFFF);
				uptable_p++;
				physbase+=PGSIZE;
			}
			printk("find_one:updir_index:%d uptable_index:%d\n",i,has_find);
			printk("physbase:0x%x\n",temp);
			printk("old_physbase:0x%x\n",pcb[current_pid].uptable[has_find][0].val);
			printk("uptable_address:0x%x\n",(void*)pcb[new_id].uptable[has_find]);
			printk("old_uptable_address:0x%x\n",(void*)pcb[current_pid].uptable[has_find]);
			PDE* updir_new=&pcb[new_id].updir[i];
			PDE* updir_old=&pcb[current_pid].updir[i];
			PTE* uptable_new=(PTE*)pa_to_va(updir_new->page_frame<<PGSHIFT);
			PTE* uptable_old=(PTE*)pa_to_va(updir_old->page_frame<<PGSHIFT);
			printk("old_uptable_address:0x%x\n",(void*)uptable_old);
			printk("new_uptable_address:0x%x\n",(void*)uptable_new);
			for(j=0;j<NR_PTE;j++){
				uint32_t new_phys_addr=uptable_new->page_frame<<PGSHIFT;
				uint32_t old_phys_addr=uptable_old->page_frame<<PGSHIFT;
				/*printk("new_addr:0x%x old_addr:0x%x\n",new_phys_addr,old_phys_addr);*/
				memcpy((void*)pa_to_va(new_phys_addr),(void*)pa_to_va(old_phys_addr),PGSIZE);
				/*printk("memcpy success\n");*/
				uptable_new++;
				uptable_old++;
			}
			printk("memcpy successfully\n");
		}
	}
	pcb[new_id].status=RUNNABLE;
	printk("fork ends here\n");
	return new_id;
}
