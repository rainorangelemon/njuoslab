#include "../include/stdio.h"
#include "../include/x86.h"

int printk(const char *ctl, ...);

int abort(const char* filename,int line)
{
	cli();
	printk("%s,%d: Failure!\n", filename, line);
	while(1);
	return 0;
}
