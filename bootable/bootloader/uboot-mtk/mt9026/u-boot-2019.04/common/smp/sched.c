#include <exports.h>
#include <config.h>
#include <errno.h>
#include <thread_info.h>
#include <thread.h>
#include <asm/smp_platform.h>
#include <asm/system.h>
#include <asm/psci.h>
#include <asm/io.h>
#include <debug_impl.h>
#include <linux/arm-smccc.h>

#define ENABLE_SMP

volatile u8 ready_for_parking[NR_CPUS] = {0};
volatile u8 SMP_parking_start;
int init_done[NR_CPUS] = {0};
int smp_cpu_released[NR_CPUS];
//static unsigned int oneshot_timer_id[NR_CPUS];  // sherlock: no prana timer flow here, don't use it
cond_t g_sub_cond[NR_CPUS];
static smp_spin_lock_t g_thread_lock = INIT_SPIN_LOCK;
smp_spin_lock_t g_release_lock = INIT_SPIN_LOCK;
smp_spin_lock_t g_sched_lock = INIT_SPIN_LOCK;
scheduler_t g_scheduler;
thread_t *current_thread[NR_CPUS];  /* Current thread */
int active_count;       /* Active thread count */
volatile int magic_number_cleaned;

int schedule(void);

extern void __switch_context(thread_t*, thread_t*);
extern void secondary_start_uboot_cleanup(void);

void add_run_queue(thread_t *);

const char *thread_state_str[] = {
	"idle",
	"running",
	"ready",
	"lock-wait",
	"cond-wait",
	"sleeping",
	"zombie",
	"suspended",
	"done"
};

static void _thread_main(void* (*func)(void *), void *arg, thread_t *thread)
{
	dbg_print("cpu:%d thread->name:%s in _thread_main\n", thread->cpu_id,  thread->name);
	//thread_info_dump(thread);
	if (!(thread->flags & FLAG_PRIMORDIAL)) {
		dbg_print("thread %s not main thread\n", thread->name);
		smp_spin_unlock(&g_sched_lock);
		irq_enable();
	}
	thread->retval = func(arg);
	thread_exit(thread->retval);
}

#define HEAP_INS_DEL_BITS 2
static thread_t **heap_insert(thread_t *thread)
{
	int target = thread->heap_index;
	int s = target;
	unsigned int cpu_id = thread->cpu_id;
	thread_t **p = &get_sleep_queue(cpu_id);
	int bits = 0;
	int bit;
	int index = 1;

	while (s) {
		s >>= 1;
		bits++;
	}
	for (bit = bits - HEAP_INS_DEL_BITS; bit >= 0; bit--) {
		if (thread->due < (*p)->due) {
			thread_t *t = *p;
			thread->left = t->left;
			thread->right = t->right;
			*p = thread;
			thread->heap_index = index;
			thread = t;
		}
		index <<= 1;
		if (target & (1 << bit)) {
			p = &((*p)->right);
			index |= 1;
		} else {
			p = &((*p)->left);
		}
	}
	thread->heap_index = index;
	*p = thread;
	thread->left = thread->right = NULL;
	return p;
}

/*
 * Delete "thread" from the timeout heap.
 */
static void heap_delete(thread_t *thread)
{
	thread_t *t, **p;
	unsigned int cpu_id = thread->cpu_id;
	int bits = 0;
	int s, bit;

	/* First find and unlink the last heap element */
	p = &get_sleep_queue(cpu_id);
	s = SLEEPQ_SIZE(cpu_id);
	while (s) {
		s >>= 1;
		bits++;
	}
	for (bit = bits - HEAP_INS_DEL_BITS; bit >= 0; bit--) {
		if (SLEEPQ_SIZE(cpu_id) & (1 << bit)) {
			p = &((*p)->right);
		} else {
			p = &((*p)->left);
		}
	}
	t = *p;
	*p = NULL;
	--SLEEPQ_SIZE(cpu_id);
	if (t != thread) {
		/*
		 * Insert the unlinked last element in place of the element we are deleting
		 */
		t->heap_index = thread->heap_index;
		p = heap_insert(t);
		t = *p;
		t->left = thread->left;
		t->right = thread->right;

		/*
		 * Reestablish the heap invariant.
		 */
		for (;;) {
			thread_t *y; /* The younger child */
			int index_tmp;
			if (t->left == NULL)
				break;
			else if (t->right == NULL)
				y = t->left;
			else if (t->left->due < t->right->due)
				y = t->left;
			else
				y = t->right;
			if (t->due > y->due) {
				thread_t *tl = y->left;
				thread_t *tr = y->right;
				*p = y;
				if (y == t->left) {
					y->left = t;
					y->right = t->right;
					p = &y->left;
				} else {
					y->left = t->left;
					y->right = t;
					p = &y->right;
				}
				t->left = tl;
				t->right = tr;
				index_tmp = t->heap_index;
				t->heap_index = y->heap_index;
				y->heap_index = index_tmp;
			} else {
				break;
			}
		}
	}
	thread->left = thread->right = NULL;
}

