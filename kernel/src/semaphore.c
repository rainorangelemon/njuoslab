#include "types.h"
#include "mysemaphore.h"
#include "pcb_struct.h"

#define new     0
#define ready   1
#define running 2
#define blocked 3
#define exit    4

extern struct PCB *current;
int printk(const char *fmt, ...);
void schedule();
void release_one_blocked(int);

int sem_count = 0;	//信号量的编号
void sem_open(sem_t *sem, int value, bool mutex)
{
	printk("value %d, mutex %d\n", value, mutex);
	if(mutex)
	{
		sem->value = 1;
		sem->mutex = true;
		sem->sem_index = sem_count;		//当前信号量的编号
	printk("sem_index %d\n", sem->sem_index);
		sem->block_proc_num = 0;		//初始化被阻塞进程数为0
	}
	else
	{
		sem->value = value;
		sem->mutex = false;
		sem->sem_index = sem_count;
	printk("sem_index %d\n", sem->sem_index);
		sem->block_proc_num = 0;
	}
	sem_count ++;
}

void sem_wait(sem_t *sem)
{
	if(!sem->mutex)
	{
		sem->value = sem->value - 1;
		if(sem->value < 0)
		{
			current->status = blocked;
			current->block_sem_index = sem->sem_index;	//当前线程被阻塞的信号量编号
			sem->block_proc_num ++;		//当前信号量上阻塞的线程数加一
			schedule();
		}
	}
	else	//二元信号量
	{
		if(sem->value == 1)
			sem->value = 0;
		else
		{
			//挂进程
			current->status = blocked;
			current->block_sem_index = sem->sem_index;	//当前线程被阻塞的信号量编号
			sem->block_proc_num ++;		//当前信号量上阻塞的线程数加一
			schedule();
		}
	}
}

void sem_post(sem_t *sem)
{
	if(!sem->mutex)
	{
		sem->value = sem->value + 1;
		if(sem->value <= 0)
		{
			//释放一个挂在的进程
			sem->block_proc_num --;
			release_one_blocked(sem->sem_index);	//释放一个线程
		}
	}
	else	//二元信号量
	{
		if(sem->block_proc_num == 0)//没有进程被挂载)
			sem->value = 1;
		else
		{
			sem->block_proc_num --;
			release_one_blocked(sem->sem_index);
		}
	}
}

void sem_close(sem_t *sem)
{
	
}

