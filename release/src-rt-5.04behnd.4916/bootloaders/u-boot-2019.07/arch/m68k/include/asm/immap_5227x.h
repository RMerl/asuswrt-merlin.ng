/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5227x Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __IMMAP_5227X__
#define __IMMAP_5227X__

/* Module Base Addresses */
#define MMAP_SCM1	(CONFIG_SYS_MBAR + 0x00000000)
#define MMAP_XBS	(CONFIG_SYS_MBAR + 0x00004000)
#define MMAP_FBCS	(CONFIG_SYS_MBAR + 0x00008000)
#define MMAP_CAN	(CONFIG_SYS_MBAR + 0x00020000)
#define MMAP_RTC	(CONFIG_SYS_MBAR + 0x0003C000)
#define MMAP_SCM2	(CONFIG_SYS_MBAR + 0x00040010)
#define MMAP_SCM3	(CONFIG_SYS_MBAR + 0x00040070)
#define MMAP_EDMA	(CONFIG_SYS_MBAR + 0x00044000)
#define MMAP_INTC0	(CONFIG_SYS_MBAR + 0x00048000)
#define MMAP_INTC1	(CONFIG_SYS_MBAR + 0x0004C000)
#define MMAP_IACK	(CONFIG_SYS_MBAR + 0x00054000)
#define MMAP_I2C	(CONFIG_SYS_MBAR + 0x00058000)
#define MMAP_DSPI	(CONFIG_SYS_MBAR + 0x0005C000)
#define MMAP_UART0	(CONFIG_SYS_MBAR + 0x00060000)
#define MMAP_UART1	(CONFIG_SYS_MBAR + 0x00064000)
#define MMAP_UART2	(CONFIG_SYS_MBAR + 0x00068000)
#define MMAP_DTMR0	(CONFIG_SYS_MBAR + 0x00070000)
#define MMAP_DTMR1	(CONFIG_SYS_MBAR + 0x00074000)
#define MMAP_DTMR2	(CONFIG_SYS_MBAR + 0x00078000)
#define MMAP_DTMR3	(CONFIG_SYS_MBAR + 0x0007C000)
#define MMAP_PIT0	(CONFIG_SYS_MBAR + 0x00080000)
#define MMAP_PIT1	(CONFIG_SYS_MBAR + 0x00084000)
#define MMAP_PWM	(CONFIG_SYS_MBAR + 0x00090000)
#define MMAP_EPORT	(CONFIG_SYS_MBAR + 0x00094000)
#define MMAP_RCM	(CONFIG_SYS_MBAR + 0x000A0000)
#define MMAP_CCM	(CONFIG_SYS_MBAR + 0x000A0004)
#define MMAP_GPIO	(CONFIG_SYS_MBAR + 0x000A4000)
#define MMAP_ADC	(CONFIG_SYS_MBAR + 0x000A8000)
#define MMAP_LCD	(CONFIG_SYS_MBAR + 0x000AC000)
#define MMAP_LCD_BGLUT	(CONFIG_SYS_MBAR + 0x000AC800)
#define MMAP_LCD_GWLUT	(CONFIG_SYS_MBAR + 0x000ACC00)
#define MMAP_USBHW	(CONFIG_SYS_MBAR + 0x000B0000)
#define MMAP_USBCAPS	(CONFIG_SYS_MBAR + 0x000B0100)
#define MMAP_USBEHCI	(CONFIG_SYS_MBAR + 0x000B0140)
#define MMAP_USBOTG	(CONFIG_SYS_MBAR + 0x000B01A0)
#define MMAP_SDRAM	(CONFIG_SYS_MBAR + 0x000B8000)
#define MMAP_SSI	(CONFIG_SYS_MBAR + 0x000BC000)
#define MMAP_PLL	(CONFIG_SYS_MBAR + 0x000C0000)

#include <asm/coldfire/crossbar.h>
#include <asm/coldfire/dspi.h>
#include <asm/coldfire/edma.h>
#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/flexcan.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/lcd.h>
#include <asm/coldfire/pwm.h>
#include <asm/coldfire/ssi.h>

