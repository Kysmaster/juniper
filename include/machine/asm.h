#ifndef _MACHINE_ASM_H_
#define	_MACHINE_ASM_H_

#undef __FBSDID
#if !defined(lint) && !defined(STRIP_FBSDID)
#define	__FBSDID(s)     .ident s
#else
#define	__FBSDID(s)     /* nothing */
#endif

#define	_C_LABEL(x)	x

#ifdef KDTRACE_HOOKS
#define	DTRACE_NOP	nop
#else
#define	DTRACE_NOP
#endif

#define	LENTRY(sym)						\
	.text; .align 2; .type sym,#function; sym:		\
	.cfi_startproc; DTRACE_NOP
#define	ENTRY(sym)						\
	.globl sym; LENTRY(sym)
#define	EENTRY(sym)						\
	.globl	sym; sym:
#define	LEND(sym) .ltorg; .cfi_endproc; .size sym, . - sym
#define	END(sym) LEND(sym)
#define	EEND(sym)

#define DATA(x) .global x; .type x,STT_OBJECT; x:
#define END_DATA(x) .size x, . - x


#define	WEAK_REFERENCE(sym, alias)				\
	.weak alias;						\
	.set alias,sym

#define	UINT64_C(x)	(x)

#if defined(PIC)
#define	PIC_SYM(x,y)	x ## @ ## y
#else
#define	PIC_SYM(x,y)	x
#endif

/* Alias for link register x30 */
#define	lr		x30

/*
 * Sets the trap fault handler. The exception handler will return to the
 * address in the handler register on a data abort or the xzr register to
 * clear the handler. The tmp parameter should be a register able to hold
 * the temporary data.
 */
#define	SET_FAULT_HANDLER(handler, tmp)					\
	ldr	tmp, [x18, #PC_CURTHREAD];	/* Load curthread */	\
	ldr	tmp, [tmp, #TD_PCB];		/* Load the pcb */	\
	str	handler, [tmp, #PCB_ONFAULT]	/* Set the handler */

#define	ENTER_USER_ACCESS(reg, tmp)					\
	ldr	tmp, =has_pan;			/* Get the addr of has_pan */ \
	ldr	reg, [tmp];			/* Read it */		\
	cbz	reg, 997f;			/* If no PAN skip */	\
	.inst	0xd500409f | (0 << 8);		/* Clear PAN */		\
	997:

#define	EXIT_USER_ACCESS(reg)						\
	cbz	reg, 998f;			/* If no PAN skip */	\
	.inst	0xd500409f | (1 << 8);		/* Set PAN */		\
	998:

#define	EXIT_USER_ACCESS_CHECK(reg, tmp)				\
	ldr	tmp, =has_pan;			/* Get the addr of has_pan */ \
	ldr	reg, [tmp];			/* Read it */		\
	cbz	reg, 999f;			/* If no PAN skip */	\
	.inst	0xd500409f | (1 << 8);		/* Set PAN */		\
	999:

/*
 * Some AArch64 CPUs speculate past an eret instruction. As the user may
 * control the registers at this point add a speculation barrier usable on
 * all AArch64 CPUs after the eret instruction.
 * TODO: ARMv8.5 adds a specific instruction for this, we could use that
 * if we know we are running on something that supports it.
 */
#define	ERET								\
	eret;								\
	dsb	sy;							\
	isb

#endif /* _MACHINE_ASM_H_ */
