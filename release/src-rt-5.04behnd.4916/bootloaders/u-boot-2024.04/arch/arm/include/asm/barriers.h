/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 ARM Ltd.
 *
 * ARM and ARM64 barrier instructions
 * split from armv7.h to allow sharing between ARM and ARM64
 *
 * Original copyright in armv7.h was:
 * (C) Copyright 2010 Texas Instruments, <www.ti.com> Aneesh V <aneesh@ti.com>
 *
 * Much of the original barrier code was contributed by:
 *   Valentine Barshak <valentine.barshak@cogentembedded.com>
 */
#ifndef __BARRIERS_H__
#define __BARRIERS_H__

#ifndef __ASSEMBLY__

#ifndef CONFIG_ARM64
/*
 * CP15 Barrier instructions
 * Please note that we have separate barrier instructions in ARMv7
 * However, we use the CP15 based instructtions because we use
 * -march=armv5 in U-Boot
 */
#define CP15ISB	asm volatile ("mcr     p15, 0, %0, c7, c5, 4" : : "r" (0))
#define CP15DSB	asm volatile ("mcr     p15, 0, %0, c7, c10, 4" : : "r" (0))
#define CP15DMB	asm volatile ("mcr     p15, 0, %0, c7, c10, 5" : : "r" (0))

#endif /* !CONFIG_ARM64 */

#if __LINUX_ARM_ARCH__ >= 7
#define ISB	asm volatile ("isb sy" : : : "memory")
#define DSB	asm volatile ("dsb sy" : : : "memory")
#define DMB	asm volatile ("dmb sy" : : : "memory")
#elif __LINUX_ARM_ARCH__ == 6
#define ISB	CP15ISB
#define DSB	CP15DSB
#define DMB	CP15DMB
#else
#define ISB	asm volatile ("" : : : "memory")
#define DSB	CP15DSB
#define DMB	asm volatile ("" : : : "memory")
#endif

#define isb()	ISB
#define dsb()	DSB
#define dmb()	DMB
#endif	/* __ASSEMBLY__ */
#endif	/* __BARRIERS_H__ */
