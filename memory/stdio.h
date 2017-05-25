#ifndef __STDIO_H__
#define __STDIO_H__

#include "types.h"
#include "stdarg.h"

void serial_printc(char);

void vprint_format(const char *fmt, va_list ap);
void print_format(const char *fmt, ...);

int printk(const char *, ...);

#endif
