/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#ifndef __ASM_NDS_SYSTEM_H
#define __ASM_NDS_SYSTEM_H

/*
 * Interrupt configuring macros.
 */

extern int irq_flags;

#define local_irq_enable() \
	__asm__ __volatile__ ( \
		"mfsr	%0, $psw\n\t" \
		"andi	%0, %0, 0x1\n\t" \
		"setgie.e\n\t" \
		: \
		: "r" (irq_flags) \
	)

#define local_irq_disable() \
	do { \
		int __tmp_dummy; \
		__asm__ __volatile__ ( \
			"mfsr	%0, $psw\n\t" \
			"andi	%0, %0, 0x1\n\t" \
			"setgie.d\n\t" \
			"dsb\n\t" \
			: "=r" (__tmp_dummy) \
		); \
	} while (0)

#define local_irq_save(x) \
	__asm__ __volatile__ ( \
		"mfsr	%0, $psw\n\t" \
		"andi	%0, %0, 0x1\n\t" \
		"setgie.d\n\t" \
		"dsb\n\t" \
		: "=&r" (x) \
	)

#define local_save_flags(x) \
	__asm__ __volatile__ ( \
		"mfsr	%0, $psw\n\t" \
		"andi	%0, %0, 0x1\n\t" \
		"setgie.e\n\t" \
		"setgie.d\n\t" \
		: "=r" (x) \
	)

#define irqs_enabled_from_flags(x) ((x) != 0x1f)

#define local_irq_restore(x) \
	do { \
		if (irqs_enabled_from_flags(x)) \
			local_irq_enable(); \
	} while (0)

/*
 * Force strict CPU ordering.
 */
#define nop()			asm volatile ("nop;\n\t" : : )
#define mb()			asm volatile (""   : : : "memory")
#define rmb()			asm volatile (""   : : : "memory")
#define wmb()			asm volatile (""   : : : "memory")

#endif	/* __ASM_NDS_SYSTEM_H */
