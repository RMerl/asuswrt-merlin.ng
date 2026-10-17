/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Special Function Register (SFR)
 *
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 */

#ifndef __AT91_SFR_H
#define __AT91_SFR_H

#include <linux/bitops.h>
struct atmel_sfr {
	u32 reserved1;	/* 0x00 */
	union {
		u32 ddrcfg;	/* 0x04: DDR Configuration Register */
		u32 ebicsa;	/* 0x04: EBI Chip Select Register */
	};
	u32 reserved2;	/* 0x08 */
	u32 reserved3;	/* 0x0c */
	u32 ohciicr;	/* 0x10: OHCI Interrupt Configuration Register */
	u32 ohciisr;	/* 0x14: OHCI Interrupt Status Register */
	u32 reserved4[4];	/* 0x18 ~ 0x24 */
	u32 secure;		/* 0x28: Security Configuration Register */
	u32 reserved5[5];	/* 0x2c ~ 0x3c */
	u32 ebicfg;		/* 0x40: EBI Configuration Register */
	u32 reserved6[2];	/* 0x44 ~ 0x48 */
	u32 sn0;		/* 0x4c */
	u32 sn1;		/* 0x50 */
	u32 aicredir;	/* 0x54 */
	u32 l2cc_hramc;	/* 0x58 */
};

/* Register Mapping*/
#define AT91_SFR_DDRCFG		0x04	/* DDR Configuration Register */
#define AT91_SFR_CCFG_EBICSA	0x04	/* EBI Chip Select Register */
/* 0x08 ~ 0x0c: Reserved */
#define AT91_SFR_OHCIICR	0x10	/* OHCI INT Configuration Register */
#define AT91_SFR_OHCIISR	0x14	/* OHCI INT Status Register */
#define AT91_SFR_UTMICKTRIM	0x30	/* UTMI Clock Trimming Register */
#define AT91_SFR_UTMISWAP	0x3c	/* UTMI DP/DM Pin Swapping Register */
#define AT91_SFR_LS		0x7c	/* Light Sleep Register */
#define AT91_SFR_I2SCLKSEL	0x90	/* I2SC Register */
#define AT91_SFR_WPMR		0xe4	/* Write Protection Mode Register */

/* Bit field in DDRCFG */
#define ATMEL_SFR_DDRCFG_FDQIEN		0x00010000
#define ATMEL_SFR_DDRCFG_FDQSIEN	0x00020000

/* Bit field in EBICFG */
#define AT91_SFR_EBICFG_DRIVE0		(0x3 << 0)
#define AT91_SFR_EBICFG_DRIVE0_LOW		(0x0 << 0)
#define AT91_SFR_EBICFG_DRIVE0_MEDIUM		(0x2 << 0)
#define AT91_SFR_EBICFG_DRIVE0_HIGH		(0x3 << 0)
#define AT91_SFR_EBICFG_PULL0		(0x3 << 2)
#define AT91_SFR_EBICFG_PULL0_UP		(0x0 << 2)
#define AT91_SFR_EBICFG_PULL0_NONE		(0x1 << 2)
#define AT91_SFR_EBICFG_PULL0_DOWN		(0x3 << 2)
#define AT91_SFR_EBICFG_SCH0		(0x1 << 4)
#define AT91_SFR_EBICFG_SCH0_OFF		(0x0 << 4)
#define AT91_SFR_EBICFG_SCH0_ON			(0x1 << 4)
#define AT91_SFR_EBICFG_DRIVE1		(0x3 << 8)
#define AT91_SFR_EBICFG_DRIVE1_LOW		(0x0 << 8)
#define AT91_SFR_EBICFG_DRIVE1_MEDIUM		(0x2 << 8)
#define AT91_SFR_EBICFG_DRIVE1_HIGH		(0x3 << 8)
#define AT91_SFR_EBICFG_PULL1		(0x3 << 10)
#define AT91_SFR_EBICFG_PULL1_UP		(0x0 << 10)
#define AT91_SFR_EBICFG_PULL1_NONE		(0x1 << 10)
#define AT91_SFR_EBICFG_PULL1_DOWN		(0x3 << 10)
#define AT91_SFR_EBICFG_SCH1		(0x1 << 12)
#define AT91_SFR_EBICFG_SCH1_OFF		(0x0 << 12)
#define AT91_SFR_EBICFG_SCH1_ON			(0x1 << 12)

/* Bit field in AICREDIR */
#define ATMEL_SFR_AICREDIR_NSAIC	0x00000001

/* Bit field in DDRCFG */
#define ATMEL_SFR_DDRCFG_FDQIEN                0x00010000
#define ATMEL_SFR_DDRCFG_FDQSIEN       0x00020000

#define AT91_SFR_CCFG_EBI_CSA(cs, val)		((val) << (cs))
#define AT91_SFR_CCFG_EBI_DBPUC			BIT(8)
#define AT91_SFR_CCFG_EBI_DBPDC			BIT(9)
#define AT91_SFR_CCFG_EBI_DRIVE_SAM9X60		BIT(16)
#define AT91_SFR_CCFG_EBI_DRIVE			BIT(17)
#define AT91_SFR_CCFG_DQIEN_F			BIT(20)
#define AT91_SFR_CCFG_NFD0_ON_D16		BIT(24)
#define AT91_SFR_CCFG_DDR_MP_EN			BIT(25)

#define AT91_SFR_OHCIICR_RES(x)			BIT(x)
#define AT91_SFR_OHCIICR_ARIE			BIT(4)
#define AT91_SFR_OHCIICR_APPSTART		BIT(5)
#define AT91_SFR_OHCIICR_USB_SUSP(x)		BIT(8 + (x))
#define AT91_SFR_OHCIICR_UDPPUDIS		BIT(23)
#define AT91_OHCIICR_USB_SUSPEND		GENMASK(10, 8)

#define AT91_SFR_OHCIISR_RIS(x)			BIT(x)

#define AT91_UTMICKTRIM_FREQ			GENMASK(1, 0)

#define AT91_SFR_UTMISWAP_PORT(x)		BIT(x)

#define AT91_SFR_LS_VALUE(x)			BIT(x)
#define AT91_SFR_LS_MEM_POWER_GATING_ULP1_EN	BIT(16)

#define AT91_SFR_WPMR_WPEN			BIT(0)
#define AT91_SFR_WPMR_WPKEY_MASK		GENMASK(31, 8)

#endif
