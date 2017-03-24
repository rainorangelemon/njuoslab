#include "game.h"
#include "common.h"
#include "string.h"
#include "adt/linklist.h"
#include "device/video.h"
#include "x86/x86.h"

static int win;

void win_initial(){
	win=0;
}

void win_add(){
	win++;
}

int win_get(){
	return win;
}

bool win_check(){
	if(((box[0].text=='O')&&(box[1].text=='O')&&(box[2].text=='O'))||
		((box[3].text=='O')&&(box[4].text=='O')&&(box[5].text=='O'))||
		((box[6].text=='O')&&(box[7].text=='O')&&(box[8].text=='O'))||
		((box[0].text=='O')&&(box[4].text=='O')&&(box[8].text=='O'))||
		((box[2].text=='O')&&(box[4].text=='O')&&(box[6].text=='O'))||
		((box[0].text=='O')&&(box[3].text=='O')&&(box[6].text=='O'))||
		((box[1].text=='O')&&(box[4].text=='O')&&(box[7].text=='O'))||
		((box[2].text=='O')&&(box[5].text=='O')&&(box[8].text=='O'))){
		return TRUE;
	}else
		return FALSE;
}

/* 更新按键 */
bool
update_keypress(void) {
	int target=0;
	
	disable_interrupt();
	/* 寻找相应键已被按下、最底部且未被击中的字符 */
	int j;
	for (j = 0; j < 9; j++) {
		if ((box[j].text=='\0')&&(query_key(box[j].index))) {
			box[j].text='O';
			target=j+1;
			int k;
			for(k=0;k<9;k++){
				if (box[k].text=='\0') {
					box[j].text='X';
					break;
				}
			}
			break;
		}
	}
	/* 如果找到则更新相应数据 */
	if (target != 0) {
		release_key(box[j].index);
		return TRUE;
	}
	enable_interrupt();

	return FALSE;
}

