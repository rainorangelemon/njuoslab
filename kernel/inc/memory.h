#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"
#include "mmu.h"
#include "x86/memory.h"
#include "x86/x86.h"


#define align_to_page __attribute((aligned(PGSIZE)))

#define KOFFSET 0xC0000000
#define PHY_MEM 0x8000000
#define KERNEL_SIZE 0x1000000
#define USER_SIZE 0x100000
#define KSTACK_SIZE 4096

#define va_to_pa(x) (((unsigned)x)-KOFFSET)
#define pa_to_va(x) (((unsigned)x)+KOFFSET)

typedef union PageDirectoryEntry{
	struct{
		uint32_t present :1;
		uint32_t read_write:1;
		uint32_t user_supervisor:1;
		uint32_t page_write_through:1;
		uint32_t page_cache_disable:1;
		uint32_t accessed:1;
		uint32_t pad0:6;
		uint32_t page_frame:20;
	};
	uint32_t val;
}PDE;

typedef union PageTableEntry{
	struct{
		uint32_t present:1;
		uint32_t read_write:1;
		uint32_t user_supervisor:1;
		uint32_t page_write_through:1;
		uint32_t page_cache_disable:1;
		uint32_t accessed:1;
		uint32_t dirty:1;
		uint32_t pad0:1;
		uint32_t global:1;
		uint32_t pad1:3;
		uint32_t page_frame:20;
	};
	uint32_t val;
}PTE;

typedef PTE (*PT) [NR_PTE];

typedef union CR0{
	struct{
		uint32_t protect_enable:1;
		uint32_t monitor_coprocessor:1;
		uint32_t emulation:1;
		uint32_t task_switched:1;
		uint32_t extension_type:1;
		uint32_t numeric_error:1;
		uint32_t pad0:10;
		uint32_t write_protect:1;
		uint32_t pad1:1;
		uint32_t alignment_mask:1;
		uint32_t pad2:10;
		uint32_t no_write_through:1;
		uint32_t cache_disable:1;
		uint32_t paging:1;
	};
	uint32_t val;
}CR0;

typedef union CR3{
	struct{
		uint32_t pad0:3;
		uint32_t page_write_through:1;
		uint32_t page_cache_disable:1;
		uint32_t pad1:7;
		uint32_t page_directory_base:20;
	};
	uint32_t val;
}CR3;

enum{
	FREE=0,
	RUNNABLE,
	RUNNING,
	SLEEP
};

typedef struct PCB{
	int PID;
	int _free_pte;
	uint32_t entry;
	CR3 ucr3;
	PDE updir[NR_PDE] align_to_page;
	PTE uptable[3][NR_PTE] align_to_page;
	uint8_t kstack[4096];
	unsigned int status;
	uint32_t sleep_time;
}PCB;

#define NR_PCB 16
#define SCR_KSTACK 4096

#define SECTSIZE 512
#define GAME_OFFSET_IN_DISK (10 * 1024 * 1024)

bool pcb_present[NR_PCB];
PCB pcb[NR_PCB];

int current_pid;

void readseg(unsigned char*,int,int);
void init_pte_info();
uint32_t get_pte();
void free_pte(int);
void free_address(int);
uint32_t page_trans(int,uint32_t);

int get_pcb();
void init_pcb();
PCB* create_process(uint32_t);
void destroy_process(int pid);
void pop_tf_process(TrapFrame4p *tf);
void run_process(int);
void schedule();


#endif
