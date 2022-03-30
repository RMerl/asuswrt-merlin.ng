/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF520x Internal Memory Map
 *
 * Copyright (C) 2004-2009 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __IMMAP_520X__
#define __IMMAP_520X__

#define MMAP_SCM1	(CONFIG_SYS_MBAR + 0x00000000)
#define MMAP_XBS	(CONFIG_SYS_MBAR + 0x00004000)
#define MMAP_FBCS	(CONFIG_SYS_MBAR + 0x00008000)
#define MMAP_FEC0	(CONFIG_SYS_MBAR + 0x00030000)
#define MMAP_SCM2	(CONFIG_SYS_MBAR + 0x00040000)
#define MMAP_EDMA	(CONFIG_SYS_MBAR + 0x00044000)
#define MMAP_INTC0	(CONFIG_SYS_MBAR + 0x00048000)
#define MMAP_INTCACK	(CONFIG_SYS_MBAR + 0x00054000)
#define MMAP_I2C	(CONFIG_SYS_MBAR + 0x00058000)
#define MMAP_QSPI	(CONFIG_SYS_MBAR + 0x0005C000)
#define MMAP_UART0	(CONFIG_SYS_MBAR + 0x00060000)
#define MMAP_UART1	(CONFIG_SYS_MBAR + 0x00064000)
#define MMAP_UART2	(CONFIG_SYS_MBAR + 0x00068000)
#define MMAP_DTMR0	(CONFIG_SYS_MBAR + 0x00070000)
#define MMAP_DTMR1	(CONFIG_SYS_MBAR + 0x00074000)
#define MMAP_DTMR2	(CONFIG_SYS_MBAR + 0x00078000)
#define MMAP_DTMR3	(CONFIG_SYS_MBAR + 0x0007C000)
#define MMAP_PIT0	(CONFIG_SYS_MBAR + 0x00080000)
#define MMAP_PIT1	(CONFIG_SYS_MBAR + 0x00084000)
#define MMAP_EPORT0	(CONFIG_SYS_MBAR + 0x00088000)
#define MMAP_WDOG	(CONFIG_SYS_MBAR + 0x0008C000)
#define MMAP_PLL	(CONFIG_SYS_MBAR + 0x00090000)
#define MMAP_RCM	(CONFIG_SYS_MBAR + 0x000A0000)
#define MMAP_CCM	(CONFIG_SYS_MBAR + 0x000A0004)
#define MMAP_GPIO	(CONFIG_SYS_MBAR + 0x000A4000)
#define MMAP_SDRAM	(CONFIG_SYS_MBAR + 0x000A8000)

#include <asm/coldfire/crossbar.h>
#include <asm/coldfire/edma.h>
#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/qspi.h>

/* System Controller Module */
typedef struct scm1 {
	u32 mpr;		/* 0x00 Master Privilege */
	u32 rsvd1[7];
	u32 pacra;		/* 0x20 Peripheral Access Ctrl A */
	u32 pacrb;		/* 0x24 Peripheral Access Ctrl B */
	u32 pacrc;		/* 0x28 Peripheral Access Ctrl C */
	u32 pacrd;		/* 0x2C Peripheral Access Ctrl D */
	u32 rsvd2[4];
	u32 pacre;		/* 0x40 Peripheral Access Ctrl E */
	u32 pacrf;		/* 0x44 Peripheral Access Ctrl F */
	u32 rsvd3[3];
	u32 bmt;		/* 0x50 bus monitor */
} scm1_t;

typedef struct scm2 {
	u8 rsvd1[19];		/* 0x00 - 0x12 */
	u8 wcr;			/* 0x13 */
	u16 rsvd2;		/* 0x14 - 0x15 */
	u16 cwcr;		/* 0x16 */
	u8 rsvd3[3];		/* 0x18 - 0x1A */
	u8 cwsr;		/* 0x1B */
	u8 rsvd4[3];		/* 0x1C - 0x1E */
	u8 scmisr;		/* 0x1F */
	u8 rsvd5[79];		/* 0x20 - 0x6F */
	u32 cfadr;		/* 0x70 */
	u8 rsvd7;		/* 0x74 */
	u8 cfier;		/* 0x75 */
	u8 cfloc;		/* 0x76 */
	u8 cfatr;		/* 0x77 */
	u32 rsvd8;		/* 0x78 - 0x7B */
	u32 cfdtr;		/* 0x7C */
} scm2_t;

/* Chip configuration module */
typedef struct rcm {
	u8 rcr;
	u8 rsr;
} rcm_t;

