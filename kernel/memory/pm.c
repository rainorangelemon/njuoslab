#include "common.h"
#include "string.h"
#include "memory.h"
#include "mmu.h"
#include "x86.h"
#include "elf.h"

#define NR_PCB 16
#define SCR_KSTACK 4096

#define SECTSIZE 512
#define GAME_OFFSET_IN_DISK (10 * 1024 * 1024)

/*
typedef struct PCB {
	int PID;
	int _free_pte;
	uint32_t entry;
	CR3 ucr3;
	PDE updir[NR_PDE] align_to_page;
	PTE uptable[3][NR_PTE] align_to_page;
	uint8_t kstack[4096];
} PCB; */

bool pcb_present[NR_PCB];
static PCB pcb[NR_PCB];

PCB* create_process(uint32_t);
PDE* get_kpdir();
void mm_malloc(int,uint32_t,int);
void readseg(unsigned char*,int,int);

void init_pcb(void) {
	int i;
	extern uint8_t *vmem;
	uint32_t VMEM_ADDR = (uint32_t)vmem;
	for(i=0; i<NR_PCB; ++i) {
		pcb[i].PID = i;
		pcb[i]._free_pte = 0;
		pcb[i].entry = 0;

		/* initialize the ucr3 */
		pcb[i].ucr3.val = 0;
		pcb[i].ucr3.page_directory_base = va_to_pa(pcb[i].updir) >> 12;

		/* initialize the uPDE */
		PDE *kpdir = get_kpdir();
		memset(pcb[i].updir, 0, NR_PDE * sizeof(PDE));
		memcpy(&pcb[i].updir[KOFFSET / PTSIZE], &kpdir[KOFFSET / PTSIZE], (PHY_MEM / PTSIZE) * sizeof(PDE));
		pcb[i].updir[PDX(VMEM_ADDR)].val = kpdir[PDX(VMEM_ADDR)].val;

		/* initialize the uPTE */
		memset(pcb[i].uptable, 0, 3 * NR_PTE * sizeof(PTE));
	}
	printk("PDX(VMEM_ADDR) = 0x%x, pdir = 0x%x\n", PDX(VMEM_ADDR), pcb[0].updir[PDX(VMEM_ADDR)].val);
	memset(pcb_present, 0, sizeof(pcb_present));
}

int get_pcb() {
	int i;
	for(i=0; i<NR_PCB; ++i) if(!pcb_present[i]) {
		pcb_present[i] = true;
		return i;
	}
	return -1;
}

void free_pcb(int i) {
	assert(i>=0 && i<NR_PCB);
	pcb_present[i] = false;
}

uint32_t page_trans(int, uint32_t);

