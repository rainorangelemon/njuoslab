#include "common.h"
#include "x86.h"

#define PORT_CH_0 0x40
#define PORT_CMD 0x43
#define PIT_FREQUENCE 1193182
#define HZ 1000

union CmdByte {
	struct {
		uint8_t present_mode : 1;
		uint8_t operate_mode : 3;
		uint8_t access_mode  : 2;
		uint8_t channel      : 2;
	};
	uint8_t val;
};

union CmdByte mode = {
	.present_mode = 0,  // 16-bit binary
	.operate_mode = 2,  // rate generator, for more accuracy
	.access_mode  = 3,  // low byte / high byte, see below
	.channel      = 0,  // use channel 0
};

void init_timer() {
	int counter = PIT_FREQUENCE / HZ;
	outb(PORT_CMD, mode.val);
	outb(PORT_CH_0, counter & 0xFF);         // access low byte
	outb(PORT_CH_0, (counter >> 8) & 0xFF);  // access high byte
}

volatile uint32_t time_tick = 0;

void timer_event(void) {
	time_tick ++;
}
