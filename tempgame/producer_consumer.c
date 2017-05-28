#include "include/syscall.h"
#include "mysemaphore.h"

int printf(const char *fmt, ...);
int syscall(int id, ...);
int pthread_create(uint32_t *);
void *  memset(void *dst, int c, size_t len);

#define N 2		//消费者或生产者的数目
#define M 10	//缓冲区的数目

int in = 0;		//生产的商品的放置位置
int	out = 0;	//消费者取商品的位置
int buf[M] = {0};
sem_t empty_sem;	//同步信号量，当满了的时候阻塞生产者
sem_t full_sem;		//同步信号量，当空了的时候阻塞消费者
sem_t mutex;

int product_id = 0;	//生产者线程id
int consume_id = 0;	//消费者线程id

void print()
{
	int i;
	for(i = 0; i < M; i ++)
		printf("%d ", buf[i]);
	printf("\n");
}

void product()
{
	int id = product_id++;
	while(1)
	{
		//sleep(1);
		sem_wait(&empty_sem);
		sem_wait(&mutex);
		in = in % M;
		printf("producter%d in %d.\n", id, in);
		buf[in] = 1;
		print();
		in++;
		sem_post(&mutex);
		sem_post(&full_sem);
	}
}

void consume()
{
	int id = consume_id++;
	while(1)
	{
		//sleep(1);
		sem_wait(&full_sem);
		sem_wait(&mutex);
		out = out % M;
		printf("consumer%d in %d.\n", id, out);
		buf[out] -= 1;
		print();
		out++;
		sem_post(&mutex);
		sem_post(&empty_sem);
	}
}

void pc_problem()
{
	in = 0;
	out = 0;
	product_id = 0;
	consume_id = 0;
	memset(buf, 0, sizeof(buf));
	sem_open(&empty_sem, M, 0);
	sem_open(&full_sem, 0, 0);
	sem_open(&mutex, 1, 1);
printf("sem_open success\n");

//	int i;
	//创建N个生产者
//	for(i = 0; i < N; i ++)
//	{
		pthread_create((uint32_t *)product);
//	printf("success productor\n");
//	}
	//创建N个消费者
//	for(i = 0; i < N; i ++)
//	{
		pthread_create((uint32_t *)consume);
//	printf("success consumer\n");
//	}
	while(1);
}
