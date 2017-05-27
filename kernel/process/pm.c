#include "pm.h"
#include "mmu.h"
#include "elf.h"
#include "memlayout.h"
#include "error.h"
#include "pmap.h"
#include "types.h"
#include "irq.h"
#include "x86.h"
#include "string.h"
#include "process/pm.h"

struct PCB PCBS[NPCB];
struct PCB* pcbs = PCBS;
struct PCB* current_pcb = NULL;
static struct PCB* pcb_free_list;
extern pde_t entry_pgdir[];  
pde_t *kern_pgdir = entry_pgdir;

#define PCBGENSHIFT 12

int pid2pcb(pid_t pid,struct PCB **pcb_store, bool checkperm)
{
	struct PCB*e;
	if (pid == 0)
	{
		*pcb_store = current_pcb;
		return 0;
	}
	e = &pcbs[PCBX(pid)];
	if (e->status == FREE || e->pid != pid)
	{
		*pcb_store = 0;
		return -E_BAD_PCB;
	}
	if (checkperm && e != current_pcb && e->parent_id != current_pcb->pid)
	{
		*pcb_store = 0;
		return -E_BAD_PCB;
	}
	*pcb_store = e;
	return 0;
}

void pcb_init(void)
{
	int i;
	for (i = NPCB - 1;i >= 0;i--)
	{
		pcbs[i].status = FREE;
		pcbs[i].pid = 0;
		pcbs[i].pcb_link = pcb_free_list;
		pcb_free_list = pcbs + i; 
	}
	return ;
}

static int pcb_setup_vm(struct PCB* e)
{
	struct PageInfo *p = NULL;
	if (!(p = page_alloc(ALLOC_ZERO))) return -E_NO_MEM;
	p->pp_ref++;
	e->pcb_pgdir = (pde_t *)page2kva(p);
	memcpy(e->pcb_pgdir,kern_pgdir,PGSIZE);
	e->pcb_pgdir[PDX(UVPT)] = PADDR(e->pcb_pgdir) | PTE_P | PTE_U;
	return 0;
}

int pcb_alloc(struct PCB**newpcb_store,pid_t parent_id)
{
	int32_t generation;
	int r;
	struct PCB *e;
	if (!(e = pcb_free_list)) return -E_NO_FREE_PCB;
	if ((r = pcb_setup_vm(e)) < 0) return r;
	generation = (e->pid + (1 << PCBGENSHIFT))& ~(NPCB - 1);
	if(generation <= 0) generation = 1 << PCBGENSHIFT;
	e->pid = generation | (e-pcbs);
	e->parent_id = parent_id;
	e->type = PCB_TYPE_USER;
	e->status = RUNNABLE;
	e->runs = 0;
	
	memset(&e->tf,0,sizeof(e->tf));
	e->tf.ds = GD_UD | 3;
	e->tf.es = GD_UD | 3;
	e->tf.ss = GD_UD | 3;
	e->tf.esp = USTACKTOP;
	e->tf.cs = GD_UT | 3;
	e->tf.eflags = 0x202;

	pcb_free_list = e->pcb_link;
	*newpcb_store = e;

	return 0;
}

static void region_alloc(struct PCB*e, void *va, size_t len)
{
	void *begin = ROUNDDOWN(va,PGSIZE);
	void *end = ROUNDUP(va + len,PGSIZE);
	for(;begin < end;begin += PGSIZE)
	{
		struct PageInfo *pg = page_alloc(0);
		if (!pg)
		{ 
			printk("region alloc failed!");
			return;
		}
		page_insert(e->pcb_pgdir,pg,begin,PTE_W | PTE_U);
	}
}

unsigned char pcb_buffer[4096];
void readseg(unsigned char *, int, int);
static void load_icode(struct PCB*e,pde_t *entry_pgdir)
{

	struct ELFHeader *elf;
	struct ProgramHeader *ph,*eph;
	unsigned char pagebuffer[4096];
	
	elf = (struct ELFHeader*)pcb_buffer;
	readseg((unsigned char *)elf,4096,0);
	printk("the entry of the elf2 = 0x%08x\n",elf->entry);

	ph = (struct ProgramHeader*)((char*)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(;ph < eph;ph++)
	{
		uint32_t va = ph->va;
		int data_loaded = 0;
		if (ph->type == 1)
		{
			while (va < ph->va + ph->memsz)
			{
				memset(pagebuffer,0,4096);
				uint32_t offset = va & 0xfff;
				va = va & 0xfffff000;
				struct PageInfo* page = page_alloc(1);
				page_insert(entry_pgdir,page,(void*)va,PTE_U | PTE_W);
				int n = (4096 - offset) > ph->memsz ? ph->memsz : (4096 - offset);
				readseg((unsigned char*)(pagebuffer + offset),n,ph->offset + data_loaded);
				memcpy((void *)page2kva(page),pagebuffer,4096);
				va += 4096;
				data_loaded += n;
			
			}
		}
	}
	//lcr3(PADDR(kern_pgdir));
	e->pcb_pgdir = entry_pgdir;
	e->tf.eip = elf->entry;
	region_alloc(e,(void*)(USTACKTOP - PGSIZE),PGSIZE);
}

void pcb_create()
{
	struct PCB *ppcb;
	pcb_alloc(&ppcb,0);
	struct PageInfo *page = page_alloc(1);
	uint32_t cr3_game = page2pa(page);
	pde_t *pgdir_game = page2kva(page);
	memcpy(pgdir_game,entry_pgdir,4096);
	load_icode(ppcb,pgdir_game);
	lcr3(cr3_game);
}

void pcb_free(struct PCB* e)
{
	pte_t *pt;
	uint32_t no_pde,no_pte;
	physaddr_t pa;
	if (e == current_pcb) lcr3(PADDR(entry_pgdir));
	for (no_pde = 0;no_pde < PDX(UTOP);no_pde++)
	{
		if (!((e->pcb_pgdir[no_pde]) & PTE_P) || (entry_pgdir[no_pde] & PTE_P)) continue;
		pa = PTE_ADDR(e->pcb_pgdir[no_pde]);
		pt = (pte_t*)KADDR(pa);
		for (no_pte = 0;no_pte <= PTX(~0);no_pte++)
		{
			if (pt[no_pte] & PTE_P) page_remove(e->pcb_pgdir,PGADDR(no_pde,no_pte,0));
		}
		e->pcb_pgdir[no_pde] = 0;
		page_decref(pa2page(pa));
	}
	pa = PADDR(e->pcb_pgdir);
	e->pcb_pgdir = 0;
	page_decref(pa2page(pa));

	e->status = FREE;
	e->pcb_link = pcb_free_list;
	pcb_free_list = e;
}

void pcb_destroy(struct PCB *e)
{
	pcb_free(e);
}


void pcb_pop_tf(struct TrapFrame* tf)
{
	 asm volatile(
		"movl %0,%%esp\n"
		"\tpopal\n"
		"\tpopl %%es\n"
		"\tpopl %%ds\n"
		"\taddl $0x8,%%esp\n"
		"\tiret"
		::"g"(tf):"memory");
	printk("iret failed\n"); 
}

void pcb_run(struct PCB* e)
{
	if (e == NULL)
	{
		printk("no pcb runnable\n");
		return;
	}
	if (current_pcb != e)
	{
		current_pcb = e;
		e->status = RUNNING;
		e->runs++;
		lcr3(PADDR(e->pcb_pgdir));
	}
	pcb_pop_tf(&e->tf);
}

