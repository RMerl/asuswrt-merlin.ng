/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5441x Internal Memory Map
 *
 * Copyright 2010-2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __IMMAP_5441X__
#define __IMMAP_5441X__

/* Module Base Addresses */
#define MMAP_XBS	0xFC004000
#define MMAP_FBCS	0xFC008000
#define MMAP_CAN0	0xFC020000
#define MMAP_CAN1	0xFC024000
#define MMAP_I2C1	0xFC038000
#define MMAP_DSPI1	0xFC03C000
#define MMAP_SCM	0xFC040000
#define MMAP_PM		0xFC04002C
#define MMAP_EDMA	0xFC044000
#define MMAP_INTC0	0xFC048000
#define MMAP_INTC1	0xFC04C000
#define MMAP_INTC2	0xFC050000
#define MMAP_IACK	0xFC054000
#define MMAP_I2C0	0xFC058000
#define MMAP_DSPI0	0xFC05C000
#define MMAP_UART0	0xFC060000
#define MMAP_UART1	0xFC064000
#define MMAP_UART2	0xFC068000
#define MMAP_UART3	0xFC06C000
#define MMAP_DTMR0	0xFC070000
#define MMAP_DTMR1	0xFC074000
#define MMAP_DTMR2	0xFC078000
#define MMAP_DTMR3	0xFC07C000
#define MMAP_PIT0	0xFC080000
#define MMAP_PIT1	0xFC084000
#define MMAP_PIT2	0xFC088000
#define MMAP_PIT3	0xFC08C000
#define MMAP_EPORT0	0xFC090000
#define MMAP_ADC	0xFC094000
#define MMAP_DAC0	0xFC098000
#define MMAP_DAC1	0xFC09C000
#define MMAP_RRTC	0xFC0A8000
#define MMAP_SIM	0xFC0AC000
#define MMAP_USBOTG	0xFC0B0000
#define MMAP_USBEHCI	0xFC0B4000
#define MMAP_SDRAM	0xFC0B8000
#define MMAP_SSI0	0xFC0BC000
#define MMAP_PLL	0xFC0C0000
#define MMAP_RNG	0xFC0C4000
#define MMAP_SSI1	0xFC0C8000
#define MMAP_ESDHC	0xFC0CC000
#define MMAP_FEC0	0xFC0D4000
#define MMAP_FEC1	0xFC0D8000
#define MMAP_L2_SW0	0xFC0DC000
#define MMAP_L2_SW1	0xFC0E0000

#define MMAP_NFC_RAM	0xFC0FC000
#define MMAP_NFC	0xFC0FF000

#define MMAP_1WIRE	0xEC008000
#define MMAP_I2C2	0xEC010000
#define MMAP_I2C3	0xEC014000
#define MMAP_I2C4	0xEC018000
#define MMAP_I2C5	0xEC01C000
#define MMAP_DSPI2	0xEC038000
#define MMAP_DSPI3	0xEC03C000
#define MMAP_UART4	0xEC060000
#define MMAP_UART5	0xEC064000
#define MMAP_UART6	0xEC068000
#define MMAP_UART7	0xEC06C000
#define MMAP_UART8	0xEC070000
#define MMAP_UART9	0xEC074000
#define MMAP_RCM	0xEC090000
#define MMAP_CCM	0xEC090000
#define MMAP_GPIO	0xEC094000

#include <asm/coldfire/crossbar.h>
#include <asm/coldfire/dspi.h>
#include <asm/coldfire/edma.h>
#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/flexcan.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/ssi.h>

/* Serial Boot Facility (SBF) */
typedef struct sbf {
	u8 resv0[0x18];
	u16 sbfsr;		/* Serial Boot Facility Status */
	u8 resv1[0x6];
	u16 sbfcr;		/* Serial Boot Facility Control */
} sbf_t;

/* Reset Controller Module (RCM) */
typedef struct rcm {
	u8 rcr;
	u8 rsr;
} rcm_t;

