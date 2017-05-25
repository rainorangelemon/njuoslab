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

// Without this extra macro, we couldn't pass macros like TEST to
// PCB_CREATE because of the C pre-processor's argument prescan rule.
#define PCB_PASTE3(x, y, z) x ## y ## z

#define PCB_CREATE(x, type)						\
	do {								\
		extern uint8_t PCB_PASTE3(_binary_obj_, x, _start)[],	\
			PCB_PASTE3(_binary_obj_, x, _size)[];		\
		pcb_create(PCB_PASTE3(_binary_obj_, x, _start),		\
			   (int)PCB_PASTE3(_binary_obj_, x, _size),	\
			   type);					\
	} while (0)

#endif // !JOS_KERN_PCB_H
