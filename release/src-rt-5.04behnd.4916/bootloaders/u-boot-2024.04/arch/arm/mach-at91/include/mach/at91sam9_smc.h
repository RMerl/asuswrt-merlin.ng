/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91sam9_smc.h]
 *
 * Copyright (C) 2007 Andrew Victor
 * Copyright (C) 2007 Atmel Corporation.
 *
 * Static Memory Controllers (SMC) - System peripherals registers.
 * Based on AT91SAM9261 datasheet revision D.
 */

#ifndef AT91SAM9_SMC_H
#define AT91SAM9_SMC_H

#ifdef __ASSEMBLY__

#ifndef ATMEL_BASE_SMC
#define ATMEL_BASE_SMC	ATMEL_BASE_SMC0
#endif

#define AT91_ASM_SMC_SETUP0	ATMEL_BASE_SMC
#define AT91_ASM_SMC_PULSE0	(ATMEL_BASE_SMC + 0x04)
#define AT91_ASM_SMC_CYCLE0	(ATMEL_BASE_SMC + 0x08)
#define AT91_ASM_SMC_MODE0	(ATMEL_BASE_SMC + 0x0C)

#else

typedef struct	at91_cs {
	u32	setup;		/* 0x00 SMC Setup Register */
	u32	pulse;		/* 0x04 SMC Pulse Register */
	u32	cycle;		/* 0x08 SMC Cycle Register */
	u32	mode;		/* 0x0C SMC Mode Register */
} at91_cs_t;

typedef struct	at91_smc {
	at91_cs_t	cs[8];
} at91_smc_t;

#endif /*  __ASSEMBLY__ */

#define AT91_SMC_SETUP_NWE(x)		(x & 0x3f)
#define AT91_SMC_SETUP_NCS_WR(x)	((x & 0x3f) << 8)
#define AT91_SMC_SETUP_NRD(x)		((x & 0x3f) << 16)
#define AT91_SMC_SETUP_NCS_RD(x)	((x & 0x3f) << 24)

#define AT91_SMC_PULSE_NWE(x)		(x & 0x7f)
#define AT91_SMC_PULSE_NCS_WR(x)	((x & 0x7f) << 8)
#define AT91_SMC_PULSE_NRD(x)		((x & 0x7f) << 16)
#define AT91_SMC_PULSE_NCS_RD(x)	((x & 0x7f) << 24)

#define AT91_SMC_CYCLE_NWE(x)		(x & 0x1ff)
#define AT91_SMC_CYCLE_NRD(x)		((x & 0x1ff) << 16)

#define AT91_SMC_MODE_RM_NCS		0x00000000
#define AT91_SMC_MODE_RM_NRD		0x00000001
#define AT91_SMC_MODE_WM_NCS		0x00000000
#define AT91_SMC_MODE_WM_NWE		0x00000002

#define AT91_SMC_MODE_EXNW_DISABLE	0x00000000
#define AT91_SMC_MODE_EXNW_FROZEN	0x00000020
#define AT91_SMC_MODE_EXNW_READY	0x00000030

#define AT91_SMC_MODE_BAT		0x00000100
#define AT91_SMC_MODE_DBW_8		0x00000000
#define AT91_SMC_MODE_DBW_16		0x00001000
#define AT91_SMC_MODE_DBW_32		0x00002000
#define AT91_SMC_MODE_TDF_CYCLE(x)	((x & 0xf) << 16)
#define AT91_SMC_MODE_TDF		0x00100000
#define AT91_SMC_MODE_PMEN		0x01000000
#define AT91_SMC_MODE_PS_4		0x00000000
#define AT91_SMC_MODE_PS_8		0x10000000
#define AT91_SMC_MODE_PS_16		0x20000000
#define AT91_SMC_MODE_PS_32		0x30000000

#endif
