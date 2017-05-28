//#include "common.h"
#include "include/x86.h"
#include "include/syscall.h"
//#include "include/assert.h" 

int syscall(int id, ...);

/* a-z对应的键盘扫描码 */
static int letter_code[] = {
	17, 30, 31, 32, 28, 19//W, A, S, D, ENTER, Reset
};
/* 对应键按下的标志位 */
static bool letter_pressed[6];

void
press_key(int scan_code) {
	int i;
	for (i = 0; i < 6; i ++) {
		if (letter_code[i] == scan_code) {
			letter_pressed[i] = true;
		}
	}
}

void
release_key(int index) {
//	assert(0 <= index && index < 6);
	letter_pressed[index] = false;
}

bool
query_key(int index) {
//	assert(0 <= index && index < 6);
	return letter_pressed[index];
}

/* key_code保存了上一次键盘事件中的扫描码 */
static volatile int key_code = 0;

int last_key_code(void) {
	return key_code;
}

void
keyboard_event() {
	int key_code;
	key_code = syscall(KEYCODE);
	press_key(key_code);
}

