/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 *
 */

#if defined(CONFIG_ARCH_MX7ULP) || defined(CONFIG_ARCH_IMX8) || \
	defined(CONFIG_ARCH_IMXRT) || defined(CONFIG_ARCH_IMX8ULP) || defined(CONFIG_ARCH_IMX9)
struct lpuart_fsl_reg32 {
	u32 verid;
	u32 param;
	u32 global;
	u32 pincfg;
	u32 baud;
	u32 stat;
	u32 ctrl;
	u32 data;
	u32 match;
	u32 modir;
	u32 fifo;
	u32 water;
};
#else
struct lpuart_fsl_reg32 {
	u32 baud;
	u32 stat;
	u32 ctrl;
	u32 data;
	u32 match;
	u32 modir;
	u32 fifo;
	u32 water;
};
#endif

struct lpuart_fsl {
	u8 ubdh;
	u8 ubdl;
	u8 uc1;
	u8 uc2;
	u8 us1;
	u8 us2;
	u8 uc3;
	u8 ud;
	u8 uma1;
	u8 uma2;
	u8 uc4;
	u8 uc5;
	u8 ued;
	u8 umodem;
	u8 uir;
	u8 reserved;
	u8 upfifo;
	u8 ucfifo;
	u8 usfifo;
	u8 utwfifo;
	u8 utcfifo;
	u8 urwfifo;
	u8 urcfifo;
	u8 rsvd[28];
};

/* Used on i.MX7ULP */
#define LPUART_BAUD_BOTHEDGE_MASK	(0x20000)
#define LPUART_BAUD_OSR_MASK		(0x1F000000)
#define LPUART_BAUD_OSR_SHIFT		(24)
#define LPUART_BAUD_OSR(x)		((((uint32_t)(x)) << 24) & 0x1F000000)
#define LPUART_BAUD_SBR_MASK		(0x1FFF)
#define LPUART_BAUD_SBR_SHIFT		(0U)
#define LPUART_BAUD_SBR(x)		(((uint32_t)(x)) & 0x1FFF)
#define LPUART_BAUD_M10_MASK		(0x20000000U)
#define LPUART_BAUD_SBNS_MASK		(0x2000U)
