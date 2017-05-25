#include "../include/stdio.h"
#include "../include/pmap.h"
#include "../include/process/env.h"
#include "../include/env.h"
#include "../include/memlayout.h"
#include "../include/error.h"
#include "../include/string.h"
#include "../include/common.h"

uint32_t time_tick=0;

struct Env*seek_next_runnable()
{
	struct Env*env;
	env = curenv;
	if (curenv->env_status == ENV_RUNNING)
		curenv->env_status = ENV_RUNNABLE;
	int i;
	for (i = 0;i <= NENV + 1;i++)
	{
		if (env == &envs[NENV - 1]) 
			env = envs;
		else
			env++;
		if (env->env_status == ENV_RUNNABLE) return env;
	}
	printk("no process\n");
	return NULL;
}

void kernel_timer_event()
{
	time_tick++;
	int i;
	for (i = 0;i < NENV;i++)
	{
		if (envs[i].env_status == ENV_SLEEP)
		{
			envs[i].sleep_time -= 1;
			if (envs[i].sleep_time == 0) envs[i].env_status = ENV_RUNNABLE;
		}
	}
	struct Env* env = seek_next_runnable();
	env_run(env);
}

int system_env_fork()
{
	int i,j;
	envid_t p_id = curenv->env_id;
	struct Env *env = NULL;
	int judge = env_alloc(&env,p_id);
	if (judge != 0)
	{
		printk("env_alloc error!");
		return judge;
	}
	for (i = 0;i < 1024;i++)
	{
		if (!(env->env_pgdir[i] & PTE_P) && (curenv->env_pgdir[i] & PTE_P))
		{
			pte_t* pte = (pte_t*)(page2kva)((pa2page)(curenv->env_pgdir[i]));
			for (j = 0;j < 1024;j++)
			{
				if (pte[j])
				{
					struct PageInfo* p = NULL;
					if (!(p = page_alloc(1))) return -E_NO_MEM;
					p->pp_ref++;
					page_insert(env->env_pgdir,p,(void*)(i << 22 | j << 12),PTE_U | PTE_W);
					memcpy((void *)page2kva(p),(void*)page2kva(pa2page(PTE_ADDR(pte[j]))),4096);	
				}
			}
		}
	}
	env->env_tf = curenv->env_tf;
	env->env_tf.eax = 0;
	return env->env_id;
}

void system_env_sleep(uint32_t time)
{
	curenv->env_status = ENV_SLEEP;
	curenv->sleep_time = time;
	struct Env* env = seek_next_runnable();
	env_run(env);
}

void system_env_exit()
{
	curenv->env_status = ENV_DYING;
	env_destroy(curenv);
	struct Env* env = seek_next_runnable();
	env_run(env);
}