/* Chip Configuration Module (CCM) */
typedef struct ccm {
	u8 ccm_resv0[0x4];	/* 0x00 */
	u16 ccr;		/* 0x04 Chip Configuration */
	u8 resv1[0x2];		/* 0x06 */
	u16 rcon;		/* 0x08 Reset Configuration */
	u16 cir;		/* 0x0A Chip Identification */
	u8 resv2[0x2];		/* 0x0C */
	u16 misccr;		/* 0x0E Miscellaneous Control */
	u16 cdrh;		/* 0x10 Clock Divider */
	u16 cdrl;		/* 0x12 Clock Divider */
	u16 uocsr;		/* 0x14 USB On-the-Go Controller Status */
	u16 uhcsr;		/* 0x16 */
	u16 misccr3;		/* 0x18 */
	u16 misccr2;		/* 0x1A */
	u16 adctsr;		/* 0x1C */
	u16 dactsr;		/* 0x1E */
	u16 sbfsr;		/* 0x20 */
	u16 sbfcr;		/* 0x22 */
	u32 fnacr;		/* 0x24 */
} ccm_t;

/* General Purpose I/O Module (GPIO) */
typedef struct gpio {
	u8 podr_a;		/* 0x00 */
	u8 podr_b;		/* 0x01 */
	u8 podr_c;		/* 0x02 */
	u8 podr_d;		/* 0x03 */
	u8 podr_e;		/* 0x04 */
	u8 podr_f;		/* 0x05 */
	u8 podr_g;		/* 0x06 */
	u8 podr_h;		/* 0x07 */
	u8 podr_i;		/* 0x08 */
	u8 podr_j;		/* 0x09 */
	u8 podr_k;		/* 0x0A */
	u8 rsvd0;		/* 0x0B */

	u8 pddr_a;		/* 0x0C */
	u8 pddr_b;		/* 0x0D */
	u8 pddr_c;		/* 0x0E */
	u8 pddr_d;		/* 0x0F */
	u8 pddr_e;		/* 0x10 */
	u8 pddr_f;		/* 0x11 */
	u8 pddr_g;		/* 0x12 */
	u8 pddr_h;		/* 0x13 */
	u8 pddr_i;		/* 0x14 */
	u8 pddr_j;		/* 0x15 */
	u8 pddr_k;		/* 0x16 */
	u8 rsvd1;		/* 0x17 */

	u8 ppdsdr_a;		/* 0x18 */
	u8 ppdsdr_b;		/* 0x19 */
	u8 ppdsdr_c;		/* 0x1A */
	u8 ppdsdr_d;		/* 0x1B */
	u8 ppdsdr_e;		/* 0x1C */
	u8 ppdsdr_f;		/* 0x1D */
	u8 ppdsdr_g;		/* 0x1E */
	u8 ppdsdr_h;		/* 0x1F */
	u8 ppdsdr_i;		/* 0x20 */
	u8 ppdsdr_j;		/* 0x21 */
	u8 ppdsdr_k;		/* 0x22 */
	u8 rsvd2;		/* 0x23 */

	u8 pclrr_a;		/* 0x24 */
	u8 pclrr_b;		/* 0x25 */
	u8 pclrr_c;		/* 0x26 */
	u8 pclrr_d;		/* 0x27 */
	u8 pclrr_e;		/* 0x28 */
	u8 pclrr_f;		/* 0x29 */
	u8 pclrr_g;		/* 0x2A */
	u8 pclrr_h;		/* 0x2B */
	u8 pclrr_i;		/* 0x2C */
	u8 pclrr_j;		/* 0x2D */
	u8 pclrr_k;		/* 0x2E */
	u8 rsvd3;		/* 0x2F */

	u16 pcr_a;		/* 0x30 */
	u16 pcr_b;		/* 0x32 */
	u16 pcr_c;		/* 0x34 */
	u16 pcr_d;		/* 0x36 */
	u16 pcr_e;		/* 0x38 */
	u16 pcr_f;		/* 0x3A */
	u16 pcr_g;		/* 0x3C */
	u16 pcr_h;		/* 0x3E */
	u16 pcr_i;		/* 0x40 */
	u16 pcr_j;		/* 0x42 */
	u16 pcr_k;		/* 0x44 */
	u16 rsvd4;		/* 0x46 */

	u8 par_fbctl;		/* 0x48 */
	u8 par_be;		/* 0x49 */
	u8 par_cs;		/* 0x4A */
	u8 par_cani2c;		/* 0x4B */
	u8 par_irqh;		/* 0x4C */
	u8 par_irql;		/* 0x4D */
	u8 par_dspi0;		/* 0x4E */
	u8 par_dspiow;		/* 0x4F */
	u8 par_timer;		/* 0x50 */
	u8 par_uart2;		/* 0x51 */
	u8 par_uart1;		/* 0x52 */
	u8 par_uart0;		/* 0x53 */
	u8 par_sdhch;		/* 0x54 */
	u8 par_sdhcl;		/* 0x55 */
	u8 par_simp0h;		/* 0x56 */
	u8 par_simp1h;		/* 0x57 */
	u8 par_ssi0h;		/* 0x58 */
	u8 par_ssi0l;		/* 0x59 */
	u8 par_dbg1h;		/* 0x5A */
	u8 par_dbg0h;		/* 0x5B */
	u8 par_dbgl;		/* 0x5C */
	u8 rsvd5;		/* 0x5D */
	u8 par_fec;		/* 0x5E */
	u8 rsvd6;		/* 0x5F */

	u8 mscr_sdram;		/* 0x60 */
	u8 rsvd7[3];		/* 0x61-0x63 */

	u8 srcr_fb1;		/* 0x64 */
	u8 srcr_fb2;		/* 0x65 */
	u8 srcr_fb3;		/* 0x66 */
	u8 srcr_fb4;		/* 0x67 */
	u8 srcr_dspiow;		/* 0x68 */
	u8 srcr_cani2c;		/* 0x69 */
	u8 srcr_irq;		/* 0x6A */
	u8 srcr_timer;		/* 0x6B */
	u8 srcr_uart;		/* 0x6C */
	u8 srcr_fec;		/* 0x6D */
	u8 srcr_sdhc;		/* 0x6E */
	u8 srcr_simp0;		/* 0x6F */
	u8 srcr_ssi0;		/* 0x70 */
	u8 rsvd8[3];		/* 0x71-0x73 */

	u16 urts_pol;		/* 0x74 */
	u16 ucts_pol;		/* 0x76 */
	u16 utxd_wom;		/* 0x78 */
	u32 urxd_wom;		/* 0x7c */

	u32 hcr1;		/* 0x80 */
	u32 hcr0;		/* 0x84 */
} gpio_t;

