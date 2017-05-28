#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

void set_keyboard_intr_handler(void (*)(int));
/*
 * 设置键盘中断的处理函数
 *
 */

void press_key(int scan_code);
void release_key(int ch);
bool query_key(int ch);
int last_key_code(void);

#endif