typedef struct ccm_ctrl {
	u16 ccr;		/* 0x00 Chip Cfg */
	u16 res1;		/* 0x02 */
	u16 rcon;		/* 0x04 Reset Cfg */
	u16 cir;		/* 0x06 Chip ID */
} ccm_t;

/* GPIO port */
typedef struct gpio_ctrl {
	/* Port Output Data */
	u8 podr_busctl;		/* 0x00 */
	u8 podr_be;		/* 0x01 */
	u8 podr_cs;		/* 0x02 */
	u8 podr_feci2c;		/* 0x03 */
	u8 podr_qspi;		/* 0x04 */
	u8 podr_timer;		/* 0x05 */
	u8 podr_uart;		/* 0x06 */
	u8 podr_fech;		/* 0x07 */
	u8 podr_fecl;		/* 0x08 */
	u8 res01[3];		/* 0x9 - 0x0B */

	/* Port Data Direction */
	u8 pddr_busctl;		/* 0x0C */
	u8 pddr_be;		/* 0x0D */
	u8 pddr_cs;		/* 0x0E */
	u8 pddr_feci2c;		/* 0x0F */
	u8 pddr_qspi;		/* 0x10*/
	u8 pddr_timer;		/* 0x11 */
	u8 pddr_uart;		/* 0x12 */
	u8 pddr_fech;		/* 0x13 */
	u8 pddr_fecl;		/* 0x14 */
	u8 res02[5];		/* 0x15 - 0x19 */

	/* Port Data Direction */
	u8 ppdr_cs;		/* 0x1A */
	u8 ppdr_feci2c;		/* 0x1B */
	u8 ppdr_qspi;		/* 0x1C */
	u8 ppdr_timer;		/* 0x1D */
	u8 ppdr_uart;		/* 0x1E */
	u8 ppdr_fech;		/* 0x1F */
	u8 ppdr_fecl;		/* 0x20 */
	u8 res03[3];		/* 0x21 - 0x23 */

	/* Port Clear Output Data */
	u8 pclrr_busctl;	/* 0x24 */
	u8 pclrr_be;		/* 0x25 */
	u8 pclrr_cs;		/* 0x26 */
	u8 pclrr_feci2c;	/* 0x27 */
	u8 pclrr_qspi;		/* 0x28 */
	u8 pclrr_timer;		/* 0x29 */
	u8 pclrr_uart;		/* 0x2A */
	u8 pclrr_fech;		/* 0x2B */
	u8 pclrr_fecl;		/* 0x2C */
	u8 res04[3];		/* 0x2D - 0x2F */

	/* Pin Assignment */
	u8 par_busctl;		/* 0x30 */
	u8 par_be;		/* 0x31 */
	u8 par_cs;		/* 0x32 */
	u8 par_feci2c;		/* 0x33 */
	u8 par_qspi;		/* 0x34 */
	u8 par_timer;		/* 0x35 */
	u16 par_uart;		/* 0x36 */
	u8 par_fec;		/* 0x38 */
	u8 par_irq;		/* 0x39 */

	/* Mode Select Control */
	/* Drive Strength Control */
	u8 mscr_fb;		/* 0x3A */
	u8 mscr_sdram;		/* 0x3B */

	u8 dscr_i2c;		/* 0x3C */
	u8 dscr_misc;		/* 0x3D */
	u8 dscr_fec;		/* 0x3E */
	u8 dscr_uart;		/* 0x3F */
	u8 dscr_qspi;		/* 0x40 */
} gpio_t;

/* SDRAM controller */
typedef struct sdram_ctrl {
	u32 mode;		/* 0x00 Mode/Extended Mode */
	u32 ctrl;		/* 0x04 Ctrl */
	u32 cfg1;		/* 0x08 Cfg 1 */
	u32 cfg2;		/* 0x0C Cfg 2 */
	u32 res1[64];		/* 0x10 - 0x10F */
	u32 cs0;		/* 0x110 Chip Select 0 Cfg */
	u32 cs1;		/* 0x114 Chip Select 1 Cfg */
} sdram_t;

/* Clock Module */
typedef struct pll_ctrl {
	u8 odr;			/* 0x00 Output divider */
	u8 rsvd1;
	u8 cr;			/* 0x02 Control */
	u8 rsvd2;
	u8 mdr;			/* 0x04 Modulation Divider */
	u8 rsvd3;
	u8 fdr;			/* 0x06 Feedback Divider */
	u8 rsvd4;
} pll_t;

/* Watchdog registers */
typedef struct wdog_ctrl {
	u16 cr;			/* 0x00 Control */
	u16 mr;			/* 0x02 Modulus */
	u16 cntr;		/* 0x04 Count */
	u16 sr;			/* 0x06 Service */
} wdog_t;

#endif				/* __IMMAP_520X__ */
