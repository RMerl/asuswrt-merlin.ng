/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5301x Internal Memory Map
 *
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __IMMAP_5301X__
#define __IMMAP_5301X__

#define MMAP_SCM1	(CONFIG_SYS_MBAR + 0x00000000)
#define MMAP_XBS	(CONFIG_SYS_MBAR + 0x00004000)
#define MMAP_FBCS	(CONFIG_SYS_MBAR + 0x00008000)
#define MMAP_MPU	(CONFIG_SYS_MBAR + 0x00014000)
#define MMAP_FEC0	(CONFIG_SYS_MBAR + 0x00030000)
#define MMAP_FEC1	(CONFIG_SYS_MBAR + 0x00034000)
#define MMAP_SCM2	(CONFIG_SYS_MBAR + 0x00040000)
#define MMAP_EDMA	(CONFIG_SYS_MBAR + 0x00044000)
#define MMAP_INTC0	(CONFIG_SYS_MBAR + 0x00048000)
#define MMAP_INTC1	(CONFIG_SYS_MBAR + 0x0004C000)
#define MMAP_INTCACK	(CONFIG_SYS_MBAR + 0x00054000)
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
#define MMAP_PIT2	(CONFIG_SYS_MBAR + 0x00088000)
#define MMAP_PIT3	(CONFIG_SYS_MBAR + 0x0008C000)
#define MMAP_EPORT0	(CONFIG_SYS_MBAR + 0x00090000)
#define MMAP_EPORT1	(CONFIG_SYS_MBAR + 0x00094000)
#define MMAP_VOICOD	(CONFIG_SYS_MBAR + 0x0009C000)
#define MMAP_RCM	(CONFIG_SYS_MBAR + 0x000A0000)
#define MMAP_CCM	(CONFIG_SYS_MBAR + 0x000A0004)
#define MMAP_GPIO	(CONFIG_SYS_MBAR + 0x000A4000)
#define MMAP_RTC	(CONFIG_SYS_MBAR + 0x000A8000)
#define MMAP_SIM	(CONFIG_SYS_MBAR + 0x000AC000)
#define MMAP_USBOTG	(CONFIG_SYS_MBAR + 0x000B0000)
#define MMAP_USBH	(CONFIG_SYS_MBAR + 0x000B4000)
#define MMAP_SDRAM	(CONFIG_SYS_MBAR + 0x000B8000)
#define MMAP_SSI	(CONFIG_SYS_MBAR + 0x000BC000)
#define MMAP_PLL	(CONFIG_SYS_MBAR + 0x000C0000)
#define MMAP_RNG	(CONFIG_SYS_MBAR + 0x000C4000)
#define MMAP_IIM	(CONFIG_SYS_MBAR + 0x000C8000)
#define MMAP_ESDHC	(CONFIG_SYS_MBAR + 0x000CC000)

#include <asm/coldfire/crossbar.h>
#include <asm/coldfire/dspi.h>
#include <asm/coldfire/edma.h>
#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/ssi.h>
#include <asm/coldfire/rng.h>
#include <asm/rtc.h>

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
	u32 pacrg;		/* 0x48 Peripheral Access Ctrl G */
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
	u32 rsvd5;		/* 0x20 - 0x23 */
	u8 bcr;			/* 0x24 */
	u8 rsvd6[74];		/* 0x25 - 0x6F */
	u32 cfadr;		/* 0x70 */
	u8 rsvd7;		/* 0x74 */
	u8 cfier;		/* 0x75 */
	u8 cfloc;		/* 0x76 */
	u8 cfatr;		/* 0x77 */
	u32 rsvd8;		/* 0x78 - 0x7B */
	u32 cfdtr;		/* 0x7C */
} scm2_t;

