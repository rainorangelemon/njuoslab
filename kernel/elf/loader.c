#include "elf.h"
#include "mmu.h"
#include "x86.h"
#include "types.h"
#include "memlayout.h"
#include "stdio.h"
#include "common.h"
#include "string.h"
#include "pmap.h"
#include "irq.h"
#include "trap.h"

#define SECTSIZE 512

void readseg(unsigned char *, int, int);
void waitdisk(void);
void readsect(void *, int);
extern struct PageInfo* page_free_list; 
unsigned char buffer[4096];

void* loader(pde_t *entry_pgdir)
{
	struct ELFHeader *elf;
	struct ProgramHeader *ph, *eph;
	unsigned char pagebuffer[4096];

	elf = (struct ELFHeader*)buffer;
	readseg((unsigned char*)elf,4096,0);

	printk("the entry of elf = 0x%08x\n",elf->entry);

	ph = (struct ProgramHeader*)((char*)elf + elf->phoff);
	eph = ph + elf->phnum;
	for (;ph < eph;ph++)
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
				struct PageInfo *page = page_alloc(1);
				page_insert(entry_pgdir,page,(void *)va,PTE_U | PTE_W);
				int n = (4096 - offset) > ph->memsz ? ph->memsz : (4096 - offset);
				readseg((unsigned char*)(pagebuffer + offset),n,ph->offset + data_loaded);
				memcpy((void *)page2kva(page),pagebuffer,4096);
				va += 4096;
				data_loaded += n;
			}	
		}
	}
	return (void*)elf->entry;

}


