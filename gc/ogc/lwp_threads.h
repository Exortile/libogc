#ifndef __LWP_THREADS_H__
#define __LWP_THREADS_H__

#include <gctypes.h>
#include <stdlib.h>
#include <lwp_states.h>
#include <lwp_tqdata.h>
#include <lwp_watchdog.h>
#include <context.h>

//#define _LWPTHREADS_DEBUG


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _lwpwaitinfo {
	u32 id;
	u32 cnt;
	void *ret_arg;
	void *ret_arg_1;
	u32 option;
	u32 ret_code;
	lwp_queue block2n;
	lwp_thrqueue *queue;
	lwp_thrqueue join;
} lwp_waitinfo;

typedef struct _lwpcntrl {
	lwp_node node;
	
	u32 id;		//only used for gdb though
	u8  cur_prio,real_prio;
	u32 suspendcnt,res_cnt;
	u32 isr_level;
	u32 cur_state;
	
	lwp_waitinfo wait;
	prio_cntrl priomap;
	wd_cntrl timer;

	void* (*entry)(void *);
	void *arg;
	void *stack;
	u32 stack_size;
	u8 stack_allocated;
	lwp_queue *ready;
	frame_context context;		//16
	fp_context fp;
	void *libc_reent;
} lwp_cntrl, *lwp_cntrl_t;

extern lwp_cntrl *_thr_idle;
extern lwp_cntrl *_thr_executing;
extern lwp_cntrl *_thr_heir;
extern lwp_cntrl *_thr_allocated_fp;
extern lwp_queue _lwp_thr_ready[];
extern volatile boolean _context_switch_want;
extern vu32 _thread_dispatch_disable_level;
extern void **__lwp_thr_libc_reent;

void __thread_dispatch();
void __lwp_thread_yield();
void __lwp_thread_setstate(lwp_cntrl *,u32);
void __lwp_thread_clearstate(lwp_cntrl *,u32);
void __lwp_thread_changepriority(lwp_cntrl *,u32,u32);
void __lwp_thread_setpriority(lwp_cntrl *,u32);
void __lwp_thread_settransient(lwp_cntrl *);
void __lwp_thread_suspend(lwp_cntrl *);
void __lwp_thread_resume(lwp_cntrl *,u32);
void __lwp_thread_loadenv(lwp_cntrl *);
void __lwp_thread_ready(lwp_cntrl *);
u32 __lwp_thread_init(lwp_cntrl *,void *,u32,u32,u32);
u32 __lwp_thread_start(lwp_cntrl *,void* (*)(void*),void *);
void __lwp_thread_exit(void *);
void __lwp_thread_close(lwp_cntrl *);
void __lwp_start_multitasking();
void __lwp_stop_multitasking();
u32 __lwp_init();

u32 __lwp_isr_in_progress();
void __lwp_rotate_readyqueue(u32);

#include <libogc/lwp_threads.inl>

#ifdef __cplusplus
	}
#endif

#endif