#include "device/video_mode.h"
#include "string.h"
#include "device/video.h"

uint8_t *vmem;

void init_vmem_addr() {
	struct ModeInfoBlock *MIB = (struct ModeInfoBlock *)(0x7e00);
	vmem = (uint8_t *)MIB->physbase;
}

void init_vmem() {
	memcpy(vmem, 0, SCR_SIZE);
}

int load_vmem(uint8_t *buffer) {
	memcpy(vmem, buffer, SCR_SIZE);
	return 1;
}
