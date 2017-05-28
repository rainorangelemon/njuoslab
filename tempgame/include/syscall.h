#ifndef __SYSCALL_H__
#define __SYSCALL_H__
#include "mysemaphore.h"

#define KEYCODE 0
#define VMEMORY 1
#define SERIAL  2
#define FORK    3
#define SLEEP   4
#define EXIT    5

int pthread_create(uint32_t *);
void sem_open(sem_t *, int, bool);
void sem_wait(sem_t *);
void sem_post(sem_t *);
void sem_close(sem_t *);

int open(char *, int);
int read(int, char*, int);
int write(int, char *, int);
void lseek(int, int, int);
void close(int);
#endif
