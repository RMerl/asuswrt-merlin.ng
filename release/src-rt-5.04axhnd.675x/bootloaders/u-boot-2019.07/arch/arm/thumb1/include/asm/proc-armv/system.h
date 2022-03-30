/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Thumb-1 drop-in for the linux/include/asm-arm/proc-armv/system.h
 *
 *  (C) Copyright 2015
 *  Albert ARIBAUD <albert.u.boot@aribaud.net>
 *
 * The original file does not build in Thumb mode. However, in U-Boot
 * we don't use interrupt context, so we can redefine these as empty
 * memory barriers, which makes Thumb-1 compiler happy.
 */

/*
 * Use the same macro name as linux/include/asm-arm/proc-armv/system.h
 * here, so that if the original ever gets included after us, it won't
 * try to re-redefine anything.
 */

#ifndef __ASM_PROC_SYSTEM_H
#define __ASM_PROC_SYSTEM_H

/*
 * Redefine all original macros with static inline functions containing
 * a simple memory barrier, so that they produce the same instruction
 * ordering constraints as their original counterparts.
 * We use static inline functions rather than macros so that we can tell
 * the compiler to not complain about unused arguments.
 */

static inline void local_irq_save(
	unsigned long flags __attribute__((unused)))
{
	__asm__ __volatile__ ("" : : : "memory");
}

static inline void local_irq_enable(void)
{
	__asm__ __volatile__ ("" : : : "memory");
}

static inline void local_irq_disable(void)
{
	__asm__ __volatile__ ("" : : : "memory");
}

static inline void __stf(void)
{
	__asm__ __volatile__ ("" : : : "memory");
}

static inline void __clf(void)
{
	__asm__ __volatile__ ("" : : : "memory");
}

static inline void local_save_flags(
	unsigned long flags __attribute__((unused)))
{
	__asm__ __volatile__ ("" : : : "memory");
}

static inline void local_irq_restore(
	unsigned long flags __attribute__((unused)))
{
	__asm__ __volatile__ ("" : : : "memory");
}

#endif /*  __ASM_PROC_SYSTEM_H */
