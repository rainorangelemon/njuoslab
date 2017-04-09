#include "mmu.h"

#define GDT_ENTRY(n)	\
	((n) << 3)

# #define MAKE_NULL_SEG_DESC	\
# 	.word 0, 0;
#	.byte 0, 0, 0, 0

# #define MAKE_SEG_DESC(type, base, lim)						\
#	.word (((lim) >> 12) & 0xffff), ((base) & 0xffff);		\
#	.byte (((base) >> 16) & 0xff), (0x90 | (type)),			\
#	(0xc0 | ((lim) >> 28) & 0xff)), (((base) >> 24) &0xff)

.globl start
start:
	.code16				# Assemble for 16-bit mode
	cli					# Disable interrupts
#	cld					# String operations increment(necessary?)

	# Open graph mode:

	movw	$0x13, %ax
	int		$0x10

	# Enable A20:
	seta20.1:
		inb		$0x64, %al
		testb	$0x2, %al
		jnz		seta20.1

		movb	$0xd1, %al
		outb	%al, $0x64

	seta20.2:
		inb		$0x64, %al
		testb	$0x2, %al
		jnz		seta20.2

		movb	$0xdf, %al
		outb	%al, $0x60

	# Set up the flat mode:
	xorw	%ax, %ax
	movw	%ax, %ds	# Data Segment
	movw	%ax, %es	# Extra Segment
	movw	%ax, %ss	# Stack Segment

	# Switch from real to protected mode
	lgdt	gdtdesc		# Set up GDT
	movl	%cr0, %eax
	orl		$0x1, %eax
	movl	%eax, %cr0	# CR0 |= PROTECT_ENABLE_BIT
	ljmp	$GDT_ENTRY(1), $start32
						# Set CS to GDT(1) and long jump to start32 in 32-bit mode


start32:
	.code32				# Assemble for 32-bit mode

	# Set up the protected-mode data segment registers
	movw	$GDT_ENTRY(2), %ax
	movw	%ax, %ds	# Data Segment
	movw	%ax, %es	# Extra Segment
#	movw	%ax, %fs	# (unnecessery)
#	movw	%ax, %gs	# (unnecessery)
	movw	%ax, %ss	# Stack Segment

	# Set up the stack pointer and call into C
	movl	$0x8000000, %esp
	call	bootmain


# GDT
.p2align 2
gdt:
	SEG_NULL							# empty segment
	SEG(0xA, 0x0, 0xffffffff)			# code
	SEG(0x2, 0x0, 0xffffffff)			# data

gdtdesc:
	.word	(gdtdesc - gdt - 1)		# limit = sizeof(gdt) - 1
	.long	gdt						# address of GDT

