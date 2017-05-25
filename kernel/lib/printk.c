#include "../include/x86.h"
#include "../include/stdarg.h"
#include "../include/stdio.h"

void serial_printc(char);

static void putch(int ch, int *cnt)
{
	serial_printc(ch);
	(*cnt)++;
}


int vprintk(const char *fmt,va_list ap)
{
	int cnt = 0;
	vprintfmt((void*)putch,&cnt,fmt,ap);
	return cnt;
}

int	printk(const char *fmt, ...)
{
	va_list ap;
	int cnt;
	va_start(ap,fmt);
	cnt = vprintk(fmt,ap);
	va_end(ap);
	return cnt;
}
