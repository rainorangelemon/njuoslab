#include "../include/env.h"
#include "../include/mmu.h"
#include "../include/elf.h"
#include "../include/memlayout.h"
#include "../include/error.h"
#include "../include/pmap.h"
#include "../include/types.h"
#include "../include/irq.h"
#include "../include/x86.h"
#include "../include/string.h"
#include "../include/process/env.h"

struct Env ENVS[NENV];
struct Env* envs = ENVS;
struct Env* curenv = NULL;
static struct Env* env_free_list;
extern pde_t entry_pgdir[];  
pde_t *kern_pgdir = entry_pgdir;

#define ENVGENSHIFT 12

int envid2env(envid_t envid,struct Env **env_store, bool checkperm)
{
	struct Env*e;
	if (envid == 0)
	{
		*env_store = curenv;
		return 0;
	}
	e = &envs[ENVX(envid)];
	if (e->env_status == ENV_FREE || e->env_id != envid)
	{
		*env_store = 0;
		return -E_BAD_ENV;
	}
	if (checkperm && e != curenv && e->env_parent_id != curenv->env_id)
	{
		*env_store = 0;
		return -E_BAD_ENV;
	}
	*env_store = e;
	return 0;
}

void env_init(void)
{
	int i;
	for (i = NENV - 1;i >= 0;i--)
	{
		envs[i].env_status = ENV_FREE;
		envs[i].env_id = 0;
		envs[i].env_link = env_free_list;
		env_free_list = envs + i; 
	}
	return ;
}

static int env_setup_vm(struct Env* e)
{
	struct PageInfo *p = NULL;
	if (!(p = page_alloc(ALLOC_ZERO))) return -E_NO_MEM;
	p->pp_ref++;
	e->env_pgdir = (pde_t *)page2kva(p);
	memcpy(e->env_pgdir,kern_pgdir,PGSIZE);
	e->env_pgdir[PDX(UVPT)] = PADDR(e->env_pgdir) | PTE_P | PTE_U;
	return 0;
}

int env_alloc(struct Env**newenv_store,envid_t parent_id)
{
	int32_t generation;
	int r;
	struct Env *e;
	if (!(e = env_free_list)) return -E_NO_FREE_ENV;
	if ((r = env_setup_vm(e)) < 0) return r;
	generation = (e->env_id + (1 << ENVGENSHIFT))& ~(NENV - 1);
	if(generation <= 0) generation = 1 << ENVGENSHIFT;
	e->env_id = generation | (e-envs);
	e->env_parent_id = parent_id;
	e->env_type = ENV_TYPE_USER;
	e->env_status = ENV_RUNNABLE;
	e->env_runs = 0;
	
	memset(&e->env_tf,0,sizeof(e->env_tf));
	e->env_tf.ds = GD_UD | 3;
	e->env_tf.es = GD_UD | 3;
	e->env_tf.ss = GD_UD | 3;
	e->env_tf.esp = USTACKTOP;
	e->env_tf.cs = GD_UT | 3;
	e->env_tf.eflags = 0x202;

	env_free_list = e->env_link;
	*newenv_store = e;

	return 0;
}

static void region_alloc(struct Env*e, void *va, size_t len)
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
		page_insert(e->env_pgdir,pg,begin,PTE_W | PTE_U);
	}
}

unsigned char env_buffer[4096];
void readseg(unsigned char *, int, int);
static void load_icode(struct Env*e,pde_t *entry_pgdir)
{

	struct Elf *elf;
	struct Proghdr *ph,*eph;
	unsigned char pagebuffer[4096];
	
	elf = (struct Elf*)env_buffer;
	readseg((unsigned char *)elf,4096,0);
	printk("the entry of the elf2 = 0x%08x\n",elf->e_entry);

	ph = (struct Proghdr*)((char*)elf + elf->e_phoff);
	eph = ph + elf->e_phnum;
	for(;ph < eph;ph++)
	{
		uint32_t va = ph->p_va;
		int data_loaded = 0;
		if (ph->p_type == 1)
		{
			while (va < ph->p_va + ph->p_memsz)
			{
				memset(pagebuffer,0,4096);
				uint32_t offset = va & 0xfff;
				va = va & 0xfffff000;
				struct PageInfo* page = page_alloc(1);
				page_insert(entry_pgdir,page,(void*)va,PTE_U | PTE_W);
				int n = (4096 - offset) > ph->p_memsz ? ph->p_memsz : (4096 - offset);
				readseg((unsigned char*)(pagebuffer + offset),n,ph->p_offset + data_loaded);
				memcpy((void *)page2kva(page),pagebuffer,4096);
				va += 4096;
				data_loaded += n;
			
			}
		}
	}
	//lcr3(PADDR(kern_pgdir));
	e->env_pgdir = entry_pgdir;
	e->env_tf.eip = elf->e_entry;
	region_alloc(e,(void*)(USTACKTOP - PGSIZE),PGSIZE);
}

void env_create()
{
	struct Env *penv;
	env_alloc(&penv,0);
	struct PageInfo *page = page_alloc(1);
	uint32_t cr3_game = page2pa(page);
	pde_t *pgdir_game = page2kva(page);
	memcpy(pgdir_game,entry_pgdir,4096);
	load_icode(penv,pgdir_game);
	lcr3(cr3_game);
}

void env_free(struct Env* e)
{
	pte_t *pt;
	uint32_t no_pde,no_pte;
	physaddr_t pa;
	if (e == curenv) lcr3(PADDR(entry_pgdir));
	for (no_pde = 0;no_pde < PDX(UTOP);no_pde++)
	{
		if (!((e->env_pgdir[no_pde]) & PTE_P) || (entry_pgdir[no_pde] & PTE_P)) continue;
		pa = PTE_ADDR(e->env_pgdir[no_pde]);
		pt = (pte_t*)KADDR(pa);
		for (no_pte = 0;no_pte <= PTX(~0);no_pte++)
		{
			if (pt[no_pte] & PTE_P) page_remove(e->env_pgdir,PGADDR(no_pde,no_pte,0));
		}
		e->env_pgdir[no_pde] = 0;
		page_decref(pa2page(pa));
	}
	pa = PADDR(e->env_pgdir);
	e->env_pgdir = 0;
	page_decref(pa2page(pa));

	e->env_status = ENV_FREE;
	e->env_link = env_free_list;
	env_free_list = e;
}

void env_destroy(struct Env *e)
{
	env_free(e);
}


void env_pop_tf(struct TrapFrame* tf)
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

void env_run(struct Env* e)
{
	if (e == NULL)
	{
		printk("no env\n");
		while(1);
	}
	if (curenv != e)
	{
		curenv = e;
		e->env_status = ENV_RUNNING;
		e->env_runs++;
		lcr3(PADDR(e->env_pgdir));
	}
	env_pop_tf(&e->env_tf);
}

