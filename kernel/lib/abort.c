#include "common.h"
#include "x86.h"

int abort(const char* filename, int line) {
	cli();
	printk("%s,%d: Boom!\n", filename, line);
	while(1);
	return 0;
}
