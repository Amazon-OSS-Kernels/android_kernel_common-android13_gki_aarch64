/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/armv8/mmu.h>
#include <arch.h>

/*************************************************************************
 *
 * Startup Code (reset vector)
 *
 *************************************************************************/
#define CPU_RELEASE_ADDR _main    //not used

.macro push, xreg1, xreg2
stp \xreg1, \xreg2, [sp, #-16]!
.endm

.macro pop, xreg1, xreg2
ldp \xreg1, \xreg2, [sp], #16
.endm

.globl	_start
_start:
#if defined(LINUX_KERNEL_IMAGE_HEADER)
#include <asm/boot0-linux-kernel-header.h>
#elif defined(CONFIG_ENABLE_ARM_SOC_BOOT0_HOOK)
/*
 * Various SoCs need something special and SoC-specific up front in
 * order to boot, allow them to set that in their boot0.h file and then
 * use it here.
 */
#include <asm/arch/boot0.h>
#else
	b	reset
#endif

	.align 3

.globl	_TEXT_BASE
_TEXT_BASE:
	.quad	CONFIG_SYS_TEXT_BASE

/*
 * These are defined in the linker script.
 */
.globl	_end_ofs
_end_ofs:
	.quad	_end - _start

.globl	_bss_start_ofs
_bss_start_ofs:
	.quad	__bss_start - _start

.globl	_bss_end_ofs
_bss_end_ofs:
	.quad	__bss_end - _start

reset:
	/* Allow the board to save important registers */
	b	save_boot_params
.globl	save_boot_params_ret
save_boot_params_ret:

#if CONFIG_POSITION_INDEPENDENT
	/* Verify that we're 4K aligned.  */
	adr	x0, _start
	ands	x0, x0, #0xfff
	b.eq	1f
0:
	/*
	 * FATAL, can't continue.
	 * U-Boot needs to be loaded at a 4K aligned address.
	 *
	 * We use ADRP and ADD to load some symbol addresses during startup.
	 * The ADD uses an absolute (non pc-relative) lo12 relocation
	 * thus requiring 4K alignment.
	 */
	wfi
	b	0b
1:

	/*
	 * Fix .rela.dyn relocations. This allows U-Boot to be loaded to and
	 * executed at a different address than it was linked at.
	 */
pie_fixup:
	adr	x0, _start		/* x0 <- Runtime value of _start */
	ldr	x1, _TEXT_BASE		/* x1 <- Linked value of _start */
	subs	x9, x0, x1		/* x9 <- Run-vs-link offset */
	beq	pie_fixup_done
	adrp    x2, __rel_dyn_start     /* x2 <- Runtime &__rel_dyn_start */
	add     x2, x2, #:lo12:__rel_dyn_start
	adrp    x3, __rel_dyn_end       /* x3 <- Runtime &__rel_dyn_end */
	add     x3, x3, #:lo12:__rel_dyn_end
pie_fix_loop:
	ldp	x0, x1, [x2], #16	/* (x0, x1) <- (Link location, fixup) */
	ldr	x4, [x2], #8		/* x4 <- addend */
	cmp	w1, #1027		/* relative fixup? */
	bne	pie_skip_reloc
	/* relative fix: store addend plus offset at dest location */
	add	x0, x0, x9
	add	x4, x4, x9
	str	x4, [x0]
pie_skip_reloc:
	cmp	x2, x3
	b.lo	pie_fix_loop
pie_fixup_done:
#endif

#if defined(CONFIG_ARMV8_SPL_EXCEPTION_VECTORS) || !defined(CONFIG_SPL_BUILD)
.macro	set_vbar, regname, reg
	msr	\regname, \reg
.endm
	adr	x0, vectors
#else
.macro	set_vbar, regname, reg
.endm
#endif
	/*
	 * Could be EL3/EL2/EL1, Initial State:
	 * Little Endian, MMU Disabled, i/dCache Disabled
	 */
	switch_el x1, 3f, 2f, 1f
3:	set_vbar vbar_el3, x0
	mrs	x0, scr_el3
	orr	x0, x0, #0xf			/* SCR_EL3.NS|IRQ|FIQ|EA */
	msr	scr_el3, x0
	msr	cptr_el3, xzr			/* Enable FP/SIMD */
	b	0f
2:	mrs	x1, hcr_el2
	tbnz	x1, #34, 1f			/* HCR_EL2.E2H */
	orr	x1, x1, #HCR_EL2_AMO_EL2	/* Route SErrors to EL2 */
	msr	hcr_el2, x1
	set_vbar vbar_el2, x0
	mov	x0, #0x33ff
	msr	cptr_el2, x0			/* Enable FP/SIMD */
	b	0f
1:	set_vbar vbar_el1, x0
	mov	x0, #3 << 20
	msr	cpacr_el1, x0			/* Enable FP/SIMD */
0:
	msr	daifclr, #0x4			/* Unmask SError interrupts */

#ifdef COUNTER_FREQUENCY
	branch_if_not_highest_el x0, 4f
	ldr	x0, =COUNTER_FREQUENCY
	msr	cntfrq_el0, x0			/* Initialize CNTFRQ */
#endif

4:	isb

	/*
	 * Enable SMPEN bit for coherency.
	 * This register is not architectural but at the moment
	 * this bit should be set for A53/A57/A72.
	 */
#ifdef CONFIG_ARMV8_SET_SMPEN
	switch_el x1, 3f, 1f, 1f
3:
	mrs     x0, S3_1_c15_c2_1               /* cpuectlr_el1 */
	orr     x0, x0, #0x40
	msr     S3_1_c15_c2_1, x0
	isb
1:
#endif

	/* Apply ARM core specific erratas */
	bl	apply_core_errata

	/*
	 * Cache/BPB/TLB Invalidate
	 * i-cache is invalidated before enabled in icache_enable()
	 * tlb is invalidated before mmu is enabled in dcache_enable()
	 * d-cache is invalidated before enabled in dcache_enable()
	 */

	/* Processor specific initialization */
	bl	lowlevel_init

#if defined(CONFIG_ARMV8_SPIN_TABLE) && !defined(CONFIG_SPL_BUILD)
	branch_if_master x0, master_cpu
	b	spin_table_secondary_jump
	/* never return */
#elif defined(CONFIG_ARMV8_MULTIENTRY)
	branch_if_master x0, master_cpu

	/*
	 * Slave CPUs
	 */
slave_cpu:
	wfe
	ldr	x1, =CPU_RELEASE_ADDR
	ldr	x0, [x1]
	cbz	x0, slave_cpu
	br	x0			/* branch to the given address */
#endif /* CONFIG_ARMV8_MULTIENTRY */
master_cpu:
	msr	SPSel, #1		/* make sure we use SP_ELx */
	bl	_main

/*-----------------------------------------------------------------------*/

WEAK(apply_core_errata)

	mov	x29, lr			/* Save LR */
	/* For now, we support Cortex-A53, Cortex-A57 specific errata */

	/* Check if we are running on a Cortex-A53 core */
	branch_if_a53_core x0, apply_a53_core_errata

	/* Check if we are running on a Cortex-A57 core */
	branch_if_a57_core x0, apply_a57_core_errata
0:
	mov	lr, x29			/* Restore LR */
	ret

apply_a53_core_errata:

#ifdef CONFIG_ARM_ERRATA_855873
	mrs	x0, midr_el1
	tst	x0, #(0xf << 20)
	b.ne	0b

	mrs	x0, midr_el1
	and	x0, x0, #0xf
	cmp	x0, #3
	b.lt	0b

	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* Enable data cache clean as data cache clean/invalidate */
	orr	x0, x0, #1 << 44
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
	isb
#endif
	b 0b

apply_a57_core_errata:

#ifdef CONFIG_ARM_ERRATA_828024
	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* Disable non-allocate hint of w-b-n-a memory type */
	orr	x0, x0, #1 << 49
	/* Disable write streaming no L1-allocate threshold */
	orr	x0, x0, #3 << 25
	/* Disable write streaming no-allocate threshold */
	orr	x0, x0, #3 << 27
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
	isb
#endif

#ifdef CONFIG_ARM_ERRATA_826974
	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* Disable speculative load execution ahead of a DMB */
	orr	x0, x0, #1 << 59
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
	isb
#endif

#ifdef CONFIG_ARM_ERRATA_833471
	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* FPSCR write flush.
	 * Note that in some cases where a flush is unnecessary this
	    could impact performance. */
	orr	x0, x0, #1 << 38
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
	isb
#endif

#ifdef CONFIG_ARM_ERRATA_829520
	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* Disable Indirect Predictor bit will prevent this erratum
	    from occurring
	 * Note that in some cases where a flush is unnecessary this
	    could impact performance. */
	orr	x0, x0, #1 << 4
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
	isb
#endif

#ifdef CONFIG_ARM_ERRATA_833069
	mrs	x0, S3_1_c15_c2_0	/* cpuactlr_el1 */
	/* Disable Enable Invalidates of BTB bit */
	and	x0, x0, #0xE
	msr	S3_1_c15_c2_0, x0	/* cpuactlr_el1 */
	isb
#endif
	b 0b
ENDPROC(apply_core_errata)

/*-----------------------------------------------------------------------*/

WEAK(lowlevel_init)
	mov	x29, lr			/* Save LR */

	ldr	x0, =REE_ARG_DATA_ADDR
	ldr w1, [x0, #12]               /* Noted:#12 is the offeset of teefail_flag in ST_REE_ARG_DATA */
	ldr w0, =0x0000EEFF
	cmp w0, w1
	beq tee_check_end

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	ldr	x0, =GICD_BASE
	bl	gic_init_secure
1:
#if defined(CONFIG_GICV3)
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
	msr     daifclr, #0x3           /* enable irq & fiq */
#elif defined(CONFIG_GICV2)
	ldr	x0, =GICD_BASE
	ldr	x1, =GICC_BASE
	bl	gic_init_secure_percpu
#endif
#endif

tee_check_end:

#ifdef CONFIG_ARMV8_MULTIENTRY
	branch_if_master x0, 2f

	/*
	 * Slave should wait for master clearing spin table.
	 * This sync prevent salves observing incorrect
	 * value of spin table and jumping to wrong place.
	 */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
#ifdef CONFIG_GICV2
	ldr	x0, =GICC_BASE
#endif
	bl	gic_wait_for_interrupt
#endif

	/*
	 * All slaves will enter EL2 and optionally EL1.
	 */
	adr	x4, lowlevel_in_el2
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2

lowlevel_in_el2:
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	adr	x4, lowlevel_in_el1
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el1

lowlevel_in_el1:
#endif

#endif /* CONFIG_ARMV8_MULTIENTRY */

2:
	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)

WEAK(smp_kick_all_cpus)
	/* Kick secondary cpus up by SGI 0 interrupt */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	ldr	x0, =GICD_BASE
	b	gic_kick_secondary_cpus
#endif
	ret
ENDPROC(smp_kick_all_cpus)

/*-----------------------------------------------------------------------*/

ENTRY(c_runtime_cpu_setup)
#if defined(CONFIG_ARMV8_SPL_EXCEPTION_VECTORS) || !defined(CONFIG_SPL_BUILD)
	/* Relocate vBAR */
	adr	x0, vectors
	switch_el x1, 3f, 2f, 1f
3:	msr	vbar_el3, x0
	b	0f
2:	msr	vbar_el2, x0
	b	0f
1:	msr	vbar_el1, x0
0:
#endif

	ret
ENDPROC(c_runtime_cpu_setup)

WEAK(save_boot_params)
	b	save_boot_params_ret	/* back to my caller */
ENDPROC(save_boot_params)

#define SPINLOCK_LOCK       1
#define SPINLOCK_UNLOCK     0

/* void __cpu_spin_lock(unsigned int *lock); */
ENTRY(__cpu_spin_lock)
	mov     w2, #SPINLOCK_LOCK
	sevl
l1:     wfe
l2:     ldaxr   w1, [x0]
	cbnz    w1, l1
	stxr    w1, w2, [x0]
	cbnz    w1, l2
	ret
ENDPROC(__cpu_spin_lock)

/* unsigned int __cpu_spin_trylock(unsigned int *lock); */
ENTRY(__cpu_spin_trylock)
	mov     x1, x0
	mov     w2, #SPINLOCK_LOCK
.loop:  ldaxr   w0, [x1]
	cbnz    w0, .cpu_spin_trylock_out
	stxr    w0, w2, [x1]
	cbnz    w0, .loop
.cpu_spin_trylock_out:
	ret
ENDPROC(__cpu_spin_trylock)

/* void __cpu_spin_unlock(unsigned int *lock); */
ENTRY(__cpu_spin_unlock)
	stlr    wzr, [x0]
	sev
	ret
ENDPROC(__cpu_spin_unlock)

ENTRY(push_lr)
	push lr, x18
	ret
ENDPROC(push_lr)

ENTRY(pop_lr)
	pop lr, x18
	ret
ENDPROC(pop_lr)

ENTRY(plat_arm_calc_core_pos)
	mov     x3, x0
	/*
	* Check for MT bit in MPIDR. If not set, shift MPIDR to left to make it
	* look as if in a multi-threaded implementation.
	*/
	tst     x0, #MPIDR_MT_MASK
	lsl     x3, x0, #MPIDR_AFFINITY_BITS
	csel    x3, x3, x0, eq
	/* Extract individual affinity fields from MPIDR */
	ubfx    x0, x3, #MPIDR_AFF0_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx    x1, x3, #MPIDR_AFF1_SHIFT, #MPIDR_AFFINITY_BITS
	ubfx    x2, x3, #MPIDR_AFF2_SHIFT, #MPIDR_AFFINITY_BITS
	/* Compute linear position */
	mov     x4, #MAX_PE_PER_CPU
	madd    x0, x1, x4, x0
	mov     x5, #MAX_CPUS_PER_CLUSTER
	madd    x0, x2, x5, x0
	ret
ENDPROC(plat_arm_calc_core_pos)