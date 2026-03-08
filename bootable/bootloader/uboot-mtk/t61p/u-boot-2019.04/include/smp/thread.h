#ifndef _THREAD_H_
#define _THREAD_H_

#include "thread_info.h"

#define SMP_USING_MUTEX 0

thread_t *thread_create_ex(const char *name, void *(*start)(void *arg), void *arg, int stk_size, unsigned int cpu_id, int priority, int join);
thread_t *thread_create(const char *name, void *(*start)(void *arg), void *arg);

thread_t *thread_init(void);
thread_t *thread_init_per_cpu(void);

void thread_start(void);
void thread_exit(void *retval);
int thread_join(thread_t *thread, void **retvalp);
int thread_cond_new(cond_t *cvar);
//int thread_cond_destroy(cond_t *cvar);
int thread_cond_timedwait(cond_t *cvar, utime_t timeout);
int thread_cond_signal(cond_t *cvar);

#if SMP_USING_MUTEX
void mutex_new(mutex_t *lock);
int mutex_destroy(mutex_t *lock);
int mutex_lock(mutex_t *lock);
int mutex_unlock(mutex_t *lock);
int mutex_trylock(mutex_t *lock);
#endif

void add_sleep_q(thread_t *thread, utime_t timeout);
void del_sleep_q(thread_t *thread);

void create_thread_test(void);
void display_run_threads(void);
void display_all_threads(void);

int thread_sleep(int secs);
int thread_msleep(unsigned int msecs);
u64 random(u64 limit);
void *main_init(void *arg);
#endif

