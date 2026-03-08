#include <common.h>
#include <command.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <exports.h>
#include <net.h>
#include <usb.h>
#include <xyzModem.h>
#include <environment.h>
#include <mmc.h>
#include <exports.h>
#include <config.h>
#include <thread.h>
#include <spinlock.h>

#if SMP_USING_MUTEX
static mutex_t mlock;
#endif
static cond_t cond;
static int nosleep;
#ifdef THREAD_DEBUG
static u32 mRes;
#endif

extern u64 randomfortest(u64 limit);
extern u64 arch_counter_get_ms(void);
extern u64 get_physical_timer_cnt(void);
extern void board_init_r_multi_core(void);
extern void ipi_broadcast(void);

void *main_init(void *arg)
{
#ifdef THREAD_DEBUG
	unsigned char cpuid = get_cpu_id();
#endif
	dbg_print("core %d main_init\n", cpuid);
	dbg_print("do board_init_r_multi_core\n");
	board_init_r_multi_core();
	return NULL;
}

#define CRITICAL_SECTION_SIZE 100
#define LOCK_TEST_TIMES 100000
#define DELAY_1US 1
#define LOCK_TEST_TIMEOUT_US 10000000
unsigned int critical_section[CRITICAL_SECTION_SIZE] = {0};
static smp_spin_lock_t processing_lock = INIT_SPIN_LOCK;

void *lock_test(void *arg)
{
	unsigned int i = 0, j = 0;
	unsigned int cpuid = get_cpu_id();
	unsigned long test_timeout = 0;
#ifdef CONFIG_MULTICORES_PLATFORM
	unsigned long irq_flag = 0;
#endif

	tlog ("lock test on CPU=%d\n", cpuid);

	while (test_timeout < LOCK_TEST_TIMEOUT_US) {
		#ifdef CONFIG_MULTICORES_PLATFORM
		smp_spin_lock_save(&processing_lock, irq_flag);
		#endif

		for (j = 0; j < CRITICAL_SECTION_SIZE; ++j)
			critical_section[j] = cpuid;

		for (j = 0; j < CRITICAL_SECTION_SIZE; ++j) {
			if (critical_section[j] != cpuid) {
					tlog("[%d]RACE condition!!!", cpuid);
			}
		}
		++i;
		if ((i % (LOCK_TEST_TIMES - 1)) == 0) {
			tlog("THTEAD SAFE CPU:(%d)....\n", get_cpu_id());
			i = 0;
		}

		udelay(DELAY_1US);
		test_timeout++;
		#ifdef CONFIG_MULTICORES_PLATFORM
		smp_spin_unlock_restore(&processing_lock, irq_flag);
		#endif
	}
	return NULL;
}


#define PATTERN_TO_WRITE 7
#define MEM_TEST_TIMES 100000
static void *mem_test(void *arg)
{
	unsigned int i = 0;
	const unsigned int HEAP_SIZE = (128);

	for (;;) {
		char *p = malloc(HEAP_SIZE);

		if (p) {
			unsigned int walk = 0;

			++i;
			memset(p, PATTERN_TO_WRITE + get_cpu_id(), HEAP_SIZE);

			for (; walk < (HEAP_SIZE / sizeof(char)); ++walk) {
				if (*(p + walk) != (char)(PATTERN_TO_WRITE + get_cpu_id()))
					tlog("!!!!!!! DANGEROUS[%d] core:%d address: 0x%p is 0x%x (it should be 0x%x)!!!!!!!\n", i, get_cpu_id(), (p + walk), *(p + walk), PATTERN_TO_WRITE + get_cpu_id());
			}

			if ((i % (MEM_TEST_TIMES - 1)) == 0) {
				tlog("cpu:%d cnt:%d PA:[0x%p]~[0x%p]\n", get_cpu_id(), i, p, p+HEAP_SIZE);
				i = 0;
			}

			memset(p, 0, HEAP_SIZE);
			free(p);
			p = NULL;
		} else {
			tlog("malloc fail!!! count=%d\n", i);
			return 0;
		}
	}
	tlog("mem test done [round-44]\n");

	return 0;
}
static thread_t *join2;
static void *join_test(void *arg)
{
	unsigned int cpu_id = get_cpu_id();
	thread_t *me = get_current_thread(cpu_id);
	me->start_sleep = arch_counter_get_ms();
	//thread_join(join2,(void **)&ret);
#ifdef THREAD_DEBUG
	u32 wakeup_time = arch_counter_get_ms() - me->start_sleep;
#endif
	tlog("Join Test thread=%s,Time gap=%d,resource=%d\n", me->name, wakeup_time, mRes);

	return NULL;
}

