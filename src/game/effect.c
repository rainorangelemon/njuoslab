#include "game.h"
#include "common.h"
#include "string.h"
#include "adt/linklist.h"
#include "device/video.h"
#include "x86/x86.h"

static int winp,winc;

static bool has_added;

void win_initial(){
	winp=0;
	winc=0;
	has_added=0;
}

void winp_add(){
	winp++;
	has_added=1;
}

void winc_add(){
	winc++;
	has_added=1;
}

int winc_get(){
	return winc;
}

int winp_get(){
	return winp;
}

bool winp_check(){
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

bool winc_check(){
	if(((box[0].text=='X')&&(box[1].text=='X')&&(box[2].text=='X'))||
		((box[3].text=='X')&&(box[4].text=='X')&&(box[5].text=='X'))||
		((box[6].text=='X')&&(box[7].text=='X')&&(box[8].text=='X'))||
		((box[0].text=='X')&&(box[4].text=='X')&&(box[8].text=='X'))||
		((box[2].text=='X')&&(box[4].text=='X')&&(box[6].text=='X'))||
		((box[0].text=='X')&&(box[3].text=='X')&&(box[6].text=='X'))||
		((box[1].text=='X')&&(box[4].text=='X')&&(box[7].text=='X'))||
		((box[2].text=='X')&&(box[5].text=='X')&&(box[8].text=='X'))){
		return TRUE;
	}else
		return FALSE;
}

bool screen_full(){
	int i;
	for(i=0;i<9;i++){
		if(box[i].text=='\0')
			return FALSE;
	}
	return TRUE;
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
					box[k].text='X';
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

