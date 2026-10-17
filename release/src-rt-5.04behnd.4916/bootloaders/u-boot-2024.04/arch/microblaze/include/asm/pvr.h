/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022, Ovidiu Panait <ovpanait@gmail.com>
 */

#ifndef __ASM_MICROBLAZE_PVR_H
#define __ASM_MICROBLAZE_PVR_H

#include <asm/asm.h>

#define PVR_FULL_COUNT 13 /* PVR0 - PVR12 */

#define __get_pvr(val, reg)						\
	__asm__ __volatile__ ("mfs %0," #reg : "=r" (val) :: "memory")
#define get_pvr(pvrid, val)						\
	__get_pvr(val, rpvr ## pvrid)

#define PVR_MSR_BIT			0x00000400

/* PVR0 masks */
#define PVR0_PVR_FULL_MASK		0x80000000
#define PVR0_VERSION_MASK		0x0000FF00

/* PVR4 masks - ICache configs */
#define PVR4_ICACHE_LINE_LEN_MASK	0x00E00000 /* ICLL */
#define PVR4_ICACHE_BYTE_SIZE_MASK	0x001F0000 /* ICBS */

/* PVR5 masks - DCache configs */
#define PVR5_DCACHE_LINE_LEN_MASK	0x00E00000 /* DCLL */
#define PVR5_DCACHE_BYTE_SIZE_MASK	0x001F0000 /* DCBS */

/* PVR10 masks - FPGA family */
#define PVR10_TARGET_FAMILY_MASK	0xFF000000

/* PVR11 masks - MMU */
#define PVR11_USE_MMU			0xC0000000

/* PVR access macros */
#define PVR_VERSION(pvr)						\
	((pvr[0] & PVR0_VERSION_MASK) >> 8)

#define PVR_ICACHE_LINE_LEN(pvr)					\
	((1 << ((pvr[4] & PVR4_ICACHE_LINE_LEN_MASK) >> 21)) << 2)
#define PVR_ICACHE_BYTE_SIZE(pvr)					\
	(1 << ((pvr[4] & PVR4_ICACHE_BYTE_SIZE_MASK) >> 16))

#define PVR_DCACHE_LINE_LEN(pvr)					\
	((1 << ((pvr[5] & PVR5_DCACHE_LINE_LEN_MASK) >> 21)) << 2)
#define PVR_DCACHE_BYTE_SIZE(pvr)					\
	(1 << ((pvr[5] & PVR5_DCACHE_BYTE_SIZE_MASK) >> 16))

#define PVR_USE_MMU(pvr)						\
	((pvr[11] & PVR11_USE_MMU) >> 30)

#define PVR_TARGET_FAMILY(pvr)						\
	((pvr[10] & PVR10_TARGET_FAMILY_MASK) >> 24)

/**
 * microblaze_cpu_has_pvr_full() - Check for full PVR support
 *
 * Check MSR register for PVR support and, if applicable, check the PVR0
 * register for full PVR support.
 *
 * Return: 1 if there is full PVR support, 0 otherwise.
 */
int microblaze_cpu_has_pvr_full(void);

/**
 * microblaze_get_all_pvrs() - Copy PVR0-PVR12 to destination array
 *
 * @pvr: destination array of size PVR_FULL_COUNT
 */
void microblaze_get_all_pvrs(u32 pvr[PVR_FULL_COUNT]);

#endif	/* __ASM_MICROBLAZE_PVR_H */
