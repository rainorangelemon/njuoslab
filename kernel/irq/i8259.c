#include "x86.h"

#define PORT_PIC_MASTER 0x20
#define PORT_PIC_SLAVE  0xA0
#define IRQ_OFFSET		0x20
#define IRQ_SLAVE       2

void init_i8259(void) {
	/* mask all interrupts */
	outb(PORT_PIC_MASTER + 1, 0xFF);
	outb(PORT_PIC_SLAVE + 1 , 0xFF);
	
	/* start initialization */
	outb(PORT_PIC_MASTER, 0x11);
	outb(PORT_PIC_MASTER + 1, IRQ_OFFSET);
	outb(PORT_PIC_MASTER + 1, 1 << IRQ_SLAVE);
	outb(PORT_PIC_MASTER + 1, 0x3); // Modified part
	outb(PORT_PIC_SLAVE, 0x11);
	outb(PORT_PIC_SLAVE + 1, IRQ_OFFSET + 8);
	outb(PORT_PIC_SLAVE + 1, IRQ_SLAVE);
	outb(PORT_PIC_SLAVE + 1, 0x3); // Modified part
	outb(PORT_PIC_MASTER, 0x68);
	outb(PORT_PIC_MASTER, 0x0A);
	outb(PORT_PIC_SLAVE, 0x68);
	outb(PORT_PIC_SLAVE, 0x0A);
}
