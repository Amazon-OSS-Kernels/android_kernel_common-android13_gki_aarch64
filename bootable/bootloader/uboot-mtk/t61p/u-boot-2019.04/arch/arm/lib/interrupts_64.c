// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <common.h>
#include <linux/compiler.h>
#include <efi_loader.h>
#include <asm/gic.h>
#include <debug_impl.h>
#if defined(CONFIG_MULTICORES_PLATFORM)
#include <thread_info.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_MULTICORES_PLATFORM)
extern int need_to_reschedule(void);
extern void reschedule_irq(void);
extern smp_spin_lock_t g_sched_lock;
#endif

/* Interrupt related define*/
#define SGI_IRM_BIT 40
#define HCR_EL2_IMO_BIT 4
#define LSBYTE 0xFF
#define INTID_EL1_PHY_TIMER 30
#define INTID_IPI_0 0
#define GICR_PPI_FRAME_SIZE 0x10000
#define GICR_SGI_FRAME_SIZE 0x10000
#define SGI_OFFSET 0x10000
#define TIMER_CNT_MS 1000
#define DEFAULT_TIMER_INTERVAL_MS 100
#define REG_LR 30

unsigned int timer_period_ms = DEFAULT_TIMER_INTERVAL_MS;

typedef void (*mhal_isr_t)(void);
mhal_isr_t timer_isr_process = NULL;

void EL2_ISR_Routing(void)
{
	unsigned long HCR_EL2 = 0;
	asm volatile("mrs %0, HCR_EL2\n":"=r" (HCR_EL2)::);
	HCR_EL2 |= (1UL << HCR_EL2_IMO_BIT);
	asm volatile("msr HCR_EL2, %0" : : "r" (HCR_EL2));
}

void set_physical_timer_period(unsigned int timeout_ms)
{
	unsigned long value, freq, cnt, cmp;

	timer_period_ms = timeout_ms;
	/* Disable Timer*/
	value = 0;
	asm volatile("msr CNTP_CTL_EL0, %0" : : "r" (value));

	/* Set the timeout value */
	asm volatile("mrs %0, CNTFRQ_EL0" : "=r" (freq));
	asm volatile("mrs %0, CNTPCT_EL0" : "=r" (cnt));
	cmp = cnt + (freq/TIMER_CNT_MS)*timeout_ms;
	asm volatile("msr CNTP_CVAL_EL0, %0" :  : "r" (cmp));

	/*Enable Timer*/
	value = 1;
	asm volatile("msr CNTP_CTL_EL0, %0" : : "r" (value));
}

void timer_interrupt_enable(void)
{
	unsigned long GICR_SGI_BASE = 0;
	unsigned long *pGICR = NULL;
#if defined(CONFIG_MULTICORES_PLATFORM)
	unsigned char cpuid = get_cpu_id();
#else
	unsigned char cpuid = 0;
#endif

	GICR_SGI_BASE = GICR_BASE + (cpuid * (GICR_PPI_FRAME_SIZE + GICR_SGI_FRAME_SIZE)) + SGI_OFFSET;

	pGICR = (unsigned long *)(GICR_SGI_BASE + GICR_ISENABLERn);
	pGICR[0] |= (1 << INTID_EL1_PHY_TIMER);

	set_physical_timer_period(timer_period_ms);
}

void cpu_interrupt_setting(void)
{
	EL2_ISR_Routing();
	timer_interrupt_enable();
}

u64 get_physical_timer_cnt(void)
{
	unsigned long cnt;

	asm volatile("mrs %0, CNTPCT_EL0" : "=r" (cnt));
	return cnt;
}


void do_timer_irq(void)
{
	if (timer_isr_process)
		timer_isr_process();

	set_physical_timer_period(timer_period_ms);
}

void timer_irq_register(mhal_isr_t irq)
{
	timer_isr_process = irq;
}

void timer_period_register(unsigned int timeout_ms)
{
	timer_period_ms = timeout_ms;
}

int interrupt_init(void)
{
	return 0;
}

void enable_interrupts(void)
{
#if defined(CONFIG_MULTICORES_PLATFORM)
	asm volatile("msr daifclr, #2");
#endif
	return;
}

int disable_interrupts(void)
{
#if defined(CONFIG_MULTICORES_PLATFORM)
	asm volatile("msr daifset, #2");
#endif
	return 0;
}

void show_regs(struct pt_regs *regs)
{
	int i;

	if (gd->flags & GD_FLG_RELOC)
		printf("elr: %016lx lr : %016lx (reloc)\n",
				regs->elr - gd->reloc_off,
				regs->regs[REG_LR] - gd->reloc_off);
	printf("elr: %016lx lr : %016lx\n", regs->elr, regs->regs[REG_LR]);

	for (i = 0; i < 29; i += 2)
		printf("x%-2d: %016lx x%-2d: %016lx\n",
				i, regs->regs[i], i+1, regs->regs[i+1]);
	printf("\n");
}

/*
 * do_bad_sync handles the impossible case in the Synchronous Abort vector.
 */
void do_bad_sync(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Synchronous Abort\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_irq handles the impossible case in the Irq vector.
 */
void do_bad_irq(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Irq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_fiq handles the impossible case in the Fiq vector.
 */
void do_bad_fiq(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Fiq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_error handles the impossible case in the Error vector.
 */
void do_bad_error(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Error\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_sync handles the Synchronous Abort exception.
 */
void do_sync(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("\"Synchronous Abort\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_irq handles the Irq exception.
 */

void do_ipi(void)
{
#if defined(CONFIG_MULTICORES_PLATFORM)
	unsigned char cpuid = get_cpu_id();

	UBOOT_DEBUG("cpu=%d got IPI and need to schedule\n", cpuid);
	g_scheduler.need_schedule[cpuid] = 1;
#endif
}

void do_irq(struct pt_regs *pt_regs, unsigned int esr)
{
	unsigned int INITD = 0;
#if defined(CONFIG_MULTICORES_PLATFORM)
	unsigned char cpuid = get_cpu_id();
#endif

	/* ack interrupt */
	asm volatile("mrs %0, S3_0_c12_c12_0":"=r"(INITD)::); //ICC_IAR1_EL1

	if (INITD == INTID_EL1_PHY_TIMER)
		do_timer_irq();
	else if (INITD == INTID_IPI_0)
		do_ipi();
	else {
#if defined(CONFIG_MULTICORES_PLATFORM)
		UBOOT_DEBUG("cpuid:%d got INITID:%d!\n", cpuid, INITD);
#else
		UBOOT_DEBUG(" Got unknown INITID:%d!\n", INITD);
#endif
		/* irq stuff */
	}

	/* EOI interrupt */
	asm volatile("msr S3_0_c12_c12_1 ,%0"::"r"(INITD) : ); //ICC_EOIR1_EL1
#if defined(CONFIG_MULTICORES_PLATFORM)
	if (INITD == INTID_EL1_PHY_TIMER) {
		if (need_to_reschedule() && g_sched_lock.lock == 0) {
			irq_disable();
			reschedule_irq();
			asm volatile("clrex");
		}
	}
#endif
}

/*
 * do_fiq handles the Fiq exception.
 */
void do_fiq(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("\"Fiq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_error handles the Error exception.
 * Errors are more likely to be processor specific,
 * it is defined with weak attribute and can be redefined
 * in processor specific code.
 */
void __weak do_error(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("\"Error\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	panic("Resetting CPU ...\n");
}
