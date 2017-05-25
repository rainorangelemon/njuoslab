#include "types.h"
#include "x86.h"
#include "common.h"

#define PORT_CH_0 0x40
#define PORT_CMD 0x43
#define PIT_FREQUENCE 1193182
#define HZ 1000

void init_timer()
{
	int counter = PIT_FREQUENCE / HZ;
	outb(PORT_CMD, 0x34);
	outb(PORT_CH_0, counter % 256);         // access low byte
	outb(PORT_CH_0, counter / 256);  // access high byte
}