#define JOIN_TEST_SLEEP_SEC 5
static void *join_test2(void *arg)
{
	thread_t *me = get_current_thread(get_cpu_id());
	me->start_sleep = arch_counter_get_ms();
	thread_sleep(JOIN_TEST_SLEEP_SEC);
#ifdef THREAD_DEBUG
	u32 wakeup_time = arch_counter_get_ms() - me->start_sleep;
#endif
	tlog("Join Test thread=%s,Time gap=%d,resource=%d\n", me->name, wakeup_time, mRes);

	return NULL;
}

#define COND_TEST_TIMEOUT 3
static void *cond_test(void *arg)
{
	thread_t *me = get_current_thread(get_cpu_id());
	me->start_sleep = arch_counter_get_ms();
	thread_cond_timedwait(&cond, COND_TEST_TIMEOUT);
#ifdef THREAD_DEBUG
	u32 wakeup_time = arch_counter_get_ms() - me->start_sleep;
#endif
	tlog("Condition Test thread=%s,Time gap=%d,resource=%d\n", me->name, wakeup_time, mRes);

	return NULL;
}

#define COND_TEST2_TIMEOUT 1
static void *cond_test2(void *arg)
{
	thread_t *me = get_current_thread(get_cpu_id());
	me->start_sleep = arch_counter_get_ms();
	thread_sleep(COND_TEST2_TIMEOUT);
	thread_cond_signal(&cond);
#ifdef THREAD_DEBUG
	u32 wakeup_time = arch_counter_get_ms() - me->start_sleep;
#endif
	tlog("Condition Test thread=%s,Time gap=%d,resource=%d\n", me->name, wakeup_time, mRes);

	return NULL;
}

#if SMP_USING_MUTEX

#define MUTEX_TEST_TIMEOUT 2
static void *mutex_test(void *arg)
{
	thread_t *me = get_current_thread(get_cpu_id());
	me->start_sleep = arch_counter_get_ms();
#if SMP_USING_MUTEX
	mutex_lock(&mlock);
#endif
	thread_sleep(MUTEX_TEST_TIMEOUT);
	mRes++;
#if SMP_USING_MUTEX
	mutex_unlock(&mlock);
#endif
#ifdef THREAD_DEBUG
	u32 wakeup_time = arch_counter_get_ms() - me->start_sleep;
#endif
	tlog("Mutex Test thread=%s,Time gap=%d,resource=%d\n", me->name, wakeup_time, mRes);

	return NULL;
}

#define MUTEX_TEST2_TIMES 1000000
static void *mutex_test2(void *arg)
{
	thread_t *me = get_current_thread(get_cpu_id());
	int i;
	me->start_sleep = arch_counter_get_ms();
#if SMP_USING_MUTEX
	mutex_lock(&mlock);
#endif
	for (i = 0; i < MUTEX_TEST2_TIMES; i++) {
		randomfortest(MUTEX_TEST2_TIMES);
	}
	mRes++;
#if SMP_USING_MUTEX
	mutex_unlock(&mlock);
#endif
#ifdef THREAD_DEBUG
	u32 wakeup_time = arch_counter_get_ms() - me->start_sleep;
#endif
	tlog("Mutex Test thread=%s,Time gap=%d,resource=%d\n", me->name, wakeup_time, mRes);

	return NULL;
}
#endif

#define THREAD_TEST_SLEEP_RANGE 9
#define THREAD_TEST_TIMEOUT_US 10000000

