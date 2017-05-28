#ifndef PCB_STRUCT_H
#define PCB_STRUCT_H

#include "./mmu.h"

#define KSTACK_SIZE 4096
struct PCB { 
	struct TrapFrame *tf;
	uint8_t kstack[KSTACK_SIZE];

	int pid;                //Process id
	int ppid;               //Parent process id
	int status;              //Process state
	int time_count;         //the running times of Process 
	int sleep_time;         //Sleep count down
	uint32_t cr3;           //PDT base address
	int pcb_index;
	int block_sem_index;	//阻塞在那个信号量上,初始值为-1
};

#endif
