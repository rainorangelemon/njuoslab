#include <elf.h>			//使用标准结构体的定义
#include "include/elf.h"	//定义的in_byte 等函数
#include "include/types.h"
#include "include/cpupa.h"
#include "include/cpu.h"
#include "include/memory.h"
#include "include/mmu.h"
#include "include/pmap.h"
#include "include/pcb.h"
#include "include/fcb.h"
#include "pcb_struct.h"
#include "include/x86.h"

#define STACK_SIZE (4 * (1 << 20))
uint32_t get_ucr3();
#define SECTSIZE 512

#define NR_PDE 1024
#define ELF_OFFSET_IN_DISK 100*(1<<10)

//用户页目录表，在其他地方定义的
extern PDE updir[NR_PDE];

void readseg(unsigned char *, int, int);
void readsect(void *dst, int offset);
uint32_t mm_malloc(uint32_t, int);
void *memset(void *v, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t len);
void page_init(void);

int printk(const char *fmt, ...);
//void create_video_mapping();

void create_video_mapping();

#define MAX_PCB 100
extern struct PCB pcb_table[MAX_PCB];
extern struct PCB *current;
//-------------------------------------
extern struct map bitmap;
extern struct dir direct;

void loader()
{
	readseg((uint8_t *)&bitmap, 256*512, ELF_OFFSET_IN_DISK);
	for(int k = 0; k < 80; k ++)
		printk("%02x", bitmap.mask[k].byte);
	printk("\n");

	readseg((uint8_t *)&direct, 512, ELF_OFFSET_IN_DISK+sizeof(bitmap));

	int pcb_index = pcb_alloc();  
	int pid = pid_alloc();      

	printk("game's pcb_index = %d 	game's pid = %d\n",pcb_index, pid);
	pcb_new(pid, 0, pcb_index);  

/*********************************************************************/
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph, *eph;
	uint8_t buffer[4096];
	uint8_t sec_buf[4096];

	elf = (Elf32_Ehdr*)buffer;
	
	struct iNode inode;
	printk("%s\n", direct.entry[0].filename);
	printk("%s\n", direct.entry[1].filename);
	int inode_offset = direct.entry[0].inode_offset; 
	readseg((uint8_t*)&inode, 512, ELF_OFFSET_IN_DISK + inode_offset * 512);
	int elf_offset = inode.data_block_offset[0];
	printk("elf_offset:0x%x\n",elf_offset);
	readseg((uint8_t *)buffer, 4096, ELF_OFFSET_IN_DISK + elf_offset * 512);
	
	ph = (Elf32_Phdr*)((uint8_t*)elf + elf->e_phoff);
	eph = ph + elf->e_phnum;

	page_init();
	for(; ph < eph; ph++)
	{
		if(ph->p_type == PT_LOAD) {
			uint32_t va = ph->p_vaddr;
			int num = 0;				//已经加载的总字节数
			for(; va < ph->p_vaddr + ph->p_memsz; va +=PGSIZE)
			{
				int off = PGOFF(va);	//页内偏移量
				va = PTE_ADDR(va);		//页  首地址
				uint32_t addr = mm_malloc(va, PGSIZE);//按页首地址对齐分配物理页
				memset(sec_buf, 0, PGSIZE);		//初始化磁盘缓冲区
				int rest = PGSIZE - off;		//按页对齐划分，实际占用的页大小
				if(ph->p_filesz - num < rest)	//剩余可加载的内容不足时
					rest = ph->p_filesz - num;
				if(rest != 0)
				//	readseg((void*)(sec_buf + off), rest, 200*512 + ph->p_offset + num);
				readseg((void *)(sec_buf + off), rest, ELF_OFFSET_IN_DISK + elf_offset * 512 + ph->p_offset + num);
				memcpy((void*)addr, sec_buf, PGSIZE);
				num += rest;
			}
		}
	}
/**********************************************/
//用户正常执行的栈
	for(int i = KOFFSET - STACK_SIZE; i < KOFFSET; i += PGSIZE)
		mm_malloc(i, PGSIZE);

//做显存映射
	create_video_mapping();

/**********************************************/
//切换cr3为用户页目录表的首地址
	PDE *pdir = (PDE*)PADDR(updir);
	CR3 cr3;
	cr3.val = 0;
	cr3.page_directory_base = ((uint32_t)pdir) >> 12;
	pcb_cr3write(pcb_index, cr3.val);
//	write_cr3(pcb_table[pcb_index].cr3);
/**********************************************/

//设置TrapFrame
	uint32_t eip = elf->e_entry;
	uint32_t cs = SELECTOR_USER(SEG_USER_CODE);
	uint32_t ss = SELECTOR_USER(SEG_USER_DATA);
	uint32_t ds = ss;
	uint32_t es = ss;

	struct TrapFrame *tf = (struct TrapFrame*)(pcb_table[pcb_index].kstack + 4096 - sizeof(struct TrapFrame));
	tf->ss = ss;
	tf->esp = KOFFSET - 4;
	tf->eflags = 0x202;
	tf->cs = cs;
	tf->eip = eip;
	tf->ds = ds;
	tf->es = es;
	pcb_table[pcb_index].tf = tf;
	pcb_ready(pcb_index);
	
	return;
}

void
waitdisk(void) {
	while((in_byte(0x1F7) & 0xC0) != 0X40); /* 等待磁盘完毕 */
}

/* 读磁盘的一个扇区 */
void
readsect(void *dst, int offset) {
	int i;
	waitdisk();
	out_byte(0x1F2, 1);
	out_byte(0x1F3, offset);
	out_byte(0x1F4, offset >> 8);
	out_byte(0x1F5, offset >> 16);
	out_byte(0x1F6, (offset >> 24) | 0xE0);
	out_byte(0x1F7, 0x20);

	waitdisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = in_long(0x1F0);
	}
}

void
writesect(void *src, int offset) {
	int i;
	waitdisk();
	out_byte(0x1F2, 1);
	out_byte(0x1F3, offset);
	out_byte(0x1F4, offset >> 8);
	out_byte(0x1F5, offset >> 16);
	out_byte(0x1F6, (offset >> 24) | 0xE0);
	out_byte(0x1F7, 0x30);

	waitdisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		out_long(0x1F0, ((int *)src)[i]);
	}
}

/* 将位于磁盘offset位置的count字节数据读入物理地址pa */
void
readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
	{
		readsect(pa, offset);
	}
}

void
writeseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
	pa -= offset % SECTSIZE;
	offset = (offset / SECTSIZE) + 1;
	for(; pa < epa; pa += SECTSIZE, offset ++)
	{
		writesect(pa, offset);
	}
}
