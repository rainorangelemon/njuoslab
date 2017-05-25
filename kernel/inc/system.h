#include"types.h"
#include"pm.h"
#ifndef RAIN_SYS_H
#define RAIN_SYS_H 
#define  drawpixeloff (62)
#define  drawpixel (60)
#define  serialprint (61)
#define  clearscreen (63)
#define  initserial (64)
#define  inittimer (65)
#define  enableinterrupt (66)
#define  disenableinterrupt (67)
#define  SYS_fork (68)
#define  SYS_sleep (69)
#define  SYS_exit (70)
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
int system_pcb_fork();
void system_pcb_sleep(uint32_t);
void system_pcb_exit();
struct PCB*seek_next_runnable();
void kernel_timer_event();
#endif