/* Reset Controller Module (RCM) */
typedef struct rcm {
	u8 rcr;
	u8 rsr;
} rcm_t;

/* Chip Configuration Module (CCM) */
typedef struct ccm {
	u16 ccr;		/* Chip Configuration (Rd-only) */
	u16 resv1;
	u16 rcon;		/* Reset Configuration (Rd-only) */
	u16 cir;		/* Chip Identification (Rd-only) */
	u32 resv2;
	u16 misccr;		/* Miscellaneous Control */
	u16 cdr;		/* Clock Divider */
	u16 uocsr;		/* USB On-the-Go Controller Status */
	u16 resv4;
	u16 sbfsr;		/* Serial Boot Status */
	u16 sbfcr;		/* Serial Boot Control */
} ccm_t;

typedef struct canex_ctrl {
	can_msg_t msg[16];	/* 0x00 Message Buffer 0-15 */
	u32 res0[0x700];	/* 0x100 */
	can_msg_t rxim[16];	/* 0x800 Rx Individual Mask 0-15 */
} canex_t;

/* General Purpose I/O Module (GPIO) */
typedef struct gpio {
	/* Port Output Data Registers */
	u8 podr_be;		/* 0x00 */
	u8 podr_cs;		/* 0x01 */
	u8 podr_fbctl;		/* 0x02 */
	u8 podr_i2c;		/* 0x03 */
	u8 rsvd1;		/* 0x04 */
	u8 podr_uart;		/* 0x05 */
	u8 podr_dspi;		/* 0x06 */
	u8 podr_timer;		/* 0x07 */
	u8 podr_lcdctl;		/* 0x08 */
	u8 podr_lcddatah;	/* 0x09 */
	u8 podr_lcddatam;	/* 0x0A */
	u8 podr_lcddatal;	/* 0x0B */

	/* Port Data Direction Registers */
	u8 pddr_be;		/* 0x0C */
	u8 pddr_cs;		/* 0x0D */
	u8 pddr_fbctl;		/* 0x0E */
	u8 pddr_i2c;		/* 0x0F */
	u8 rsvd2;		/* 0x10 */
	u8 pddr_uart;		/* 0x11 */
	u8 pddr_dspi;		/* 0x12 */
	u8 pddr_timer;		/* 0x13 */
	u8 pddr_lcdctl;		/* 0x14 */
	u8 pddr_lcddatah;	/* 0x15 */
	u8 pddr_lcddatam;	/* 0x16 */
	u8 pddr_lcddatal;	/* 0x17 */

	/* Port Pin Data/Set Data Registers */
	u8 ppdsdr_be;		/* 0x18 */
	u8 ppdsdr_cs;		/* 0x19 */
	u8 ppdsdr_fbctl;	/* 0x1A */
	u8 ppdsdr_i2c;		/* 0x1B */
	u8 rsvd3;		/* 0x1C */
	u8 ppdsdr_uart;		/* 0x1D */
	u8 ppdsdr_dspi;		/* 0x1E */
	u8 ppdsdr_timer;	/* 0x1F */
	u8 ppdsdr_lcdctl;	/* 0x20 */
	u8 ppdsdr_lcddatah;	/* 0x21 */
	u8 ppdsdr_lcddatam;	/* 0x22 */
	u8 ppdsdr_lcddatal;	/* 0x23 */

	/* Port Clear Output Data Registers */
	u8 pclrr_be;		/* 0x24 */
	u8 pclrr_cs;		/* 0x25 */
	u8 pclrr_fbctl;		/* 0x26 */
	u8 pclrr_i2c;		/* 0x27 */
	u8 rsvd4;		/* 0x28 */
	u8 pclrr_uart;		/* 0x29 */
	u8 pclrr_dspi;		/* 0x2A */
	u8 pclrr_timer;		/* 0x2B */
	u8 pclrr_lcdctl;	/* 0x2C */
	u8 pclrr_lcddatah;	/* 0x2D */
	u8 pclrr_lcddatam;	/* 0x2E */
	u8 pclrr_lcddatal;	/* 0x2F */

	/* Pin Assignment Registers */
	u8 par_be;		/* 0x30 */
	u8 par_cs;		/* 0x31 */
	u8 par_fbctl;		/* 0x32 */
	u8 par_i2c;		/* 0x33 */
	u16 par_uart;		/* 0x34 */
	u8 par_dspi;		/* 0x36 */
	u8 par_timer;		/* 0x37 */
	u8 par_lcdctl;		/* 0x38 */
	u8 par_irq;		/* 0x39 */
	u16 rsvd6;		/* 0x3A - 0x3B */
	u32 par_lcdh;		/* 0x3C */
	u32 par_lcdl;		/* 0x40 */

	/* Mode select control registers */
	u8 mscr_fb;		/* 0x44 */
	u8 mscr_sdram;		/* 0x45 */

	u16 rsvd7;		/* 0x46 - 0x47 */
	u8 dscr_dspi;		/* 0x48 */
	u8 dscr_timer;		/* 0x49 */
	u8 dscr_i2c;		/* 0x4A */
	u8 dscr_lcd;		/* 0x4B */
	u8 dscr_debug;		/* 0x4C */
	u8 dscr_clkrst;		/* 0x4D */
	u8 dscr_irq;		/* 0x4E */
	u8 dscr_uart;		/* 0x4F */
} gpio_t;

