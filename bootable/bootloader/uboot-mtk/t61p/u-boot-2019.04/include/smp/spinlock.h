#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <asm/system.h>

typedef struct {
	volatile u32 lock;
} smp_spin_lock_t;

#define UNLOCK 0
#define LOCK 1

/* Current Program Status Register(CPSR) */
#define CPSR_N_FLAG				(0x01 << 31)
#define CPSR_Z_FLAG				(0x01 << 30)
#define CPSR_C_FLAG				(0x01 << 29)
#define CPSR_V_FLAG				(0x01 << 28)
#define CPSR_Q_FLAG				(0x01 << 27)
#define CPSR_IT_1_0				(0x03 << 25)
#define CPSR_JAZELLE			(0x01 << 24)
/* [23:20] RAZ/SBZP		*/
#define CPSR_GE					(0x01 << 16)
#define CPSR_IT_7_2				(0x3F << 10)
#define CPSR_E_STATE			(0x01 << 9)		/* Endianness execution state bit. 0:Little, 1:Big */
#define CPSR_A_DISABLE			(0x01 << 8)		/* Asynchronous abort disable bit */
#define CPSR_I_DISABLE			(0x01 << 7)		/* Interrupt disable bit */
#define CPSR_F_DISABLE			(0x01 << 6)		/* Fast interrupt diable bit */
#define CPSR_T_STATE			(0x01 << 5)		/* Thumb execution state bit */
#define CPSR_MODE				(0x1F << 0)		/* The current mode of the processor */

#define CPSR_MODE_USER			0x10
#define CPSR_MODE_FIQ			0x11
#define CPSR_MODE_IRQ			0x12
#define CPSR_MODE_SVC			0x13
#define CPSR_MODE_ABT			0x17
#define CPSR_MODE_UND			0x1B
#define CPSR_MODE_SYS			0x1F

#define DAIF_IRQ (0x01 << 7)
#define DAIF_FIQ (0x01 << 6)

/* Interrupt related define*/
#define SGI_IRM_BIT 40
#define HCR_EL2_IMO_BIT 4

#define LSBYTE 0xFF

#define INIT_SPIN_LOCK		{.lock = UNLOCK}

static inline void write_thread_id(unsigned int id)
{
	asm volatile("msr TPIDR_EL2, %0" :: "r" (id));
}

static inline unsigned int get_thread_id(void)
{
	unsigned int id;
	asm volatile("mrs %0, TPIDR_EL2" : "=r" (id));
	return id;
}

unsigned int plat_arm_calc_core_pos(unsigned int);

static inline unsigned int get_cpu_id(void)
{
	unsigned long mpidr = read_mpidr();
	return plat_arm_calc_core_pos(mpidr);
}


static inline unsigned int get_DAIF(void)
{
	unsigned int old;

	asm volatile (
			"   mrs     %0, DAIF\n"
			: "=r"(old) : );

	return old;
}

static inline int is_irq_on(void)
{
	if (get_DAIF() & DAIF_IRQ)
		return 0;
	return 1;
}
static inline unsigned int save_irq(void)
{
	unsigned int flag;
	local_irq_save(flag);
	return flag;
}
static inline void restore_irq(unsigned int flag)
{
	local_irq_restore(flag);
}
static inline void irq_enable(void)
{
	asm volatile("msr daifclr, #3");
}
static inline void irq_disable(void)
{
	asm volatile("msr daifset, #3");
}
#if defined(CONFIG_MULTICORES_PLATFORM)
/* Below functions should be defined in arch */
extern volatile u32 SMP_init_done;
extern volatile unsigned int spin_lock_count[NR_CPUS];
#define SMP_INIT_MAGIC (0x534D50) //"SMP"
void __cpu_spin_lock(void *lock);
int __cpu_spin_trylock(void *lock);
void __cpu_spin_unlock(void *lock);

static inline void smp_spin_lock_init(smp_spin_lock_t *lock)
{
	lock->lock = UNLOCK;
}

extern smp_spin_lock_t g_sched_lock;

static inline void smp_spin_lock(smp_spin_lock_t *lock)
{
	if (SMP_init_done == SMP_INIT_MAGIC) {
		__cpu_spin_lock((void *)&lock->lock);
		spin_lock_count[get_cpu_id()] += 1;
	}
}

static inline int smp_spin_trylock(smp_spin_lock_t *lock)
{
	return __cpu_spin_trylock((void *)&lock->lock);
}

static inline void smp_spin_unlock(smp_spin_lock_t *lock)
{
	if (SMP_init_done == SMP_INIT_MAGIC) {
		__cpu_spin_unlock((void *)&lock->lock);
		spin_lock_count[get_cpu_id()] -= 1;
	}
}

#define smp_spin_lock_irq(lock) \
	do { \
		irq_disable(); \
		smp_spin_lock(lock); \
	} while (0)
#define smp_spin_unlock_irq(lock) \
	do { \
		smp_spin_unlock(lock); \
		irq_enable(); \
	} while (0)

#define smp_spin_lock_save(lock, flags) \
	do { \
		flags = save_irq(); \
		irq_disable(); \
		smp_spin_lock(lock); \
	} while (0)
#define smp_spin_unlock_restore(lock, flags) \
	do { \
		smp_spin_unlock(lock); \
		restore_irq(flags); \
		irq_enable(); \
	} while (0)

#else
#define smp_spin_lock_init(lock)	do {(void)lock; } while (0)
#define smp_spin_lock(lock)			do {(void)lock; } while (0)
#define smp_spin_trylock(lock)		(1)
#define smp_spin_unlock(lock)		do {(void)lock; } while (0)

#define smp_spin_lock_irq(lock)		do {irq_disable(); (void)lock; } while (0)
#define smp_spin_unlock_irq(lock)	do {(void)lock; irq_enable(); } while (0)
#define smp_spin_lock_save(lock, flags)			do {flags = save_cpsr(); (void)lock; } while (0)
#define smp_spin_unlock_restore(lock, flags)	do {(void)lock; restore_cpsr(flags); } while (0)
#endif
#endif  //_SPINLOCK_H_

