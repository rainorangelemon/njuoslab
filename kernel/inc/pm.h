#ifndef __PCB_H__
#define __PCB_H__

#include"types.h"
#include"trap.h"
#include"memlayout.h"
#include"irq.h"

typedef int32_t pid_t;
#define LOG2NPCB	10
#define NPCB		(1<<LOG2NPCB)
#define PCBX(pid)	((pid)&(NPCB))
enum{
	FREE=0,
	DYING,
	RUNNABLE,
	RUNNING,
	NOT_RUNNABLE,
	SLEEP
};

enum PcbType{
	PCB_TYPE_USER=0
};

struct context{
	uint32_t eip;
	uint32_t esp;
	uint32_t edi;
	uint32_t esi;
	uint32_t edx;
	uint32_t ecx;
	uint32_t ebx;
	uint32_t ebp;
};

#ifndef __struct_pcb__
#define __struct_pcb__
struct PCB{
	struct TrapFrame tf;
	struct PCB*pcb_link;
	pid_t pid;
	struct context context;
	pid_t parent_id;
	enum PcbType type;
	unsigned status;
	uint32_t runs;
	uint32_t sleep_time;
	pde_t *pcb_pgdir;
};
#endif
extern struct PCB*pcbs;
extern struct PCB*current_pcb;


#endif


