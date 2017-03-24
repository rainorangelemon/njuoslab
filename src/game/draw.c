#include "game.h"
#include "string.h"
#include "device/video.h"

/* 绘制屏幕上的内容。
 * 注意程序在绘图之前调用了prepare_buffer，结束前调用了display_buffer。
 * prepare_buffer会准备一个空白的绘图缓冲区，display_buffer则会将缓冲区绘制到屏幕上，
 * draw_pixel或draw_string绘制的内容将保存在缓冲区内(暂时不会显示在屏幕上)，调用
 * display_buffer后才会显示。
*/

void
redraw_screen() {
	const char *win;
	
	prepare_buffer(); /* 准备缓冲区 */

	/* 绘制每个字符 */
	int j;
	for(j=0;j<9;j++){	
		if(box[j].text!='\0')
			draw_character(box[j].text, box[j].x, box[j].y, 15);
	}

	/* 绘制win数、最后一次按键扫描码和fps */
	draw_string(itoa(last_key_code()), SCR_HEIGHT - 8, 0, 48);
	win = itoa(win_get());
	draw_string(win, SCR_HEIGHT - 8, SCR_WIDTH - strlen(win) * 8, 12);
	draw_string(itoa(get_fps()), 0, 0, 14);
	draw_string("FPS", 0, strlen(itoa(get_fps())) * 8, 14);

	display_buffer(); /* 绘制缓冲区 */
}