u64 arch_counter_get_ms(void)
{
	u64 freq, cnt;

	/* Get the timer cnt & freq */
	asm volatile("mrs %0, CNTFRQ_EL0" : "=r" (freq));
	asm volatile("mrs %0, CNTPCT_EL0" : "=r" (cnt));

	return (cnt/freq * MILLISECOND);
}

#if 0
static void timer_wakeup(void)
{
	thread_t *thread;
	unsigned int cpu_id = get_cpu_id();
	u64 now = arch_counter_get_ms();

	while (get_sleep_queue(cpu_id) != NULL) {
		thread = get_sleep_queue(cpu_id);

		if (!(thread->flags & FLAG_ON_SLEEPQ)) {
			tlog("Error: Thread(%s) status  must be SLEEP\n", thread->name);
			continue;
		}
		if (thread->due > now) {
			set_physical_timer_period(thread->due-now);
			break;
		}
		del_sleep_q(thread);

		if (thread->state == STATE_COND_WAIT)
			thread->flags |= FLAG_TIMEDOUT;

		if (thread->flags & FLAG_IDLE_THREAD) {
			tlog("Error: IDLE thread should NOT be in SLEEPQ\n");
			continue;
		}
		smp_spin_lock(&g_sched_lock);
		dbg_print("XXX wakeup thread =%s, due=%llu,state=%d\n", thread->name, thread->due, thread->state);
		add_run_queue(thread);
		smp_spin_unlock(&g_sched_lock);
	}
}
#endif

static void send_ipi(thread_t *new)
{
	unsigned int cpu_id = new->cpu_id;
	thread_t *me = get_current_thread(cpu_id);
	if (init_done[cpu_id] > 0) {
		if (new->priority > me->priority) {
			//dbg_print("Send IPI thread=%s,pri=%d,new=%s,pri=%d, target cpu=%d\n", me->name, me->priority,new->name,new->priority, cpu_id);
			ipi_send_target(cpu_id);
		}
	}
}

static thread_t *allocate_thread(const char *name, void *(*start)(void *arg), void *arg, int stk_size, unsigned int cpu_id, int priority)
{
	thread_t *thread = NULL;
	unsigned char *stack;
	int stack_size;
	unsigned char *data;
	unsigned long irq_flag = 0;

	if (stk_size == 0) {
		stack_size = DEFAULT_STACK_SIZE;
	} else if (stk_size < MIN_STACK_SIZE) {
		stack_size = MIN_STACK_SIZE;
	} else {
		stack_size = stk_size;
	}
	data = (unsigned char *)malloc(sizeof(thread_t) + stack_size + (STACK_BUFFER_SIZE*2));
	if (data == NULL) {
		dbg_print("Failed to allocate thread\n");
		return NULL;
	}
	thread = (thread_t *)data;
	stack = (unsigned char *)(data + sizeof(thread_t) + STACK_BUFFER_SIZE + stack_size);

	memset(thread, 0, sizeof(thread_t));

	thread->reg.sp		= (u64)stack;
	thread->reg.pc		= (u64)_thread_main;
	thread->reg.spsr	= (u64)(CPSR_MODE_SVC | CPSR_F_DISABLE | CPSR_I_DISABLE); //0xd3

	thread->reg.r[0] = (u64)start;
	thread->reg.r[1] = (u64)arg;
	thread->reg.r[2] = (u64)thread;
	thread->start = start;
	thread->cpu_id = cpu_id;
	thread->priority = priority;
	thread->stack_top = (unsigned long)stack;

	if(snprintf(thread->name, MAX_THREAD_NAME, "%s", name) >= MAX_THREAD_NAME)
		printf("tread name recording fail!");

	smp_spin_lock_save(&g_thread_lock, irq_flag);
	ADD_THREADQ(thread);
	active_count++;
	smp_spin_unlock_restore(&g_thread_lock, irq_flag);

	return thread;
}

