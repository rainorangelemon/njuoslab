/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_PCB_H
#define JOS_KERN_PCB_H

#include "../pm.h"

extern struct PCB *pcbs;

void	pcb_init(void);
void	pcb_init_percpu(void);
int	pcb_alloc(struct PCB **p, pid_t parent_id);
void	pcb_free(struct PCB *e);
void	pcb_create();
void	pcb_destroy(struct PCB *e);
struct PCB*seek_next_runnable();
void kernel_timer_event();

int	pid2pcb(pid_t pid, struct PCB **pcb_store, bool checkperm);
// The following two functions do not return
void	pcb_run(struct PCB *e) __attribute__(());
void	pcb_pop_tf(struct TrapFrame *tf) __attribute__(());

#endif // !JOS_KERN_PCB_H
