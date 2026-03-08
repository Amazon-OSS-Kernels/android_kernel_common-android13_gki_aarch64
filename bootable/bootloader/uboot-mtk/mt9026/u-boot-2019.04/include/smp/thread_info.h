#ifndef __THREAD_INFO_H__
#define __THREAD_INFO_H__

#include "spinlock.h"
#include <linux/list.h>

#define MAX_THREAD_NAME 15
#define SUPPORT_PREEMPTION

#define MILLISECOND 1000

//#define THREAD_DEBUG

#ifdef THREAD_DEBUG
#define dbg_print(fmt, ...) printf(fmt, ##__VA_ARGS__);
#define tlog(fmt, ...) printf(fmt, ##__VA_ARGS__);
#else
#define dbg_print(fmt, ...) do {; } while (0)
#define tlog(fmt, ...) do {; } while (0)
#endif


#define THREAD_MAX_PRIORITY             255
#define THREAD_DEFAULT_PRIORITY         127
#define THREAD_MIN_PRIORITY             1
#define THREAD_IDLE_PRIORITY            0

#define MIN_STACK_SIZE 			64 * 1024
#define DEFAULT_STACK_SIZE 		128 * 1024
#define STACK_BUFFER_SIZE 		32

typedef struct _list {
	struct _list *next;
	struct _list *prev;
} list_t;

/* Insert element "_e" into the list, before "_l" */
#define INSERT_BEFORE(_e, _l)	 \
	{ \
(_e)->next = (_l);	 \
(_e)->prev = (_l)->prev; \
(_l)->prev->next = (_e); \
(_l)->prev = (_e);	 \
	}

/* Insert element "_e" into the list, after "_l" */
#define INSERT_AFTER(_e, _l)	 \
	{ \
(_e)->next = (_l)->next; \
(_e)->prev = (_l);	 \
(_l)->next->prev = (_e); \
(_l)->next = (_e);	 \
	}
/* Return the element following element "_e" */
#define NEXT_LINK(_e)  ((_e)->next)

/* Append an element "_e" to the end of the list "_l" */
#define APPEND_LINK(_e, _l) INSERT_BEFORE(_e, _l)

/* Insert an element "_e" at the head of the list "_l" */
#define INSERT_LINK(_e, _l) INSERT_AFTER(_e, _l)

/* Remove the element "_e" from it's circular list */
#define REMOVE_LINK(_e)	       \
	{\
(_e)->prev->next = (_e)->next; \
(_e)->next->prev = (_e)->prev; \
	}
/* Return non-zero if the given circular list "_l" is empty, */
/* zero if the circular list is not empty */
#define LIST_IS_EMPTY(_l) \
	((_l)->next == (_l))

/* Initialize a circular list */
#define INIT_LIST(_l)  \
	{ \
(_l)->next = (_l); \
(_l)->prev = (_l); \
	}
#define INIT_STATIC_LIST(_l) \
{(_l), (_l)}


/*****************************************
 * Basic types definitions
 */
typedef void (*CoreWakeupFunc) (void);
typedef unsigned int utime_t;
typedef struct thread thread_t;
typedef struct regs {
	u64 sp, lr, pc;
	u64 spsr;
	u64 r[13];
} regs_t;

typedef struct _wait_queue {
	list_t links;
	unsigned int flags;
	smp_spin_lock_t lock;
} wait_queue_t, cond_t;


struct thread {
	regs_t reg;
	unsigned int state;         /* Thread's state */
	int flags;                  /* Thread's flags */

	void *(*start)(void *arg);  /* The start function of the thread */
	int priority;
	unsigned int cpu_id;
	char name[MAX_THREAD_NAME + 1];
	void *arg;                  /* Argument of the start function */
	void *retval;               /* Return value of the start function */

	list_t links;          /* For putting on run/sleep/zombie queue */
	list_t wait_link;     /* For putting on mutex/condvar wait queue */
	list_t tlink;          /* For putting on thread queue */

	utime_t due;             /* Wakeup time when thread is sleeping */
	thread_t *left;         /* For putting in timeout heap */
	thread_t *right;	      /* -- see docs/timeout_heap.txt for details */
	thread_t *prev;	      /* -- see docs/timeout_heap.txt for details */
	int heap_index;