extern void mhal_tlb_flush(void);
extern void MAsm_CPU_TimerClean(void);

void secondary_start_uboot_cleanup(void)
{
	unsigned long irq_flag = 0;

    smp_spin_lock_save(&g_release_lock, irq_flag);
    disable_interrupts();
    flush_dcache_all();
    smp_spin_unlock_restore(&g_release_lock, irq_flag);
}

// this means the non-boot CPU x have already finished all their jobs!!!
// use this function carefully!!!
void __release_smp_cpu(void)
{
#if 1
	unsigned char cpuid = get_cpu_id();
	struct arm_smccc_res res;

	if (BOOT_CPU == get_cpu_id())
		return;

	while (cpuid < NR_CPUS) {
		if (magic_number_cleaned) {
			printf("cpu:%d %s going to psci_off it self!\n", cpuid, __func__);
			secondary_start_uboot_cleanup();
			if (!smp_cpu_released[cpuid]) {
				smp_cpu_released[cpuid] = 1;
			}
			arm_smccc_smc(ARM_PSCI_0_2_FN_CPU_OFF, 0, 0, 0, 0, 0, 0, 0, &(res));
			printf("%s should not be here\n", __func__);
			break;
		}
	}
#endif
}

static void *sub_init(void *arg)
{
#ifdef THREAD_DEBUG
	unsigned int cpu_id = get_cpu_id();
#endif
	dbg_print("cpu:%d Starting in sub thread main context\n", cpu_id);
	thread_cond_timedwait(&g_sub_cond[get_cpu_id()], NO_TIMEOUT);
	while (get_cpu_id() < NR_CPUS)
		__release_smp_cpu();
	return NULL;
}

int __smp_cpu_parking_check(void)
{
	int all_cpu_ready_for_parking = 1, perCPU;

	for (perCPU = 1; perCPU < NR_CPUS; perCPU++)
		all_cpu_ready_for_parking &= ready_for_parking[perCPU];

	return all_cpu_ready_for_parking;
}

int __smp_cpu_released(void)
{
	int all_cpu_released = 1, perCPU;

	for (perCPU = 1; perCPU < NR_CPUS; perCPU++)
		all_cpu_released &= smp_cpu_released[perCPU];

	return all_cpu_released;
}


static void *_idle_thread_start(void *arg)
{
	unsigned char cpuid = get_cpu_id();

	dbg_print("cpu:%x _idle_thread_start\n", cpuid);
	if (cpuid == 0) {
		printf("CPU 0 should not enter idle state");
		return NULL;
	}

	dbg_print("cpu:%d idle!\n", cpuid);
	while (cpuid < NR_CPUS) {
		if (SMP_parking_start)
			ready_for_parking[cpuid] = 1;

		if (magic_number_cleaned) {
			struct arm_smccc_res res;

			dbg_print("cpu:%d %s going to psci_off it self!\n", cpuid, __func__);
			secondary_start_uboot_cleanup();
			if (!smp_cpu_released[cpuid]) {
				smp_cpu_released[cpuid] = 1;
			}
			arm_smccc_smc(ARM_PSCI_0_2_FN_CPU_OFF, 0, 0, 0, 0, 0, 0, 0, &(res));
			printf("%s should not be here\n", __func__);
			break;
		}
	}
	return NULL;
}

