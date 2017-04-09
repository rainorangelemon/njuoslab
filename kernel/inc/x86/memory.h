#ifndef __X86_MEMORY_H__
#define __X86_MEMORY_H__

struct TrapFrame {
	uint32_t edi, esi, ebp, xxx, ebx, edx, ecx, eax;
	int32_t irq;
	uint32_t err,eip,cs,eflags;
}TrapFrame;

struct TrapFrame4p{
	uint32_t edi, esi, ebp, xxx, ebx, edx, ecx, eax;
	uint32_t gs,fs,es,ds;
	int32_t irq;
	uint32_t err,eip;
	uint16_t cs,padding3;
	uint32_t eflags;
	uint32_t esp;
	uint16_t ss;
	uint16_t padding4;
}__attribute__((packed));

typedef struct TrapFrame4p TrapFrame4p;

#endif