static void *thread_test(void *arg)
{
	unsigned int cpu_id = get_cpu_id();
	thread_t *me = get_current_thread(cpu_id);
	unsigned long test_timeout = 0;
#ifdef THREAD_DEBUG
	u32 wakeup_time = 0;
#endif
	int rnd = 0;
	if (nosleep) {
		me->start_sleep = arch_counter_get_ms();
#ifdef THREAD_DEBUG
		wakeup_time = arch_counter_get_ms() - me->start_sleep;
#endif
		tlog("Test thread=%s sleep=%d,executing,daif=%x,cpu=%d,Time gap=%d\n", me->name, rnd, get_DAIF(), get_cpu_id(), wakeup_time);

	} else {
		while (test_timeout < THREAD_TEST_TIMEOUT_US) {
			rnd = randomfortest(THREAD_TEST_SLEEP_RANGE);
			me->start_sleep = arch_counter_get_ms();
			thread_sleep(rnd);
#ifdef THREAD_DEBUG
			wakeup_time = arch_counter_get_ms() - me->start_sleep;
#endif
			tlog("Test thread=%s sleep=%d,executing,daif=%x,cpu=%d,Time gap=%d\n", me->name, rnd, get_DAIF(), get_cpu_id(), wakeup_time);
			test_timeout++;
			udelay(DELAY_1US);
		}
	}

	return NULL;
}

#define CREATE_THREAD_TEST_NUM 20
void create_thread_test(void)
{
	int i;
	char name[MAX_THREAD_NAME + 1];
	int pri;

	thread_t *t;
	unsigned int cpu_id = get_cpu_id();
	for (i = 0; i < CREATE_THREAD_TEST_NUM; i++) {
		if (snprintf(name, MAX_THREAD_NAME, "test-%d", i) >= MAX_THREAD_NAME)
			printf("thread name recording fail");
		//pri = randomfortest(THREAD_MAX_PRIORITY-THREAD_MIN_PRIORITY);
		pri = THREAD_DEFAULT_PRIORITY;
		cpu_id = (cpu_id+1) % NR_CPUS;
		dbg_print("Creating test thread=%s,pri=%d,cpu=%d\n", name, pri, cpu_id);
		t = thread_create_ex(name, thread_test, NULL, 0, cpu_id, pri, 0);
		if (t == NULL)
			break;
	}
}

#define THREAD_STATUS_MAX 8
void display_all_threads(void)
{
	thread_t *thread;
	list_t *q = get_thread_queue().next;
	int i = 0;
	tlog("## Display all threads(%d)\n", active_count);
	for (; q != &get_thread_queue(); q = q->next) {

		thread = get_thread_ptr_threadq(q);
		if (thread->state > THREAD_STATUS_MAX)
			printf("%d : thread(%p) name=%s,priority=%d,state=unknown,cpu=%d\n", i++, thread, thread->name, thread->priority, thread->cpu_id);
		else
			printf("%d : thread(%p) name=%s,priority=%d,state=%s,cpu=%d\n", i++, thread, thread->name, thread->priority, thread_state_str[thread->state], thread->cpu_id);
	}
}

