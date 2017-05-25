#ifndef __GAME_H__
#define __GAME_H__
#include "keyboard.h"

/* 初始化串口 */
void init_serial();

/* 中断时调用的函数 */
/*void timer_event(void);
void keyboard_event(int scan_code);*/
void init_timer();
void set_timer_intr_handler(void (*ptr)(void));
/*init_idt*/
void init_idt();
void init_intr();
/* 按键相关 */
void set_keyboard_intr_handler(void(*ptr)(int));
void keyboard_event();
void press_key(int scan_code);
void release_key(int ch);
bool query_key(int ch);
int last_key_code(void);

/* 随机数 */
int rand(void);
void srand(int seed);
/*about game*/
void init_squares();
void game_loop();

#endif
