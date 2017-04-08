#include "x86.h"

#define PORT 0x3f8   /* COM1 */


void init_serial() {
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x80);
	outb(PORT + 0, 0x03);
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x03);
	outb(PORT + 2, 0xC7);
	outb(PORT + 4, 0x0B);
}

int is_serial_idle() {
	return inb(PORT + 5) & 0x20;
}

void serial_printc(char a){
	while(!is_serial_idle());
	outb(PORT, a);
}

void serial_output_test(){
	outb(PORT + 0, 'Y');
	outb(PORT + 0, 'C');
	outb(PORT + 0, 'N');
}