/* SDRAM Controller (SDRAMC) */
typedef struct sdramc {
	u32 cr00;		/* 0x00 */
	u32 cr01;		/* 0x04 */
	u32 cr02;		/* 0x08 */
	u32 cr03;		/* 0x0C */
	u32 cr04;		/* 0x10 */
	u32 cr05;		/* 0x14 */
	u32 cr06;		/* 0x18 */
	u32 cr07;		/* 0x1C */

	u32 cr08;		/* 0x20 */
	u32 cr09;		/* 0x24 */
	u32 cr10;		/* 0x28 */
	u32 cr11;		/* 0x2C */
	u32 cr12;		/* 0x30 */
	u32 cr13;		/* 0x34 */
	u32 cr14;		/* 0x38 */
	u32 cr15;		/* 0x3C */

	u32 cr16;		/* 0x40 */
	u32 cr17;		/* 0x44 */
	u32 cr18;		/* 0x48 */
	u32 cr19;		/* 0x4C */
	u32 cr20;		/* 0x50 */
	u32 cr21;		/* 0x54 */
	u32 cr22;		/* 0x58 */
	u32 cr23;		/* 0x5C */

	u32 cr24;		/* 0x60 */
	u32 cr25;		/* 0x64 */
	u32 cr26;		/* 0x68 */
	u32 cr27;		/* 0x6C */
	u32 cr28;		/* 0x70 */
	u32 cr29;		/* 0x74 */
	u32 cr30;		/* 0x78 */
	u32 cr31;		/* 0x7C */

	u32 cr32;		/* 0x80 */
	u32 cr33;		/* 0x84 */
	u32 cr34;		/* 0x88 */
	u32 cr35;		/* 0x8C */
	u32 cr36;		/* 0x90 */
	u32 cr37;		/* 0x94 */
	u32 cr38;		/* 0x98 */
	u32 cr39;		/* 0x9C */

	u32 cr40;		/* 0xA0 */
	u32 cr41;		/* 0xA4 */
	u32 cr42;		/* 0xA8 */
	u32 cr43;		/* 0xAC */
	u32 cr44;		/* 0xB0 */
	u32 cr45;		/* 0xB4 */
	u32 cr46;		/* 0xB8 */
	u32 cr47;		/* 0xBC */
	u32 cr48;		/* 0xC0 */
	u32 cr49;		/* 0xC4 */
	u32 cr50;		/* 0xC8 */
	u32 cr51;		/* 0xCC */
	u32 cr52;		/* 0xD0 */
	u32 cr53;		/* 0xD4 */
	u32 cr54;		/* 0xD8 */
	u32 cr55;		/* 0xDC */
	u32 cr56;		/* 0xE0 */
	u32 cr57;		/* 0xE4 */
	u32 cr58;		/* 0xE8 */
	u32 cr59;		/* 0xEC */
	u32 cr60;		/* 0xF0 */
	u32 cr61;		/* 0xF4 */
	u32 cr62;		/* 0xF8 */
	u32 cr63;		/* 0xFC */

	u32 rsvd3[32];		/* 0xF4-0x1A8 */

	u32 rcrcr;		/* 0x180 */
	u32 swrcr;		/* 0x184 */
	u32 rcr;		/* 0x188 */
	u32 msovr;		/* 0x18C */
	u32 rcrdbg;		/* 0x190 */
	u32 sl0adj;		/* 0x194 */
	u32 sl1adj;		/* 0x198 */
	u32 sl2adj;		/* 0x19C */
	u32 sl3adj;		/* 0x1A0 */
	u32 sl4adj;		/* 0x1A4 */
	u32 flight_tm;		/* 0x1A8 */
	u32 padcr;		/* 0x1AC */
} sdramc_t;

