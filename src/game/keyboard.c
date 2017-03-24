#include "common.h"
#include "string.h"

/* 1-9对应的键盘扫描码 */
static int letter_code[] = {
	2,3,4,5,6,7,8,9,10
};
/* 对应键按下的标志位 */
static bool letter_pressed[9];

void
press_key(int scan_code) {
	int i;
	for (i = 0; i < 9; i ++) {
		if (letter_code[i] == scan_code) {
			letter_pressed[i] = TRUE;
		}
	}
}

void
release_key(int index) {
	assert(0 <= index && index < 9);
	letter_pressed[index] = FALSE;
}

bool
query_key(int index) {
	assert(0 <= index && index < 9);
	return letter_pressed[index];
}

/* key_code保存了上一次键盘事件中的扫描码 */
static volatile int key_code = 0;

int last_key_code(void) {
	return key_code;
}

void
keyboard_event(int code) {
	key_code = code;
	press_key(code);
}

