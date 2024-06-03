/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Atmel Corporation.
 *
 * Static Memory Controllers (SMC) - System peripherals registers.
 * Based on SAMA5D3 datasheet.
 */

#ifndef SAMA5D3_SMC_H
#define SAMA5D3_SMC_H

#ifdef __ASSEMBLY__
#define AT91_ASM_SMC_SETUP0	(ATMEL_BASE_SMC + 0x600)
#define AT91_ASM_SMC_PULSE0	(ATMEL_BASE_SMC + 0x604)
#define AT91_ASM_SMC_CYCLE0	(ATMEL_BASE_SMC + 0x608)
#define AT91_ASM_SMC_TIMINGS0	(ATMEL_BASE_SMC + 0x60c)
#define AT91_ASM_SMC_MODE0	(ATMEL_BASE_SMC + 0x610)
#else
struct at91_cs {
	u32	setup;		/* 0x600 SMC Setup Register */
	u32	pulse;		/* 0x604 SMC Pulse Register */
	u32	cycle;		/* 0x608 SMC Cycle Register */
	u32	timings;	/* 0x60C SMC Cycle Register */
	u32	mode;		/* 0x610 SMC Mode Register */
};

struct at91_smc {
	u32 reserved[384];
	struct at91_cs cs[4];
};
#endif /*  __ASSEMBLY__ */

#define AT91_SMC_SETUP_NWE(x)		(x & 0x3f)
#define AT91_SMC_SETUP_NCS_WR(x)	((x & 0x3f) << 8)
#define AT91_SMC_SETUP_NRD(x)		((x & 0x3f) << 16)
#define AT91_SMC_SETUP_NCS_RD(x)	((x & 0x3f) << 24)

#define AT91_SMC_PULSE_NWE(x)		(x & 0x3f)
#define AT91_SMC_PULSE_NCS_WR(x)	((x & 0x3f) << 8)
#define AT91_SMC_PULSE_NRD(x)		((x & 0x3f) << 16)
#define AT91_SMC_PULSE_NCS_RD(x)	((x & 0x3f) << 24)

#define AT91_SMC_CYCLE_NWE(x)		(x & 0x1ff)
#define AT91_SMC_CYCLE_NRD(x)		((x & 0x1ff) << 16)

#define AT91_SMC_TIMINGS_TCLR(x)	(x & 0xf)
#define AT91_SMC_TIMINGS_TADL(x)	((x & 0xf) << 4)
#define AT91_SMC_TIMINGS_TAR(x)		((x & 0xf) << 8)
#define AT91_SMC_TIMINGS_OCMS(x)	((x & 0x1) << 12)
#define AT91_SMC_TIMINGS_TRR(x)		((x & 0xf) << 16)
#define AT91_SMC_TIMINGS_TWB(x)		((x & 0xf) << 24)
#define AT91_SMC_TIMINGS_RBNSEL(x)	((x & 0xf) << 28)
#define AT91_SMC_TIMINGS_NFSEL(x)	((x & 0x1) << 31)

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