/* PWM module */
typedef struct pwm_ctrl {
	u8 en;			/* 0x00 PWM Enable */
	u8 pol;			/* 0x01 Polarity */
	u8 clk;			/* 0x02 Clock Select */
	u8 prclk;		/* 0x03 Prescale Clock Select */
	u8 cae;			/* 0x04 Center Align Enable */
	u8 ctl;			/* 0x05 Ctrl */
	u8 res1[2];		/* 0x06 - 0x07 */
	u8 scla;		/* 0x08 Scale A */
	u8 sclb;		/* 0x09 Scale B */
	u8 res2[2];		/* 0x0A - 0x0B */
	u8 cnt0;		/* 0x0C Channel 0 Counter */
	u8 cnt1;		/* 0x0D Channel 1 Counter */
	u8 cnt2;		/* 0x0E Channel 2 Counter */
	u8 cnt3;		/* 0x0F Channel 3 Counter */
	u8 cnt4;		/* 0x10 Channel 4 Counter */
	u8 cnt5;		/* 0x11 Channel 5 Counter */
	u8 cnt6;		/* 0x12 Channel 6 Counter */
	u8 cnt7;		/* 0x13 Channel 7 Counter */
	u8 per0;		/* 0x14 Channel 0 Period */
	u8 per1;		/* 0x15 Channel 1 Period */
	u8 per2;		/* 0x16 Channel 2 Period */
	u8 per3;		/* 0x17 Channel 3 Period */
	u8 per4;		/* 0x18 Channel 4 Period */
	u8 per5;		/* 0x19 Channel 5 Period */
	u8 per6;		/* 0x1A Channel 6 Period */
	u8 per7;		/* 0x1B Channel 7 Period */
	u8 dty0;		/* 0x1C Channel 0 Duty */
	u8 dty1;		/* 0x1D Channel 1 Duty */
	u8 dty2;		/* 0x1E Channel 2 Duty */
	u8 dty3;		/* 0x1F Channel 3 Duty */
	u8 dty4;		/* 0x20 Channel 4 Duty */
	u8 dty5;		/* 0x21 Channel 5 Duty */
	u8 dty6;		/* 0x22 Channel 6 Duty */
	u8 dty7;		/* 0x23 Channel 7 Duty */
	u8 sdn;			/* 0x24 Shutdown */
	u8 res3[3];		/* 0x25 - 0x27 */
} pwm_t;

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
	u32 res2;		/* 0x08 */
	u16 misccr;		/* 0x0A Misc Ctrl */
	u16 cdr;		/* 0x0C Clock divider */
	u16 uhcsr;		/* 0x10 USB Host status */
	u16 uocsr;		/* 0x12 USB On-the-Go Status */
	u16 res3;		/* 0x14 */
	u16 codeccr;		/* 0x16 Codec Control */
	u16 misccr2;		/* 0x18 Misc2 Ctrl */
} ccm_t;

