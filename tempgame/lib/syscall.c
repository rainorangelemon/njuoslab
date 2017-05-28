#include <sys/syscall.h>
#include <sys/stat.h>
#include "mysemaphore.h"

#define THREAD		6
#define SEM_OPEN	7
#define SEM_WAIT	8
#define SEM_POST	9
#define SEM_CLOSE	10

#define F_OPEN 		11
#define F_READ		12
#define F_WRITE		13
#define F_LSEEK		14
#define F_CLOSE		15

int printf(const char *fmt, ...);

int __attribute__((__noinline__))
syscall(int id, ...) {
	int ret;
	int *args = &id;
	asm volatile("int $0x80": "=a"(ret) : "a"(args[0]), "b"(args[1]), "c"(args[2]), "d"(args[3]));
	return ret;
}

int open(char *filename, int mode)
{
	return syscall(F_OPEN, (void *)filename, mode);//??
}

int read(int fd, char *buf, int count)
{
	return syscall(F_READ, fd, (void *)buf, count);
}

int write(int fd, char *buf, int count)
{
	return syscall(F_WRITE, fd, (void *)buf, count);
}

void lseek(int fd, int offset, int whence)
{
	syscall(F_LSEEK, fd, offset, whence);
}

void close(int fd)
{
	syscall(F_CLOSE, fd);
}
//------------------------------------------
int pthread_create(uint32_t *func_addr)
{
	return syscall(THREAD, func_addr);
}

void sem_open(sem_t *sem, int value, bool mutex)
{
	syscall(SEM_OPEN, (void *)sem, value, mutex);
}

void sem_wait(sem_t *sem)
{
	syscall(SEM_WAIT, (void *)sem);
}

void sem_post(sem_t *sem)
{
	syscall(SEM_POST, (void *)sem);
}

void sem_close(sem_t *sem)
{
	syscall(SEM_CLOSE, (void *)sem);
}