#if 0
static int assign_cpu(thread_t *thread)
{
	int i;
	unsigned int cpu_id = -1;

	if (thread->flags & FLAG_FIX_CPU) {
		return thread->cpu_id;
	}
	for (i = 0; i < NR_CPUS; i++) {
		if (current_thread[i]->flags & FLAG_IDLE_THREAD) {
			cpu_id = i;
			break;
		}
	}

#ifdef SUPPORT_PREEMPTION
	if (cpu_id < 0) {
		for (i = 0; i < NR_CPUS; i++) {
			if (thread->priority > current_thread[i]->priority) {
				cpu_id = i;
				break;
			}
		}
	}
#endif
	if (cpu_id < 0) {
		int rnd = random(NR_CPUS-1);
		cpu_id = rnd;
	}
	return cpu_id;
}
#endif

static thread_t *get_proper_thread(thread_t *current)
{
	thread_t *thread = NULL;
	unsigned int cpu_id = get_cpu_id();

#ifdef SUPPORT_PREEMPTION
	if (get_run_queue(cpu_id).next != &get_run_queue(cpu_id)) {
		thread = get_thread_ptr(get_run_queue(cpu_id).next);
		//dbg_print("XXX scheduler exec running next=%s,pri=%d, current name=%s,pri=%dstate=%x,flag=%x\n", thread->name, thread->priority, current->name, current->priority, current->state, current->flags);
#if 0
		if (!(current->flags & FLAG_PRIMORDIAL) &&
				(current->state == STATE_RUNNING)) {
#else
		if ((current->state == STATE_RUNNING)) {
#endif

#if 0
			//dbg_print("XXX running thread=%s\n", current->name);
			if (current->priority < thread->priority) {
				//dbg_print("Next thread has preemption\n");
				DEL_RUNQ(thread);
			} else
				thread = NULL;
#else
			thread = get_thread_ptr(get_run_queue(cpu_id).next);
			DEL_RUNQ(thread);
#endif
		} else {
			thread = get_thread_ptr(get_run_queue(cpu_id).next);
			//dbg_print("XXX next thread to be executed=%s\n", thread->name);
			DEL_RUNQ(thread);
		}
	} else {
		thread = get_idle_thread(cpu_id);
	}
#else
	if (get_run_queue(cpu_id).next != &get_run_queue(cpu_id)) {
		/* Pull thread off of the run queue */
		thread = get_thread_ptr(get_run_queue().next);
		//dbg_print("XXX scheduler exec running thread =%p\n", thread);
		DEL_RUNQ(thread);
	} else {
		/* If there are no threads to run, switch to the idle thread */
		thread = get_idle_thread(cpu_id);
		//dbg_print("XXX scheduler exec idle=%p\n", thread);
	}
#endif

	return thread;
}

void add_run_queue(thread_t *thread)
{
	unsigned int cpu_id = thread->cpu_id;
#ifdef SUPPORT_PREEMPTION
	thread_t *n;
	list_t *q = get_run_queue(cpu_id).next;

	thread->state = STATE_RUNNABLE;
	for (; q != &get_run_queue(cpu_id); q = q->next) {

		n = get_thread_ptr(q);
		//dbg_print("Scanning priority thread cpu=%d,name = %s,prio=%d,new=%s,prio=%d\n", cpu_id,n->name,n->priority, thread->name, thread->priority);
		if (thread->priority > n->priority) {
			//dbg_print("Higher priority thread name = %s,prio=%d,new=%s,prio=%d\n",n->name,n->priority, thread->name, thread->priority);
			INSERT_BEFORE(&thread->links, q);
			send_ipi(thread);
			return;
		}
	}

	if (q == &get_run_queue(cpu_id)) {
		dbg_print("Added new thread=%s, cpu=%d\n", thread->name, cpu_id);
		ADD_RUNQ(thread, cpu_id);
		send_ipi(thread);
	}
#else
	thread->state = STATE_RUNNABLE;
	ADD_RUNQ(thread, cpu_id);
	ipi_send_target(cpu_id);
#endif
}

#define MSEC_TO_USEC (1000)
#define SECONDARY_CORE_TIMEOUT_MS (5000)
void release_non_boot_core(void)
{
	unsigned int wait_cnt = 0;
	unsigned int perCPU;
	u8  runq_check = 0;

	while (1) {
		for (perCPU = 1; perCPU < NR_CPUS; perCPU++) {
			if (get_run_queue(perCPU).next == &get_run_queue(perCPU)) {
				//printf("runQ check done!  CPU=%d , me=%s,NEXT=%s\n",perCPU, cur_thread->name,nxt_thread->name);
				runq_check++;
			}
		}

		if (runq_check == (NR_CPUS-1)) {
			dbg_print("runQ are all empty!\n");
			break;
		}
		runq_check = 0;
		udelay(MSEC_TO_USEC);
		wait_cnt++;
	}

	dbg_print("wait_cnt:%d\n", wait_cnt);
	wait_cnt = 0;
	SMP_parking_start = 1;
	while (!__smp_cpu_parking_check()) {
		++wait_cnt;
		udelay(MSEC_TO_USEC);

		if (wait_cnt > SECONDARY_CORE_TIMEOUT_MS) {
			for (perCPU = 1; perCPU < NR_CPUS; perCPU++) {
				if (!ready_for_parking[perCPU])
					printf("smp: cpu%d is stuck not finishing its work !\n", perCPU);
			}
			do_reset(NULL, 0, 0, NULL);  //just in case, should never be here!
		}
	}

	flush_dcache_all();
	magic_number_cleaned = 1;

	dbg_print("%s waiting for secondary cores finish\n", __func__);
	wait_cnt = 0;
	while (!__smp_cpu_released()) {
		++wait_cnt;
		udelay(MSEC_TO_USEC);

		if (wait_cnt > SECONDARY_CORE_TIMEOUT_MS) {
			for (perCPU = 1; perCPU < NR_CPUS; perCPU++) {
				if (!smp_cpu_released[perCPU])
					printf("smp: cpu_not_released_%d !\n", perCPU);
			}
			do_reset(NULL, 0, 0, NULL);  //just in case, should never be here!
		}
	}
	dbg_print("%s %d all cpu released!!\n", __func__, __LINE__);
}

void reschedule_irq(void)
{
	int cpuid = get_cpu_id();
	g_scheduler.need_schedule[cpuid] = 0;
	dbg_print("cpu:%d re-sch\n", cpuid);
	schedule();
#ifdef THREAD_DEBUG
    thread_t *cur_th = get_current_thread(cpuid);
#endif
    dbg_print("cpu:%d name:%s exit reschedule\n", cpuid, cur_th->name);
}

int schedule(void)
{
	thread_t *thread;
	unsigned long irq_flag = 0;
	smp_spin_lock_save(&g_sched_lock, irq_flag);
	unsigned int cpu_id = get_cpu_id();
	dbg_print("core:%x schedule\n", cpu_id);

	thread_t *me = get_current_thread(cpu_id);

	thread = get_proper_thread(me);

/*
	if (me->prev) {
		free(me->prev);
		me->prev = NULL;
	}
*/

	if (thread == NULL) {
		dbg_print("Next is NULL\n");
	} else if (me == thread) {
		dbg_print("Current is same with next=%s\n", thread->name);
		thread->state = STATE_RUNNING;
	} else {
		if (me->state == STATE_RUNNING) {
			dbg_print("This (%s) is still running ,added it into runQ\n", me->name);
			add_run_queue(me);
		}
		if (thread->state != STATE_IDLE)
			if (thread->state != STATE_RUNNABLE) {
				tlog("Error: Thread(%s) status must be RUNNABLE\n", me->name);
			}
		/* Resume the thread */
		if (!(thread->flags & FLAG_IDLE_THREAD))
			thread->state = STATE_RUNNING;
		set_current_thread(thread, cpu_id);

		//dbg_print("CPU=%d,CURRENT daif=%x , me=%s,pc=%x,spsr=%x,NEXT=%s,pc=%x,spsr=%x\n", cpu_id,get_DAIF(), me->name, me->reg.pc, me->reg.spsr, thread->name, thread->reg.pc, thread->reg.spsr);
		dbg_print("CPU=%d in schedule, me=%s,NEXT=%s\n", cpu_id, me->name, thread->name);
		if ((me->state == STATE_DONE) &&
				!(me->flags & FLAG_PRIMORDIAL)) {
			dbg_print(" thread %s to be freed\n", me->name);
			//thread->prev = me;
			me = NULL;
		}

		dbg_print("%d CPU=%d in schedule, me=%s,NEXT=%s time:%lu\n", __LINE__, cpu_id, me->name, thread->name, get_timer(0));
		__switch_context(me, thread);

		if ((thread->state == STATE_DONE) &&
				!(thread->flags & FLAG_PRIMORDIAL)) {
			// remove to avoid double free
			//free(thread);
		}
	}

	//dbg_print("core:%x thead:%s prepare exiting schedule\n", cpu_id, thread->name);
	smp_spin_unlock_restore(&g_sched_lock, irq_flag);
	dbg_print("core:%x thead:%s DID exit schedule\n", cpu_id, thread->name);
	return 0;
}

thread_t *thread_init_per_cpu(void)
{
	thread_t *thread;
	unsigned int cpu_id = get_cpu_id();
	char tmp[MAX_THREAD_NAME+1];

	if (init_done[cpu_id] > 0) {
		dbg_print("XXX already init\n");
		return NULL;
	}
	dbg_print("%s cpu:%d\n", __func__, cpu_id);

	INIT_LIST(&get_run_queue(cpu_id));
	//INIT_LIST(&get_zombie_queue(cpu_id));
	/*
	 * Create idle thread
	 */
	if(snprintf(tmp, sizeof(tmp) - 1, "idle[%d]", cpu_id) >= sizeof(tmp))
		printf("%s %d snprintf fail!\n", __func__, __LINE__);

	g_scheduler.idle_thread[cpu_id] = thread_create_ex(tmp, _idle_thread_start, NULL, MIN_STACK_SIZE, cpu_id, THREAD_MIN_PRIORITY, 0);
	if (!get_idle_thread(cpu_id)) {
		printf("cpu:%d create idle thread fail!\n", cpu_id);
		return NULL;
	}
	get_idle_thread(cpu_id)->flags = FLAG_IDLE_THREAD;
	get_idle_thread(cpu_id)->state = STATE_IDLE;
	active_count--;
	DEL_RUNQ(get_idle_thread(cpu_id));

	if(snprintf(tmp, sizeof(tmp) - 1, "sub_init[%d]", cpu_id) >= sizeof(tmp))
		printf("%s %d snprintf fail!\n", __func__, __LINE__);

	thread = thread_create_ex(tmp, sub_init, NULL, 0, cpu_id, THREAD_DEFAULT_PRIORITY - 1, 0);
	if (!thread) {
		printf("%s cpu:%d thread_create_ex fail!\n", __func__, cpu_id);
		return NULL;
	}

	thread->state = STATE_RUNNING;
	thread->flags = FLAG_PRIMORDIAL;
	set_current_thread(thread, cpu_id);
	DEL_RUNQ(thread);
	thread_cond_new(&g_sub_cond[cpu_id]);
	//dbg_print("thread per cpu init done, current cpu=%d\n", cpu_id);

	init_done[cpu_id] = 1;
	return thread;
}

void smp_thread(void)
{
	thread_t *thread = thread_init_per_cpu();
	asm volatile (
				"mov    x5, %0\n\t"
				"ldr	x4 ,[%0]\n\t"
				"mov	sp ,x4\n\t"
				"ldr	x4,[%0, #16]\n\t"
				"ldp	x0, x1, [x5, #32]\n\t"
				"ldr	x2, [x5, #48]\n\t"
				"blr	x4\n\t"
				: : "r"(thread)
				: "memory"
				);

	//if (thread)
		//_thread_main(thread->start,NULL, thread);
}

thread_t *thread_init(void)
{
	thread_t *thread;
	unsigned int cpu_id = get_cpu_id();
	int perCPU = 0;
	char tmp[MAX_THREAD_NAME+1];

	if (init_done[cpu_id] > 0) {
		dbg_print("XXX already init\n");
		return 0;
	}

	memset(&g_scheduler, 0, sizeof(scheduler_t));

	INIT_LIST(&get_run_queue(cpu_id));
	//INIT_LIST(&get_zombie_queue(cpu_id));
	INIT_LIST(&get_thread_queue());

	/*
	 * Create idle thread
	 */

	if(snprintf(tmp, sizeof(tmp)-1, "idle[%d]", cpu_id) >= sizeof(tmp))
		printf("%s %d snprintf fail!\n", __func__, __LINE__);

	g_scheduler.idle_thread[cpu_id] = thread_create_ex("idle", _idle_thread_start, NULL, MIN_STACK_SIZE, 0, THREAD_IDLE_PRIORITY, 0);
	if (!get_idle_thread(cpu_id)) {
		printf("cpu:%d create idle thread fail!\n", cpu_id);
		return NULL;
	}
	get_idle_thread(cpu_id)->flags = FLAG_IDLE_THREAD;
	get_idle_thread(cpu_id)->state = STATE_IDLE;
	active_count--;
	DEL_RUNQ(get_idle_thread(cpu_id));

	thread = thread_create_ex("init", main_init, NULL, 0, 0, THREAD_DEFAULT_PRIORITY - 1, 0);
	if (!thread) {
		printf("%s cpu:%d thread_create_ex fail!\n", __func__, cpu_id);
		return NULL;
	}

	thread->state = STATE_RUNNING;
	thread->flags = FLAG_PRIMORDIAL;
	set_current_thread(thread, cpu_id);
	DEL_RUNQ(thread);

#ifdef ENABLE_SMP
	for (perCPU = 1; perCPU < NR_CPUS; perCPU++)
		Core_Wakeup(smp_thread, perCPU);
#endif
	//dbg_print("thread init done, current cpu=%d\n",get_current_cpu());
	init_done[cpu_id] = 1;
    //thread_info_dump(thread);
	return thread;
}

void thread_start(void)
{
	thread_t *thread = thread_init();
	asm volatile (
				"mov    x5, %0\n\t"
				"ldr	x4 ,[%0]\n\t"
				"mov	sp ,x4\n\t"
				"ldr	x4,[%0, #16]\n\t"
				"ldp	x0, x1, [x5, #32]\n\t"
				"ldr	x2, [x5, #48]\n\t"
				"blr	x4\n\t"
				: : "r"(thread)
				: "memory"
				);
	//(thread)
	//hread_main(thread->start,NULL, thread);
}

thread_t *thread_create_ex(const char *name, void *(*start)(void *arg), void *arg, int stk_size, unsigned int cpu_id, int priority, int join)
{
	unsigned long irq_flag = 0;
	thread_t *thread = allocate_thread(name, start, arg, stk_size, cpu_id, priority);

	if (!thread) {
		printf("tread allocate fail!\n");
		return NULL;
	}

	if (join) {
		cond_t *term = calloc(1, sizeof(cond_t));
		if (term) {
			thread_cond_new(term);
			thread->term = term;
		} else {
			printf("term calloc fail!");
			if (thread)
				free(thread);
			return NULL;
		}
	}

	dbg_print("%s cpu:%d thread:%s\n", __func__, thread->cpu_id, thread->name);
	smp_spin_lock_save(&g_sched_lock, irq_flag);
	add_run_queue(thread);
	smp_spin_unlock_restore(&g_sched_lock, irq_flag);

	return thread;
}

thread_t *thread_create(const char *name, void *(*start)(void *arg), void *arg)
{
	unsigned long irq_flag = 0;
	thread_t *thread;
	unsigned int cpu_id = get_cpu_id();
	thread = allocate_thread(name, start, arg, DEFAULT_STACK_SIZE, cpu_id, THREAD_DEFAULT_PRIORITY);

	if (!thread) {
		printf("tread allocate fail!\n");
		return NULL;
	}

	smp_spin_lock_save(&g_sched_lock, irq_flag);
	add_run_queue(thread);
	smp_spin_unlock_restore(&g_sched_lock, irq_flag);

	return thread;
}

void thread_exit(void *retval)
{
	unsigned int cpu_id = get_cpu_id();
	thread_t *thread = get_current_thread(cpu_id);
	unsigned long irq_flag = 0;

	dbg_print("thread->name:%s in thread_exit\n", thread->name);

	thread->retval = retval;
	active_count--;
	if (thread->term) {
		thread->state = STATE_ZOMBIE;
		//ADD_ZOMBIEQ(thread, cpu_id);
		dbg_print("thread :%s have term!\n", thread->name);
		while (!(thread->flags & FLAG_JOINED)) {
		thread_cond_signal(thread->term);
		schedule();
		}

		dbg_print("thread :%s going to free term!\n", thread->name);
		free(thread->term);
		thread->term = NULL;
	}

	smp_spin_lock_save(&g_thread_lock, irq_flag);
	dbg_print("thread :%s going to delete !\n", thread->name);
	DEL_THREADQ(thread);
	smp_spin_unlock_restore(&g_thread_lock, irq_flag);

	if (!(thread->flags & FLAG_ON_SLEEPQ)) {
		thread->state = STATE_DONE;
	}

	dbg_print("thread :%s end and to sched next !\n", thread->name);
	schedule();
}

int thread_join(thread_t *thread, void **retvalp)
{
	unsigned int cpu_id = get_cpu_id();
	unsigned long irq_flag = 0;
	if (thread->term == NULL) {
		return -EINVAL;
	}
	if (get_current_thread(cpu_id) == thread) {
		return -EDEADLK;
	}

	if (thread->state == STATE_ZOMBIE) {
		goto EXIT;
	}
	dbg_print("cpu:%d is waiting for thread:%s join!, its cvar->lock:%p\n", cpu_id, thread->name, &(thread->term->lock));
	while (thread->state != STATE_ZOMBIE) {
		if (thread_cond_timedwait(thread->term, NO_TIMEOUT) != 0)
			return -1;
	}

	if (retvalp)
		*retvalp = thread->retval;

	/*
	 * Remove target thread from the zombie queue and make it runnable.
	 * When it gets scheduled later, it will do the clean up.
	 */
EXIT:
	//DEL_ZOMBIEQ(thread);
	smp_spin_lock_save(&g_sched_lock, irq_flag);
	thread->flags |= FLAG_JOINED;
	add_run_queue(thread);
	smp_spin_unlock_restore(&g_sched_lock, irq_flag);
	return 0;
}

void add_sleep_q(thread_t *thread, utime_t timeout)
{
	thread_t *root;
	u32 now = (u32)arch_counter_get_ms();
	thread->due = now + timeout;
	//dbg_print("XXX sleepQ name =%s, timeout=%d,due=%d, cpu=%d\n", thread->name, timeout, thread->due, thread->cpu_id);
	thread->flags |= FLAG_ON_SLEEPQ;
	thread->heap_index = ++SLEEPQ_SIZE(thread->cpu_id);
	heap_insert(thread);

	root = get_sleep_queue(thread->cpu_id);
	if (root) {
		int gap = root->due-now;
		if (gap <= 0)
			set_physical_timer_period(1);
		else
			set_physical_timer_period(gap);
	}
}

void del_sleep_q(thread_t *thread)
{
	heap_delete(thread);
	thread->flags &= ~FLAG_ON_SLEEPQ;
}

int thread_msleep(unsigned int msecs)
{
	unsigned int cpu_id = get_cpu_id();
	thread_t *me = get_current_thread(cpu_id);

	if (msecs == 0)
		return 0;

	if (!init_done[cpu_id] || !is_irq_on()) {
		udelay(msecs*MILLISECOND);
		return -EINTR;
	}

	me->flags |= FLAG_FIX_CPU;
	irq_disable();
	if (msecs != NO_TIMEOUT) {
		me->state = STATE_SLEEPING;
		add_sleep_q(me, msecs);
	} else
		me->state = STATE_SUSPENDED;
	irq_enable();

	schedule();
	return 0;
}

int need_to_reschedule(void)
{
	unsigned int cpu_id = get_cpu_id();
	//if (g_scheduler.need_schedule[0] > 0)
		//dbg_print("XXX need to reschedule=%d\n",g_scheduler.need_schedule[0]);
	return (get_run_queue(cpu_id).next != &get_run_queue(cpu_id));
}

int thread_sleep(int secs)
{
	return thread_msleep((secs >= 0)?secs*(utime_t)MILLISECOND:NO_TIMEOUT);
}
