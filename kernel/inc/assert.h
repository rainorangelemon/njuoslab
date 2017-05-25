#ifndef __ASSERT_H__
#define __ASSERT_H__

int abort(const char *, int);
int printk(const char *, ...);

/*
#define HIT_GOOD_TRAP \
	asm volatile(".byte 0xd6" : : "a" (0))

#define HIT_BAD_TRAP \
	asm volatile(".byte 0xd6" : : "a" (1))
*/

#define assert(cond) \
	((cond) ? (0) : (abort(__FILE__, __LINE__)))

#define Assert(cond, ...) \
	do { \
		if(!(cond)) { \
			printk(__VA_ARGS__); \
			assert(0); \
		} \
	} while(0)

#define panic(...) \
	Assert(0, __VA_ARGS__)

#endif
