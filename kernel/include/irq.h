#ifndef __IRQ_H__
#define __IRQ_H__

#include "types.h"

typedef struct TrapFrame {
	uint32_t edi, esi, ebp, xxx, ebx, edx, ecx, eax;
	uint16_t es;
	uint16_t pad0;
	uint16_t ds;
	uint16_t pad1;
	int32_t irq;
	uint32_t error_code;
	uint32_t eip;
	uint16_t cs;
	uint16_t pad2;
	uint32_t eflags;
	uint32_t esp;
	uint16_t ss;
	uint16_t pad3;
}TrapFrame;


#endif