PCB* create_process(uint32_t disk_offset) {
	int pcb_idx = get_pcb();
	if(pcb_idx < 0) panic("No more available pcb!\n");
	
	/* get the ELF header */
	struct ELFHeader *elf;
	struct ProgramHeader *ph, *eph;
	uint32_t va;

	uint8_t buf[4096];
	elf = (struct ELFHeader*)buf;
	printk("addr of buf: 0x%x\n", (uint32_t)buf);

	readseg((unsigned char*)elf, 4096, GAME_OFFSET_IN_DISK);

	ph = (struct ProgramHeader*)((char *)elf + elf->phoff);
	eph = ph + elf->phnum;
	for(; ph < eph; ph ++) {
		if(ph->type != ELF_PROG_LOAD) continue;
		va = ph->vaddr;
		int pde_num = PDX(va + ph->memsz - 1) - PDX(va) + 1;
		printk("0x%x, 0x%x, 0x%x, pde_num=0x%x\n", va, ph->filesz, ph->memsz, pde_num);

		mm_malloc(pcb_idx, PDX(va), pde_num); //fill the pde and pte

		int i;
		for(i=0; i<pde_num; ++i) {
			uint32_t pdx = PDX(va) + i;
			if(PDX(va + ph->filesz - 1) >= pdx) {
				//readseg
				uint32_t start_va = pdx << PDXSHIFT;
				uint32_t end_va = (pdx + 1) << PDXSHIFT;
				if(va > start_va) start_va = va;
				if(va + ph->filesz < end_va) end_va = va + ph->filesz;
				readseg((void*)page_trans(pcb_idx, start_va), end_va - start_va, GAME_OFFSET_IN_DISK + ph->off + start_va - va);
			}
			if(PDX(va + ph->filesz) <= pdx) {
				//memset
				uint32_t start_va = pdx << PDXSHIFT;
				uint32_t end_va = (pdx + 1) << PDXSHIFT;
				if(va + ph->filesz > start_va) start_va = va + ph->filesz;
				if(va + ph->memsz < end_va) end_va = va + ph->memsz;
				memset((void*)page_trans(pcb_idx, start_va), 0, end_va - start_va);
			}
		}

		printk("\n");
		//readseg(pa, ph->p_filesz, GAME_OFFSET_IN_DISK + ph->p_offset);
		//for(i = pa + ph->p_filesz; i < pa + ph->p_memsz; *i ++ = 0);
	}
	/* set for stack */
	PCB *pcb_p = &pcb[pcb_idx];
	uint32_t physbase = get_pte();
	PDE *updir_p = &pcb_p->updir[PDX(KOFFSET - 1)];
	int *_free = &pcb_p->_free_pte;
	PTE *uptable_p = pcb_p->uptable[*_free]; (*_free) ++; assert((*_free) < 3);
	updir_p->val = va_to_pa(uptable_p) | PTE_P | PTE_W | PTE_U;
	printk("(create_process) physbase = 0x%x, updir of stack = 0x%x, uptable = 0x%x\n", physbase, updir_p, uptable_p);
	int j;
	for(j = 0; j < NR_PTE; ++ j) {
		uptable_p->val = physbase | PTE_P | PTE_W | PTE_U;
		//if(j%32 == 0) printk("uptable_p = 0x%x, val = 0x%x\n", uptable_p, uptable_p->val);
		uptable_p ++; physbase += PGSIZE;
	}

	/* for test */
	printk("(create_process, test) entry = 0x%x, pa_entry = 0x%x, code of entry = 0x%x\n", elf->entry, page_trans(pcb_idx, elf->entry), *(uint32_t*)page_trans(pcb_idx, elf->entry));
	printk("(create_process, test) va = 0x8048001, pa = 0x%x\n", page_trans(pcb_idx, 0x8048001));
	printk("(create_process, test) va = 0x8068140, pa = 0x%x\n", page_trans(pcb_idx, 0x8068140));

	printk("(create_process) ucr3=0x%x, updir=0x%x\n", pcb[pcb_idx].ucr3.val, pcb[pcb_idx].updir);
	lcr3(pcb[pcb_idx].ucr3.val);
	printk("(create_process) about to leave\n"); //while(1);
	pcb[pcb_idx].entry = elf->entry;
	return &pcb[pcb_idx];
}

void mm_malloc(int pcb_idx, uint32_t pde_idx, int pde_num) {
	printk("(mm_malloc) pcb_idx=%d, pde_idx=0x%x, pde_num=0x%x\n", pcb_idx, pde_idx, pde_num);
	Assert(pde_num <= 2, "Why so many pde? pde_num = %d\n", pde_num);

	int i,j;
	PCB *pcb_p = &pcb[pcb_idx];
	for(i = 0; i < pde_num; ++ i) {
		if(pcb_p->updir[pde_idx + i].present) {
			printk("(mm_malloc) pde_idx = 0x%x has been allocated!\n", pde_idx + i);
			continue;
		}
		uint32_t physbase = get_pte();
		PDE *updir_p = &pcb_p->updir[pde_idx + i];
		int *_free = &pcb_p->_free_pte;
		PTE *uptable_p = pcb_p->uptable[*_free]; (*_free) ++; assert((*_free) < 3);

		updir_p->val = va_to_pa(uptable_p) | PTE_P | PTE_W | PTE_U;
		for(j = 0; j < NR_PTE; ++ j) {
			uptable_p->val = physbase | PTE_P | PTE_W | PTE_U;
			uptable_p ++; physbase += PGSIZE;
		}
	}
}

uint32_t page_trans(int pcb_idx, uint32_t va) {
	PDE *updir_p = &pcb[pcb_idx].updir[PDX(va)];
	PTE *uptable_p = (PTE*) ((updir_p->page_frame << PGSHIFT) + 4 * PTX(va));
	printk("(page_trans) updir = 0x%x, uptable = 0x%x\n");
	return (uptable_p->page_frame << PGSHIFT) + PGOFF(va);
}

void waitdisk(void) {
	while((inb(0x1f7) & 0xc0) != 0x40);
}

void readsect(void *dst, int offset) {
	/* int i; */
	waitdisk();

	outb(0x1f2, 1);		// count = 1
	outb(0x1f3, offset);
	outb(0x1f4, offset >> 8);
	outb(0x1f5, offset >> 16);
	outb(0x1f6, (offset >> 24) | 0xe0);
	outb(0x1f7, 0x20);	// cmd 0x20 - read sectors

	waitdisk();

	insl(0x1f0, dst, SECTSIZE/4);	//read a sector
	/*	this part does the same thing
	for(i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = in_long(0x1f0);
	} */
}

void readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
		readsect(pa, offset);
}