	cond_t *term;           /* Termination condition variable for join */
	unsigned long stack_top;
	unsigned int start_sleep;
};


typedef struct _mutex {
	thread_t *owner;        /* Current mutex owner */
	wait_queue_t wait;
} mutex_t;


typedef struct _scheduler {
	thread_t *idle_thread[NR_CPUS];  /* Idle thread for this vp */

	list_t run_q[NR_CPUS];          /* run queue for this vp */
	//list_t zombie_q[NR_CPUS];       /* zombie queue for this vp */
	list_t thread_q;       /* all threads of this vp */

	thread_t *sleep_q[NR_CPUS];      /* sleep queue for this vp */
	int sleepq_size[NR_CPUS];	      /* number of threads on sleep queue */

	int need_schedule[NR_CPUS];
} scheduler_t ;

extern scheduler_t g_scheduler ;
extern thread_t *current_thread[NR_CPUS];
extern const char *thread_state_str[];
extern int active_count;
extern int init_done[NR_CPUS];
extern cond_t g_sub_cond[NR_CPUS];
extern int smp_cpu_released[NR_CPUS];

/*****************************************
 * Thread states and flags
 */

#define STATE_IDLE		   0
#define STATE_RUNNING      1
#define STATE_RUNNABLE     2
#define STATE_LOCK_WAIT    3
#define STATE_COND_WAIT    4
#define STATE_SLEEPING     5
#define STATE_ZOMBIE       6
#define STATE_SUSPENDED    7
#define STATE_DONE		   8

#define FLAG_PRIMORDIAL   0x01
#define FLAG_IDLE_THREAD  0x02
#define FLAG_ON_SLEEPQ    0x04
#define FLAG_INTERRUPT    0x08
#define FLAG_TIMEDOUT     0x10
#define FLAG_FIX_CPU	  0x20
#define FLAG_JOINED       0x40


#define get_current_thread(i)            (current_thread[i])
#define set_current_thread(_thread, i) (current_thread[i] = (_thread))

#define get_idle_thread(i)			(g_scheduler.idle_thread[i])
#define get_run_queue(i)                        (g_scheduler.run_q[i])
//#define get_zombie_queue(i)                     (g_scheduler.zombie_q[i])
#define get_thread_queue()                     (g_scheduler.thread_q)

#define get_sleep_queue(i)                      (g_scheduler.sleep_q[i])
#define SLEEPQ_SIZE(i)                 (g_scheduler.sleepq_size[i])

#define ADD_RUNQ(_thr, i)  APPEND_LINK(&(_thr)->links, &get_run_queue(i))
#define DEL_RUNQ(_thr)  REMOVE_LINK(&(_thr)->links)

//#define ADD_ZOMBIEQ(_thr,i)  APPEND_LINK(&(_thr)->links, &get_zombie_queue(i))
//#define DEL_ZOMBIEQ(_thr)  REMOVE_LINK(&(_thr)->links)

#define ADD_THREADQ(_thr)  APPEND_LINK(&(_thr)->tlink, &get_thread_queue())
#define DEL_THREADQ(_thr)  REMOVE_LINK(&(_thr)->tlink)

/*****************************************
 * Pointer conversion
 */

#ifndef offsetof
#define offsetof(type, identifier) ((size_t)&(((type *)0)->identifier))
#endif

#define get_thread_ptr(_qp)         \
	((thread_t *)((char *)(_qp) - offsetof(thread_t, links)))

#define get_thread_ptr_waitq(_qp)   \
	((thread_t *)((char *)(_qp) - offsetof(thread_t, wait_link)))

#define get_thread_ptr_threadq(_qp)         \
	((thread_t *)((char *)(_qp) - offsetof(thread_t, tlink)))


/*****************************************
 * Constants
 */

#ifndef NO_TIMEOUT
#define NO_TIMEOUT (~0U)
#endif

/*****************************************
 * Function prototype
 */
void set_physical_timer_period(unsigned);
void ipi_send_target(u8);
void release_non_boot_core(void);
void reschedule_irq(void);
void Core_Wakeup(CoreWakeupFunc, u32);

#endif /* !__THREAD_INFO_H__ */

