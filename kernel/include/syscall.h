#ifndef __SYSCALL_H__
#define __SYSCALL_H__

int get_time();
int get_keyboard();
int write(int, char*, int);
int display_video(uint8_t*);


#endif 