/* Phase Locked Loop (PLL) */
typedef struct pll {
	u32 pcr;		/* Control */
	u32 pdr;		/* Divider */
	u32 psr;		/* Status */
} pll_t;

typedef struct scm {
	u8 rsvd1[19];		/* 0x00 - 0x12 */
	u8 wcr;			/* 0x13 */
	u16 rsvd2;		/* 0x14 - 0x15 */
	u16 cwcr;		/* 0x16 */
	u8 rsvd3[3];		/* 0x18 - 0x1A */
	u8 cwsr;		/* 0x1B */
	u8 rsvd4[3];		/* 0x1C - 0x1E */
	u8 scmisr;		/* 0x1F */
	u32 rsvd5;		/* 0x20 - 0x23 */
	u32 bcr;		/* 0x24 */
	u8 rsvd6[72];		/* 0x28 - 0x6F */
	u32 cfadr;		/* 0x70 */
	u8 rsvd7;		/* 0x74 */
	u8 cfier;		/* 0x75 */
	u8 cfloc;		/* 0x76 */
	u8 cfatr;		/* 0x77 */
	u32 rsvd8;		/* 0x78 - 0x7B */
	u32 cfdtr;		/* 0x7C */
} scm_t;

typedef struct pm {
	u8 pmsr0;		/* */
	u8 pmcr0;
	u8 pmsr1;
	u8 pmcr1;
	u32 pmhr0;
	u32 pmlr0;
	u32 pmhr1;
	u32 pmlr1;
} pm_t;

#endif				/* __IMMAP_5441X__ */
