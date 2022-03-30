/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5329 Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __IMMAP_5235__
#define __IMMAP_5235__

#define MMAP_SCM	(CONFIG_SYS_MBAR + 0x00000000)
#define MMAP_SDRAM	(CONFIG_SYS_MBAR + 0x00000040)
#define MMAP_FBCS	(CONFIG_SYS_MBAR + 0x00000080)
#define MMAP_DMA0	(CONFIG_SYS_MBAR + 0x00000100)
#define MMAP_DMA1	(CONFIG_SYS_MBAR + 0x00000110)
#define MMAP_DMA2	(CONFIG_SYS_MBAR + 0x00000120)
#define MMAP_DMA3	(CONFIG_SYS_MBAR + 0x00000130)
#define MMAP_UART0	(CONFIG_SYS_MBAR + 0x00000200)
#define MMAP_UART1	(CONFIG_SYS_MBAR + 0x00000240)
#define MMAP_UART2	(CONFIG_SYS_MBAR + 0x00000280)
#define MMAP_I2C	(CONFIG_SYS_MBAR + 0x00000300)
#define MMAP_QSPI	(CONFIG_SYS_MBAR + 0x00000340)
#define MMAP_DTMR0	(CONFIG_SYS_MBAR + 0x00000400)
#define MMAP_DTMR1	(CONFIG_SYS_MBAR + 0x00000440)
#define MMAP_DTMR2	(CONFIG_SYS_MBAR + 0x00000480)
#define MMAP_DTMR3	(CONFIG_SYS_MBAR + 0x000004C0)
#define MMAP_INTC0	(CONFIG_SYS_MBAR + 0x00000C00)
#define MMAP_INTC1	(CONFIG_SYS_MBAR + 0x00000D00)
#define MMAP_INTCACK	(CONFIG_SYS_MBAR + 0x00000F00)
#define MMAP_FEC	(CONFIG_SYS_MBAR + 0x00001000)
#define MMAP_FECFIFO	(CONFIG_SYS_MBAR + 0x00001400)
#define MMAP_GPIO	(CONFIG_SYS_MBAR + 0x00100000)
#define MMAP_CCM	(CONFIG_SYS_MBAR + 0x00110000)
#define MMAP_PLL	(CONFIG_SYS_MBAR + 0x00120000)
#define MMAP_EPORT	(CONFIG_SYS_MBAR + 0x00130000)
#define MMAP_WDOG	(CONFIG_SYS_MBAR + 0x00140000)
#define MMAP_PIT0	(CONFIG_SYS_MBAR + 0x00150000)
#define MMAP_PIT1	(CONFIG_SYS_MBAR + 0x00160000)
#define MMAP_PIT2	(CONFIG_SYS_MBAR + 0x00170000)
#define MMAP_PIT3	(CONFIG_SYS_MBAR + 0x00180000)
#define MMAP_MDHA	(CONFIG_SYS_MBAR + 0x00190000)
#define MMAP_RNG	(CONFIG_SYS_MBAR + 0x001A0000)
#define MMAP_SKHA	(CONFIG_SYS_MBAR + 0x001B0000)
#define MMAP_CAN1	(CONFIG_SYS_MBAR + 0x001C0000)
#define MMAP_ETPU	(CONFIG_SYS_MBAR + 0x001D0000)
#define MMAP_CAN2	(CONFIG_SYS_MBAR + 0x001F0000)

#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/flexcan.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/mdha.h>
#include <asm/coldfire/qspi.h>
#include <asm/coldfire/rng.h>
#include <asm/coldfire/skha.h>

/* System Control Module register */
typedef struct scm_ctrl {
	u32 ipsbar;		/* 0x00 - MBAR */
	u32 res1;		/* 0x04 */
	u32 rambar;		/* 0x08 - RAMBAR */
	u32 res2;		/* 0x0C */
	u8 crsr;		/* 0x10 Core Reset Status Register */
	u8 cwcr;		/* 0x11 Core Watchdog Control Register */
	u8 lpicr;		/* 0x12 Low-Power Interrupt Control Register */
	u8 cwsr;		/* 0x13 Core Watchdog Service Register */
	u32 dmareqc;		/* 0x14 */
	u32 res3;		/* 0x18 */
	u32 mpark;		/* 0x1C */
	u8 mpr;			/* 0x20 */
	u8 res4[3];		/* 0x21 - 0x23 */
	u8 pacr0;		/* 0x24 */
	u8 pacr1;		/* 0x25 */
	u8 pacr2;		/* 0x26 */
	u8 pacr3;		/* 0x27 */
	u8 pacr4;		/* 0x28 */
	u32 res5;		/* 0x29 */
	u8 pacr5;		/* 0x2a */
	u8 pacr6;		/* 0x2b */
	u8 pacr7;		/* 0x2c */
	u32 res6;		/* 0x2d */
	u8 pacr8;		/* 0x2e */
	u32 res7;		/* 0x2f */
	u8 gpacr;		/* 0x30 */
	u8 res8[3];		/* 0x31 - 0x33 */
} scm_t;

/* SDRAM controller registers */
typedef struct sdram_ctrl {
	u16 dcr;		/* 0x00 Control register */
	u16 res1[3];		/* 0x02 - 0x07 */
	u32 dacr0;		/* 0x08 address and control register 0 */
	u32 dmr0;		/* 0x0C mask register block 0 */
	u32 dacr1;		/* 0x10 address and control register 1 */
	u32 dmr1;		/* 0x14 mask register block 1 */
} sdram_t;

