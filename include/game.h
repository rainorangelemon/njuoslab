#ifndef __GAME_H__
#define __GAME_H__

#include "common.h"
#include "adt/linklist.h"

/* 初始化串口 */
void init_serial();

/* 中断时调用的函数 */
void timer_event(void);
void keyboard_event(int scan_code);

/* 按键相关 */
void press_key(int scan_code);
void release_key(int ch);
bool query_key(int ch);
int last_key_code(void);

/* 定义fly_t array */
typedef struct fly_t{
	float x,y;
	char text;
	int index;
}jkdwiejfrv;

struct fly_t box[10];

/* 主循环 */
void main_loop(void);

/* 游戏逻辑相关 */
void initial_game(void);
bool update_keypress(void);

void win_initial(void);
int winp_get(void);
bool winp_check(void);
void winp_add(void);
int winc_get(void);
bool winc_check(void);
void winc_add(void);
bool screen_full(void);
int get_fps(void);
void set_fps(int fps);

void redraw_screen(void);

/* 随机数 */
int rand(void);
void srand(int seed);

#endif
