#include "common.h"
#include "mmu.h"
#include "x86.h"
#include "memory.h"
#include "string.h"
#include "irq.h"
#include "x86/memory.h"

static PDE kpdir[NR_PDE] align_to_page;
static PTE kptable[PHY_MEM / PGSIZE] align_to_page;

PDE* get_kpdir() { return kpdir; }

/* set up ptables for kernel */
void init_page(void) {
	CR0 cr0;
	CR3 cr3;
	PDE *pdir = (PDE *)va_to_pa(kpdir);
	PTE *ptable = (PTE *)va_to_pa(kptable);
	uint32_t pdir_idx;
	
	/* let all PDEs invalid */
	memset(pdir, 0, NR_PDE * sizeof(PDE));

	/* Fill all the pde */
	for(pdir_idx = 0; pdir_idx < PHY_MEM / PTSIZE; pdir_idx ++) {
		pdir[pdir_idx].val = PTE_ADDR(ptable) | PTE_P | PTE_W;
		pdir[pdir_idx + KOFFSET / PTSIZE].val = PTE_ADDR(ptable) | PTE_P | PTE_W;
		ptable += NR_PTE;
	}

	/* fill PTEs */

	asm volatile ("std;\
			1: stosl;\
			   subl %0, %%eax;\
			   jge 1b" : :
			   "i"(PGSIZE), "a"((PHY_MEM - PGSIZE) | PTE_P | PTE_W), "D"(ptable - 1));

	/* cr3 is the entry of page directory */
	cr3.val = 0;
	cr3.page_directory_base = ((uint32_t)pdir) >> 12;
	lcr3(cr3.val);

	/* set PG bit in CR0 to enable paging */
	cr0.val = rcr0();
	cr0.paging = 1;
	lcr0(cr0.val);

}


static Taskstate tss;

static void set_tss(Segdesc *ptr) {
	tss.ts_ss0 = SELECTOR_KERNEL(SEG_KERNEL_DATA);
	uint32_t base = (uint32_t)&tss;
	uint32_t limit = sizeof(Taskstate) - 1;
	ptr->sd_lim_15_0 = limit & 0xffff;
	ptr->sd_base_15_0 = base & 0xffff;
	ptr->sd_base_23_16 = (base >> 16) & 0xff;
	ptr->sd_type = STS_T32A;
	ptr->sd_s = 0;
	ptr->sd_dpl = DPL_USER;
	ptr->sd_p = 1;
	ptr->sd_lim_19_16 = limit >> 16;
	ptr->sd_avl = 0;
	ptr->sd_rsv1 = 0;
	ptr->sd_db = 1;
	ptr->sd_g = 0;
	ptr->sd_base_31_24 = base >> 24;
}

static Segdesc gdt[NR_SEGMENTS];

static void set_segment(Segdesc *ptr, uint32_t pl, uint32_t type) {
	ptr->sd_lim_15_0  = 0xFFFF;
	ptr->sd_base_15_0   = 0x0;
	ptr->sd_base_23_16  = 0x0;
	ptr->sd_type = type;
	ptr->sd_s = 1;
	ptr->sd_dpl = pl;
	ptr->sd_p = 1;
	ptr->sd_lim_19_16 = 0xF;
	ptr->sd_avl = 0;
	ptr->sd_rsv1 = 0;
	ptr->sd_db = 1;
	ptr->sd_g = 1;
	ptr->sd_base_31_24  = 0x0;
}


static void write_gdtr(void *addr, uint32_t size) {
	static volatile uint16_t data[3];
	data[0] = size - 1;
	data[1] = (uint32_t)addr;
	data[2] = ((uint32_t)addr) >> 16;
	lgdt((void*)data);
}

void init_segment(void) {
	memset(gdt, 0, sizeof(gdt));
	set_segment(&gdt[SEG_KERNEL_CODE], DPL_KERNEL, STA_X | STA_R);
	set_segment(&gdt[SEG_KERNEL_DATA], DPL_KERNEL, STA_W);
	set_segment(&gdt[SEG_USER_CODE], DPL_USER, STA_X | STA_R);
	set_segment(&gdt[SEG_USER_DATA], DPL_USER, STA_W);
	set_tss(&gdt[SEG_TSS]);

	write_gdtr(gdt, sizeof(gdt));
	ltr( SELECTOR_USER(SEG_TSS) );
}

void write_tss_esp0(uint32_t esp0) {
	tss.ts_esp0 = esp0;
}

void set_trapframe(struct TrapFrame4p *tf, uint32_t entry) {
	tss.ts_esp0 = ((uint32_t) tf) + KSTACK_SIZE;
	printk("\nsizeof TrapFrame4p is: 0x%x\n", sizeof(struct TrapFrame4p));
	printk("ss0 = 0x%x,  esp0 = 0x%x\n\n", tss.ts_ss0, tss.ts_esp0);

	tf->ebp = 0;
	tf->eax = 1;
	tf->ebx = 2;
	tf->ecx = 3;
	tf->edx = 4;
	tf->gs = SELECTOR_USER(SEG_USER_DATA);
	tf->fs = SELECTOR_USER(SEG_USER_DATA);
	tf->es = SELECTOR_USER(SEG_USER_DATA);
	tf->ds = SELECTOR_USER(SEG_USER_DATA);
	tf->irq = 0;
	tf->err = 0;
	tf->eip = entry;
	tf->cs = SELECTOR_USER(SEG_USER_CODE);
	tf->eflags = 0x2 | FL_IF;
	tf->esp = KOFFSET - 16;
	tf->ss = SELECTOR_USER(SEG_USER_DATA);
	
	printk("The TrapFrame we create:\n");
	printk("%x\t%x\t%x\t%x\n", tf->edi, tf->esi, tf->ebp, tf->xxx);
	printk("%x\t%x\t%x\t%x\n", tf->ebx, tf->edx, tf->ecx, tf->eax);
	printk("%x\t%x\t%x\t%x\n", tf->gs, tf->fs, tf->es, tf->ds);
	printk("irq = %d,  err = %d\n", tf->irq, tf->err);
	printk("eip = 0x%x,  cs = 0x%x\n", tf->eip, tf->cs);
	printk("eflags = 0x%x\n", tf->eflags);
	printk("esp = 0x%x,  ss = 0x%x\n", tf->esp, tf->ss);
}
