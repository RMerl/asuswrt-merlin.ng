/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */
#ifndef __ASM_NIOS2_SYSTEM_H_
#define __ASM_NIOS2_SYSTEM_H_

#define local_irq_enable() __asm__ __volatile__ (  \
	"rdctl	r8, status\n"			   \
	"ori	r8, r8, 1\n"			   \
	"wrctl	status, r8\n"			   \
	: : : "r8")

#define local_irq_disable() __asm__ __volatile__ ( \
	"rdctl	r8, status\n"			   \
	"andi	r8, r8, 0xfffe\n"		   \
	"wrctl	status, r8\n"			   \
	: : : "r8")

#define local_save_flags(x) __asm__ __volatile__ (	\
	"rdctl	r8, status\n"				\
	"mov	%0, r8\n"				\
	: "=r" (x) : : "r8", "memory")

#define local_irq_restore(x) __asm__ __volatile__ (	\
	"mov	r8, %0\n"				\
	"wrctl	status, r8\n"				\
	: : "r" (x) : "r8", "memory")

/* For spinlocks etc */
#define local_irq_save(x) do { local_save_flags(x); local_irq_disable(); } \
	while (0)

#define	irqs_disabled()					\
({							\
	unsigned long flags;				\
	local_save_flags(flags);			\
	((flags & NIOS2_STATUS_PIE_MSK) == 0x0);	\
})

/* indirect call to go beyond 256MB limitation of toolchain */
#define nios2_callr(addr) __asm__ __volatile__ (	\
	"callr	%0"					\
	: : "r" (addr))

void display_sysid(void);

#endif /* __ASM_NIOS2_SYSTEM_H */
