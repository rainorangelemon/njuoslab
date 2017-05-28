#include "include/syscall.h"
int printf(const char *fmt, ...);
int syscall(int id, ...);

#define FORK 3
int Fork()
{
	int i = syscall(FORK);
	printf("FORK %d\n", i);
	return i;
	//return syscall(FORK);
}

void pingpang()
{
	if(Fork()==-1)
	{
		while(1)
		{
			syscall(SLEEP, 100);
			printf("ping\n");
			//printf("pingping\n");
			syscall(SLEEP, 100);
		}
	}
	else
	{
		while(1)
		{
			printf("pong\n");
			syscall(SLEEP, 300);
		}
	}
}
