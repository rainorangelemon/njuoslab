#ifndef __ASSERT_H__
#define __ASSERT_H__

int abort(const char *, int);
void printk(const char *, ...);

/* assert: 断言条件为真，若为假则蓝屏退出 */
#define assert(cond) \
	((cond) ? (0) : (abort(__FILE__, __LINE__)))

#define Assert(cond,...)\
	do{\
		if(!(cond)){\
			printk(__VA_ARGS__);\
			assert(0);\
		}\
	}while(0)

#define panic(...)\
	Assert(0,__VA_ARGS__)

#endif
