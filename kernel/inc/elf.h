#ifndef __ELF_H__
#define __ELF_H__

#define ELF_MAGIC 0x464C457FU

/* ELF32二进制文件头 */
struct ELFHeader {
	unsigned int   magic;
	unsigned char  elf[12];
	unsigned short type;
	unsigned short machine;
	unsigned int   version;
	unsigned int   entry;
	unsigned int   phoff;
	unsigned int   shoff;
	unsigned int   flags;
	unsigned short ehsize;
	unsigned short phentsize;
	unsigned short phnum;
	unsigned short shentsize;
	unsigned short shnum;
	unsigned short shstrndx;
};

/* ELF32 Program header */
struct ProgramHeader {
	unsigned int type;
	unsigned int off;
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int filesz;
	unsigned int memsz;
	unsigned int flags;
	unsigned int align;
};

struct SectionHeader{
	unsigned int name;
	unsigned int type;
	unsigned int flags;
	unsigned int addr;
	unsigned int offset;
	unsigned int size;
	unsigned int link;
	unsigned int info;
	unsigned int addralign;
	unsigned int entsize;
};

#define ELF_PROG_LOAD 1
#define ELF_PROG_FLAG_EXEC  1
#define ELF_PROG_FLAG_WRITE 2
#define ELF_PROG_FLAG_READ  4

#define ELF_SHT_NULL 0
#define ELF_SHT_PROGBITS 1
#define ELF_SHT_SYMTAB 2
#define ELF_SHT_STRTAB 3

#define ELF_SHN_UNDEF 0

#endif