#define SPIN_LOCK_TEST_LOOP_CNT 1000000
#define THREAD_CMD_TOTAL_ARGS   2
#define THREAD_CMD_ARGS_0       0
#define THREAD_CMD_ARGS_1       1
#define THREAD_CMD_ARGS_2       2
int do_thread_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < THREAD_CMD_TOTAL_ARGS) {
		tlog ("Usage:\n%s\n", cmdtp->usage);
		tlog ("%s\n", cmdtp->help);
		return 1;
	}

	if (!strcmp(argv[THREAD_CMD_ARGS_1], "create")) {
		if (argc > THREAD_CMD_TOTAL_ARGS)  {
			if (!strcmp(argv[THREAD_CMD_ARGS_2], "nosleep")) {
				nosleep = 1;
				create_thread_test();
			}
		} else {
			nosleep = 0;
			create_thread_test();
		}
	} else if (!strcmp(argv[THREAD_CMD_ARGS_1], "sleep")) {
		int rnd = randomfortest(THREAD_TEST_SLEEP_RANGE);
		tlog("XXX thread_sleep(%d)\n", rnd);
		thread_sleep(rnd);
	} else if (!strcmp(argv[THREAD_CMD_ARGS_1], "spinlock")) {
		int  x = 0;
		smp_spin_lock_t lock;
		unsigned long irq_flag = 0;
		smp_spin_lock_init(&lock);
		smp_spin_lock_save(&lock, irq_flag);
		while (x < SPIN_LOCK_TEST_LOOP_CNT) {
			x++;
		}
		smp_spin_trylock(&lock);
		tlog("XXX 2nd trylcok\n");
		smp_spin_unlock_restore(&lock, irq_flag);

	} else if (!strcmp(argv[THREAD_CMD_ARGS_1], "display")) {
		display_all_threads();
#if SMP_USING_MUTEX
	} else if (!strcmp(argv[THREAD_CMD_ARGS_1], "mutex")) {
		mutex_new(&mlock);
		thread_create("mutex", mutex_test, NULL);
		thread_create("mutex2", mutex_test2, NULL);
#endif
	} else if (!strcmp(argv[THREAD_CMD_ARGS_1], "cond")) {
		thread_cond_new(&cond);
		thread_create("cond", cond_test, NULL);
		thread_create("cond2", cond_test2, NULL);
	} else if (!strcmp(argv[THREAD_CMD_ARGS_1], "join")) {
		join2 = thread_create("join2", join_test2, NULL);
		thread_t *join = thread_create_ex("join", join_test, NULL, 0, 1, THREAD_DEFAULT_PRIORITY, 1);
		if (!join) {
			tlog("fail to create join thread!\n");
		} else {
			thread_sleep(JOIN_TEST_SLEEP_SEC);
			thread_join(join, NULL);
		}
	} else if (!strcmp(argv[THREAD_CMD_ARGS_1], "broad")) {
		ipi_broadcast();
	} else if (!strcmp(argv[THREAD_CMD_ARGS_1], "target")) {
		int cpu = simple_strtoul(argv[THREAD_CMD_ARGS_2], NULL, 0);
		ipi_send_target(cpu);
	} else if (!strcmp(argv[THREAD_CMD_ARGS_1], "mem")) {
		tlog("start mem test\n");
		thread_create_ex("mem_1", mem_test, NULL, 0, 1, THREAD_DEFAULT_PRIORITY, 0);
		//thread_create_ex("mem_2",mem_test, NULL,0,2,THREAD_DEFAULT_PRIORITY,0);
		//thread_create_ex("mem_3",mem_test, NULL,0,3,THREAD_DEFAULT_PRIORITY,0);
		mem_test(NULL);
	} else if (!strcmp(argv[THREAD_CMD_ARGS_1], "lock")) {
		tlog("spin lock test\n");
		thread_create_ex("mem_1", lock_test, NULL, 0, 1, THREAD_DEFAULT_PRIORITY, 0);
		//thread_create_ex("mem_2",lock_test, NULL,0,2,THREAD_DEFAULT_PRIORITY,0);
		//thread_create_ex("mem_3",lock_test, NULL,0,3,THREAD_DEFAULT_PRIORITY,0);
		lock_test(NULL);
    } else if (!strcmp(argv[THREAD_CMD_ARGS_1], "mapping")) {

	} else {
		tlog("Unknown command for thread\n");
	}

	return 0;
}

#define RANDOM_GLIBC_CONST1 51284
#define RANDOM_GLIBC_CONST2 1103515245
#define RANDOM_GLIBC_CONST3 12345
#define RANDOM_GLIBC_64K    65536
#define RANDOM_GLIBC_1024    1024
#define RANDOM_GLIBC_2048    2048
#define RANDOM_GLIBC_LSH_BIT 10
u64 randomfortest(u64 limit)
{
	static u64 seed;
	u64 next, res;

	if (seed == 0)
		seed = (get_physical_timer_cnt() + RANDOM_GLIBC_CONST1);
	next = seed;

	next *= RANDOM_GLIBC_CONST2;
	next += RANDOM_GLIBC_CONST3;
	res = (unsigned int) (next / RANDOM_GLIBC_64K) % RANDOM_GLIBC_2048;

	next *= RANDOM_GLIBC_CONST2;
	next += RANDOM_GLIBC_CONST3;
	res <<= RANDOM_GLIBC_LSH_BIT;
	res ^= (unsigned int) (next / RANDOM_GLIBC_64K) % RANDOM_GLIBC_1024;

	next *= RANDOM_GLIBC_CONST2;
	next += RANDOM_GLIBC_CONST3;
	res <<= RANDOM_GLIBC_LSH_BIT;
	res ^= (unsigned int) (next / RANDOM_GLIBC_64K) % RANDOM_GLIBC_1024;

	seed = next;

	return res % limit;
}

#define THREAD_CMD_MAXARGS 4
U_BOOT_CMD(
	thread,	  THREAD_CMD_MAXARGS,	  0,	  do_thread_test,
	"thread create\n",
	"thread display \n"
);
