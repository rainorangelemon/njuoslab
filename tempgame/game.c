#include "include/string.h"
#include "include/x86.h"
#include "include/video.h"
#include "include/keyboard.h"
#include "include/syscall.h"

int syscall(int id, ...);


void add_irq_handle(int irq, void *handler) {
	syscall(0, irq, handler);
}
void keyboard_event();

volatile int tick = 0;

void
timer_event(void) {
	tick ++;
}

static int real_fps;
void
set_fps(int value) {
	real_fps = value;
}

int
get_fps() {
	return real_fps;
}

struct Pos
{
	int x,y;
	int length;
	int width;
	int dir;
}pos;

int printf(const char *fmt, ...);

//int a = 0;

#define FORK 3
int fork()
{
	int i = syscall(FORK);
	printf("FORK %d\n", i);
	return i;
//	return syscall(FORK);
}

#define EXIT    5
void Exit()
{
	syscall(EXIT);
}

int main_loop()
{
	printf("game start!\n");
	pos.x=3,pos.y=0;
	pos.length=37, pos.width=37;

	if(fork()==-1)	//子进程
	{
		printf("this is the child proc\n");
		Exit();
//		syscall(SLEEP, 100);
	}
	else	//父进程
	{
		printf("this is the father proc\n");
//		syscall(SLEEP, 100);
	}

//	syscall(SLEEP, 100);  //sleep系统调用

	while(1)
	{
		prepare_buffer();
		init_screen();
		pos.dir = -1;

		keyboard_event();
		
		for(int i=0;i<6;i++)
		{
			if(query_key(i))
			{
				pos.dir = i;
				release_key(i);
				break;
			}
		}

		if(pos.dir == 0 && pos.x >3)
			pos.x -= 39;
		else if(pos.dir == 1 && pos.y > 0)
			pos.y -= 39;
		else if(pos.dir == 2 && (pos.x + pos.width) < 196)
			pos.x += 39;
		else if(pos.dir == 3 && (pos.y + pos.length) < 193)
			pos.y += 39;
		else if(pos.dir == 4)
			light_state(pos.x, pos.y);
		else if(pos.dir == 5)
			reset();
		draw_light(pos.length, pos.width);
		draw_obj(pos.x, pos.y, pos.length, pos.width);
		draw_success();			
		display_buffer();

//		syscall(SLEEP, 100);  //sleep系统调用
	}
}