/* GPIO port */
typedef struct gpio_ctrl {
	/* Port Output Data */
	u8 podr_fbctl;		/* 0x00 */
	u8 podr_be;		/* 0x01 */
	u8 podr_cs;		/* 0x02 */
	u8 podr_dspi;		/* 0x03 */
	u8 res01;		/* 0x04 */
	u8 podr_fec0;		/* 0x05 */
	u8 podr_feci2c;		/* 0x06 */
	u8 res02[2];		/* 0x07 - 0x08 */
	u8 podr_simp1;		/* 0x09 */
	u8 podr_simp0;		/* 0x0A */
	u8 podr_timer;		/* 0x0B */
	u8 podr_uart;		/* 0x0C */
	u8 podr_debug;		/* 0x0D */
	u8 res03;		/* 0x0E */
	u8 podr_sdhc;		/* 0x0F */
	u8 podr_ssi;		/* 0x10 */
	u8 res04[3];		/* 0x11 - 0x13 */

	/* Port Data Direction */
	u8 pddr_fbctl;		/* 0x14 */
	u8 pddr_be;		/* 0x15 */
	u8 pddr_cs;		/* 0x16 */
	u8 pddr_dspi;		/* 0x17 */
	u8 res05;		/* 0x18 */
	u8 pddr_fec0;		/* 0x19 */
	u8 pddr_feci2c;		/* 0x1A */
	u8 res06[2];		/* 0x1B - 0x1C */
	u8 pddr_simp1;		/* 0x1D */
	u8 pddr_simp0;		/* 0x1E */
	u8 pddr_timer;		/* 0x1F */
	u8 pddr_uart;		/* 0x20 */
	u8 pddr_debug;		/* 0x21 */
	u8 res07;		/* 0x22 */
	u8 pddr_sdhc;		/* 0x23 */
	u8 pddr_ssi;		/* 0x24 */
	u8 res08[3];		/* 0x25 - 0x27 */

	/* Port Data Direction */
	u8 ppdr_fbctl;		/* 0x28 */
	u8 ppdr_be;		/* 0x29 */
	u8 ppdr_cs;		/* 0x2A */
	u8 ppdr_dspi;		/* 0x2B */
	u8 res09;		/* 0x2C */
	u8 ppdr_fec0;		/* 0x2D */
	u8 ppdr_feci2c;		/* 0x2E */
	u8 res10[2];		/* 0x2F - 0x30 */
	u8 ppdr_simp1;		/* 0x31 */
	u8 ppdr_simp0;		/* 0x32 */
	u8 ppdr_timer;		/* 0x33 */
	u8 ppdr_uart;		/* 0x34 */
	u8 ppdr_debug;		/* 0x35 */
	u8 res11;		/* 0x36 */
	u8 ppdr_sdhc;		/* 0x37 */
	u8 ppdr_ssi;		/* 0x38 */
	u8 res12[3];		/* 0x39 - 0x3B */

	/* Port Clear Output Data */
	u8 pclrr_fbctl;		/* 0x3C */
	u8 pclrr_be;		/* 0x3D */
	u8 pclrr_cs;		/* 0x3E */
	u8 pclrr_dspi;		/* 0x3F */
	u8 res13;		/* 0x40 */
	u8 pclrr_fec0;		/* 0x41 */
	u8 pclrr_feci2c;	/* 0x42 */
	u8 res14[2];		/* 0x43 - 0x44 */
	u8 pclrr_simp1;		/* 0x45 */
	u8 pclrr_simp0;		/* 0x46 */
	u8 pclrr_timer;		/* 0x47 */
	u8 pclrr_uart;		/* 0x48 */
	u8 pclrr_debug;		/* 0x49 */
	u8 res15;		/* 0x4A */
	u8 pclrr_sdhc;		/* 0x4B */
	u8 pclrr_ssi;		/* 0x4C */
	u8 res16[3];		/* 0x4D - 0x4F */

	/* Pin Assignment */
	u8 par_fbctl;		/* 0x50 */
	u8 par_be;		/* 0x51 */
	u8 par_cs;		/* 0x52 */
	u8 res17;		/* 0x53 */
	u8 par_dspih;		/* 0x54 */
	u8 par_dspil;		/* 0x55 */
	u8 par_fec;		/* 0x56 */
	u8 par_feci2c;		/* 0x57 */
	u8 par_irq0h;		/* 0x58 */
	u8 par_irq0l;		/* 0x59 */
	u8 par_irq1h;		/* 0x5A */
	u8 par_irq1l;		/* 0x5B */
	u8 par_simp1h;		/* 0x5C */
	u8 par_simp1l;		/* 0x5D */
	u8 par_simp0;		/* 0x5E */
	u8 par_timer;		/* 0x5F */
	u8 par_uart;		/* 0x60 */
	u8 res18;		/* 0x61 */
	u8 par_debug;		/* 0x62 */
	u8 par_sdhc;		/* 0x63 */
	u8 par_ssih;		/* 0x64 */
	u8 par_ssil;		/* 0x65 */
	u8 res19[2];		/* 0x66 - 0x67 */

	/* Mode Select Control */
	/* Drive Strength Control */
	u8 mscr_mscr1;		/* 0x68 */
	u8 mscr_mscr2;		/* 0x69 */
	u8 mscr_mscr3;		/* 0x6A */
	u8 mscr_mscr45;		/* 0x6B */
	u8 srcr_dspi;		/* 0x6C */
	u8 dscr_fec;		/* 0x6D */
	u8 srcr_i2c;		/* 0x6E */
	u8 srcr_irq;		/* 0x6F */

	u8 srcr_sim;		/* 0x70 */
	u8 srcr_timer;		/* 0x71 */
	u8 srcr_uart;		/* 0x72 */
	u8 res20;		/* 0x73 */
	u8 srcr_sdhc;		/* 0x74 */
	u8 srcr_ssi;		/* 0x75 */
	u8 res21[2];		/* 0x76 - 0x77 */
	u8 pcr_pcrh;		/* 0x78 */
	u8 pcr_pcrl;		/* 0x79 */
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
	u32 pcr;		/* 0x00 Ctrl */
	u32 pdr;		/* 0x04 Divider */
	u32 psr;		/* 0x08 Status */
} pll_t;

typedef struct rtcex {
	u32 rsvd1[3];
	u32 gocu;
	u32 gocl;
} rtcex_t;
#endif				/* __IMMAP_5301X__ */