typedef struct canex_ctrl {
	can_msg_t msg[16];	/* 0x00 Message Buffer 0-15 */
} canex_t;

/* GPIO port registers */
typedef struct gpio_ctrl {
	/* Port Output Data Registers */
	u8 podr_addr;		/* 0x00 */
	u8 podr_datah;		/* 0x01 */
	u8 podr_datal;		/* 0x02 */
	u8 podr_busctl;		/* 0x03 */
	u8 podr_bs;		/* 0x04 */
	u8 podr_cs;		/* 0x05 */
	u8 podr_sdram;		/* 0x06 */
	u8 podr_feci2c;		/* 0x07 */
	u8 podr_uarth;		/* 0x08 */
	u8 podr_uartl;		/* 0x09 */
	u8 podr_qspi;		/* 0x0A */
	u8 podr_timer;		/* 0x0B */
	u8 podr_etpu;		/* 0x0C */
	u8 res1[3];		/* 0x0D - 0x0F */

	/* Port Data Direction Registers */
	u8 pddr_addr;		/* 0x10 */
	u8 pddr_datah;		/* 0x11 */
	u8 pddr_datal;		/* 0x12 */
	u8 pddr_busctl;		/* 0x13 */
	u8 pddr_bs;		/* 0x14 */
	u8 pddr_cs;		/* 0x15 */
	u8 pddr_sdram;		/* 0x16 */
	u8 pddr_feci2c;		/* 0x17 */
	u8 pddr_uarth;		/* 0x18 */
	u8 pddr_uartl;		/* 0x19 */
	u8 pddr_qspi;		/* 0x1A */
	u8 pddr_timer;		/* 0x1B */
	u8 pddr_etpu;		/* 0x1C */
	u8 res2[3];		/* 0x1D - 0x1F */

	/* Port Data Direction Registers */
	u8 ppdsdr_addr;		/* 0x20 */
	u8 ppdsdr_datah;	/* 0x21 */
	u8 ppdsdr_datal;	/* 0x22 */
	u8 ppdsdr_busctl;	/* 0x23 */
	u8 ppdsdr_bs;		/* 0x24 */
	u8 ppdsdr_cs;		/* 0x25 */
	u8 ppdsdr_sdram;	/* 0x26 */
	u8 ppdsdr_feci2c;	/* 0x27 */
	u8 ppdsdr_uarth;	/* 0x28 */
	u8 ppdsdr_uartl;	/* 0x29 */
	u8 ppdsdr_qspi;		/* 0x2A */
	u8 ppdsdr_timer;	/* 0x2B */
	u8 ppdsdr_etpu;		/* 0x2C */
	u8 res3[3];		/* 0x2D - 0x2F */

	/* Port Clear Output Data Registers */
	u8 pclrr_addr;		/* 0x30 */
	u8 pclrr_datah;		/* 0x31 */
	u8 pclrr_datal;		/* 0x32 */
	u8 pclrr_busctl;	/* 0x33 */
	u8 pclrr_bs;		/* 0x34 */
	u8 pclrr_cs;		/* 0x35 */
	u8 pclrr_sdram;		/* 0x36 */
	u8 pclrr_feci2c;	/* 0x37 */
	u8 pclrr_uarth;		/* 0x38 */
	u8 pclrr_uartl;		/* 0x39 */
	u8 pclrr_qspi;		/* 0x3A */
	u8 pclrr_timer;		/* 0x3B */
	u8 pclrr_etpu;		/* 0x3C */
	u8 res4[3];		/* 0x3D - 0x3F */

	/* Pin Assignment Registers */
	u8 par_ad;		/* 0x40 */
	u8 res5;		/* 0x41 */
	u16 par_busctl;		/* 0x42 */
	u8 par_bs;		/* 0x44 */
	u8 par_cs;		/* 0x45 */
	u8 par_sdram;		/* 0x46 */
	u8 par_feci2c;		/* 0x47 */
	u16 par_uart;		/* 0x48 */
	u8 par_qspi;		/* 0x4A */
	u8 res6;		/* 0x4B */
	u16 par_timer;		/* 0x4C */
	u8 par_etpu;		/* 0x4E */
	u8 res7;		/* 0x4F */

	/* Drive Strength Control Registers */
	u8 dscr_eim;		/* 0x50 */
	u8 dscr_etpu;		/* 0x51 */
	u8 dscr_feci2c;		/* 0x52 */
	u8 dscr_uart;		/* 0x53 */
	u8 dscr_qspi;		/* 0x54 */
	u8 dscr_timer;		/* 0x55 */
	u16 res8;		/* 0x56 */
} gpio_t;

/*Chip configuration module registers */
typedef struct ccm_ctrl {
	u8 rcr;			/* 0x01 */
	u8 rsr;			/* 0x02 */
	u16 res1;		/* 0x03 */
	u16 ccr;		/* 0x04 Chip configuration register */
	u16 lpcr;		/* 0x06 Low-power Control register */
	u16 rcon;		/* 0x08 Rreset configuration register */
	u16 cir;		/* 0x0a Chip identification register */
} ccm_t;

/* Clock Module registers */
typedef struct pll_ctrl {
	u32 syncr;		/* 0x00 synthesizer control register */
	u32 synsr;		/* 0x04 synthesizer status register */
} pll_t;

/* Watchdog registers */
typedef struct wdog_ctrl {
	u16 cr;			/* 0x00 Control register */
	u16 mr;			/* 0x02 Modulus register */
	u16 cntr;		/* 0x04 Count register */
	u16 sr;			/* 0x06 Service register */
} wdog_t;

#endif				/* __IMMAP_5235__ */
