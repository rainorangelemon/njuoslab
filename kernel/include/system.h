#include"types.h"
#include"env.h"
#define  drawpixeloff (62)
#define  drawpixel (60)
#define  serialprint (61)
#define  clearscreen (63)
#define  initserial (64)
#define  inittimer (65)
#define  enableinterrupt (66)
#define  disenableinterrupt (67)
#define  env_fork (68)
#define  env_sleep (69)
#define  env_exit (70)
#define  SYS_video (71)
#define  SYS_kbd (72)
#define  SYS_time (73)
#define  SYS_write (74)
#define  ReKbdbuf (76)

int __attribute__((__noinline__)) syscall(int id,...);
void system_draw_pixel( int, int, int);
void system_serial_print(char);
void system_draw_pixel_off( int, int);
void system_clear_screen(int);
void system_init_serial();
void system_init_timer();
void system_enable_interrupt();
void system_disenable_interrupt();
int system_env_fork();
void system_env_sleep(uint32_t);
void system_env_exit();
struct Env*seek_next_runnable();
void kernel_timer_event();
