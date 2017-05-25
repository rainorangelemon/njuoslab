#include "../include/stdio.h"
#include "../include/x86.h"
#include "../include/common.h"

const char *panicstr;

void _panic(const char *file, int line, const char *fmt,...)
{
	va_list ap;
	if (panicstr) goto dead;
	panicstr = fmt;
	// Be extra sure that the machine is in as reasonable state
	__asm __volatile("cli; cld");
	va_start(ap, fmt);
	printk("kernel panic at %s:%d: ", file, line);
	vprintk(fmt, ap);
	printk("\n");
	va_end(ap);

dead:
	hlt();
}

void _warn(const char *file, int line, const char *fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	printk("kernel warning at %s:%d: ", file, line);
	vprintk(fmt, ap);
	printk("\n");
	va_end(ap);
}