/* SDRAM Controller (SDRAMC) */
typedef struct sdramc {
	u32 sdmr;		/* Mode/Extended Mode */
	u32 sdcr;		/* Control */
	u32 sdcfg1;		/* Configuration 1 */
	u32 sdcfg2;		/* Chip Select */
	u8 resv0[0x100];
	u32 sdcs0;		/* Mode/Extended Mode */
	u32 sdcs1;		/* Mode/Extended Mode */
} sdramc_t;

/* Phase Locked Loop (PLL) */
typedef struct pll {
	u32 pcr;		/* PLL Control */
	u32 psr;		/* PLL Status */
} pll_t;

/* System Control Module register  */
typedef struct scm1 {
	u32 mpr;		/* 0x00 Master Privilege */
	u32 rsvd1[7];
	u32 pacra;		/* 0x20 */
	u32 pacrb;		/* 0x24 */
	u32 pacrc;		/* 0x28 */
	u32 pacrd;		/* 0x2C */
	u32 rsvd2[4];
	u32 pacre;		/* 0x40 */
	u32 pacrf;		/* 0x44 */
	u32 pacrg;		/* 0x48 */
	u32 rsvd3;
	u32 pacri;		/* 0x50 */
} scm1_t;

typedef struct scm2_ctrl {
	u8 res1[3];		/* 0x00 - 0x02 */
	u8 wcr;			/* 0x03 wakeup control */
	u16 res2;		/* 0x04 - 0x05 */
	u16 cwcr;		/* 0x06 Core Watchdog Control */
	u8 res3[3];		/* 0x08 - 0x0A */
	u8 cwsr;		/* 0x0B Core Watchdog Service */
	u8 res4[2];		/* 0x0C - 0x0D */
	u8 scmisr;		/* 0x0F Interrupt Status */
	u32 res5;		/* 0x20 */
	u32 bcr;		/* 0x24 Burst Configuration */
} scm2_t;

typedef struct scm3_ctrl {
	u32 cfadr;		/* 0x00 Core Fault Address */
	u8 res7;		/* 0x04 */
	u8 cfier;		/* 0x05 Core Fault Interrupt Enable */
	u8 cfloc;		/* 0x06 Core Fault Location */
	u8 cfatr;		/* 0x07 Core Fault Attributes */
	u32 cfdtr;		/* 0x08 Core Fault Data */
} scm3_t;

typedef struct rtcex {
	u32 rsvd1[3];
	u32 gocu;
	u32 gocl;
} rtcex_t;
#endif				/* __IMMAP_5227X__ */
