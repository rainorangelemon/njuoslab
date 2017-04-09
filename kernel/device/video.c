#include "string.h"
#include "device/video.h"

uint8_t *vmem;

void init_vmem_addr() {
	vmem=((uint8_t*)0xFD000000);
}

int load_vmem(uint8_t *buffer) {
	memcpy(vmem, buffer, SCR_SIZE);
	return 1;
}
