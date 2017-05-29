#include "mysemaphore.h"

void sem_open(sem_t *, int, bool);
void sem_wait(sem_t *sem);
void sem_post(sem_t *sem);
void sem_close(sem_t *sem);
