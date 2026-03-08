#define ENTRY(name) .globl name; .align 4; name:
#define END(name)   .size name, .-name
#define UNLOCK 0
#define LOCK 1

#if 0
ENTRY(arch_spin_lock)
	mov		r2, #(LOCK)
spin:
	ldrex	r1, [r0]			// see what state the lock is in
	teq		r1, #(UNLOCK)		// locked already?
	wfene						// wait if it's locked
	strexeq	r1, r2, [r0]		// try to lock it, if it's unlocked
	teqeq	r1, #0x0			// did we fail?
	bne		spin				// if any failure, loop
	dmb							// Required before accessing protected resource
	bx		lr
END(arch_spin_lock)


ENTRY(arch_try_lock)
	mov 	r2, #(LOCK)
	ldrex	r1, [r0]			// see what state the lock is in
	teq 	r1, #(UNLOCK) 		// locked already?
	strexeq r1, r2, [r0]		// try to lock it, if it's unlocked
	teqeq	r1, #0x0			// did we fail?
	movne	r0, #0x0
	bne		end
	mov 	r0, #0x1			// put success status of the lock in r0 for function return
	dmb
end:
	bx		lr
END(arch_try_lock)


ENTRY(arch_spin_unlock)
	mov 	r1, #(UNLOCK)
	dmb							// Ensure accesses to protected resource have completed
	str 	r1, [r0]			// release spin lock
	dsb							// Ensure update of the mutex occurs before other CPUs wake
	sev 						// unlock any spinlock
	bx		lr
END(arch_spin_unlock)
#endif

.global __switch_context

__switch_context:
	cmp 	x0, #0
	beq 	_next

	/* Store current thread context */
	mov     x2, sp              /* store sp */
	mov     x3, x30             /* store lr */
	stp 	x2, x3, [x0], #16
	mov		x2, x30             /* Store the lr to the thread.reg.pc */
	//mrs		x3, cpsr        /* AAarh64 do not have cpsr */
	stp     x2, x3, [x0], #16
	add		x0, x0, #(8*4)		/* skip r0 - r3 */
	stp     x4, x5, [x0], #16
	stp     x6, x7, [x0], #16
	stp     x8, x9, [x0], #16
	stp     x10, x11, [x0], #16
    str     x12, [x0]

_next:
	clrex						/*clear local exclusive monitor */
	ldr		x2, [x1], #8
	mov     sp, x2              /* restore sp */
	ldp     x2, x3, [x1], #16   /* load lr pc to x2 x3 */
	ldr     x4, [x1], #8
	stp     x2, x3, [sp, #-16]!  /* push lr, pc */
	//msr 	spsr, x4             /* AAarh64 do not have cpsr */
	mov		x30, x1              /* use x30(lr) as temp reg */
	ldp     x0, x1, [x30], #16
	ldp     x2, x3, [x30], #16
	ldp     x4, x5, [x30], #16
	ldp     x6, x7, [x30], #16
	ldp     x8, x9, [x30], #16
	ldp     x10, x11, [x30], #16
	ldr     x12, [x30]
	ldp     x13, x30, [sp], #16  /* pop lr, pc, but pc is not important here we just load it to as temp reg x13 */
	ret                          /* using ret to jump to lr address */

	/* ldmfd	sp!, {lr, pc}^ */
	/*	ldr		lr,[sp] */
	/*	add		sp,sp,#4	*/
	/*	ldr		pc,[sp]	*/
