/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5445x Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __IMMAP_5445X__
#define __IMMAP_5445X__

/* Module Base Addresses */
#define MMAP_SCM1	0xFC000000
#define MMAP_XBS	0xFC004000
#define MMAP_FBCS	0xFC008000
#define MMAP_FEC0	0xFC030000
#define MMAP_FEC1	0xFC034000
#define MMAP_RTC	0xFC03C000
#define MMAP_SCM2	0xFC040000
#define MMAP_EDMA	0xFC044000
#define MMAP_INTC0	0xFC048000
#define MMAP_INTC1	0xFC04C000
#define MMAP_IACK	0xFC054000
#define MMAP_I2C	0xFC058000
#define MMAP_DSPI	0xFC05C000
#define MMAP_UART0	0xFC060000
#define MMAP_UART1	0xFC064000
#define MMAP_UART2	0xFC068000
#define MMAP_DTMR0	0xFC070000
#define MMAP_DTMR1	0xFC074000
#define MMAP_DTMR2	0xFC078000
#define MMAP_DTMR3	0xFC07C000
#define MMAP_PIT0	0xFC080000
#define MMAP_PIT1	0xFC084000
#define MMAP_PIT2	0xFC088000
#define MMAP_PIT3	0xFC08C000
#define MMAP_EPORT	0xFC094000
#define MMAP_WTM	0xFC098000
#define MMAP_SBF	0xFC0A0000
#define MMAP_RCM	0xFC0A0000
#define MMAP_CCM	0xFC0A0000
#define MMAP_GPIO	0xFC0A4000
#define MMAP_PCI	0xFC0A8000
#define MMAP_PCIARB	0xFC0AC000
#define MMAP_RNG	0xFC0B4000
#define MMAP_SDRAM	0xFC0B8000
#define MMAP_SSI	0xFC0BC000
#define MMAP_PLL	0xFC0C4000
#define MMAP_ATA	0x90000000
#define MMAP_USBHW	0xFC0B0000
#define MMAP_USBCAPS	0xFC0B0100
#define MMAP_USBEHCI	0xFC0B0140
#define MMAP_USBOTG	0xFC0B01A0

#include <asm/coldfire/ata.h>
#include <asm/coldfire/crossbar.h>
#include <asm/coldfire/dspi.h>
#include <asm/coldfire/edma.h>
#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/ssi.h>

/* Watchdog Timer Modules (WTM) */
typedef struct wtm {
	u16 wcr;
	u16 wmr;
	u16 wcntr;
	u16 wsr;
} wtm_t;

/* Serial Boot Facility (SBF) */
typedef struct sbf {
	u8 resv0[0x18];
	u16 sbfsr;		/* Serial Boot Facility Status Register */
	u8 resv1[0x6];
	u16 sbfcr;		/* Serial Boot Facility Control Register */
} sbf_t;

/* Reset Controller Module (RCM) */
typedef struct rcm {
	u8 rcr;
	u8 rsr;
} rcm_t;

/* Chip Configuration Module (CCM) */
typedef struct ccm {
	u8 ccm_resv0[0x4];
	u16 ccr;		/* Chip Configuration Register (256 TEPBGA, Read-only) */
	u8 resv1[0x2];
	u16 rcon;		/* Reset Configuration (256 TEPBGA, Read-only) */
	u16 cir;		/* Chip Identification Register (Read-only) */
	u8 resv2[0x4];
	u16 misccr;		/* Miscellaneous Control Register */
	u16 cdr;		/* Clock Divider Register */
	u16 uocsr;		/* USB On-the-Go Controller Status Register */
} ccm_t;

/* General Purpose I/O Module (GPIO) */
typedef struct gpio {
	u8 podr_fec0h;		/* FEC0 High Port Output Data Register */
	u8 podr_fec0l;		/* FEC0 Low Port Output Data Register */
	u8 podr_ssi;		/* SSI Port Output Data Register */
	u8 podr_fbctl;		/* Flexbus Control Port Output Data Register */
	u8 podr_be;		/* Flexbus Byte Enable Port Output Data Register */
	u8 podr_cs;		/* Flexbus Chip-Select Port Output Data Register */
	u8 podr_dma;		/* DMA Port Output Data Register */
	u8 podr_feci2c;		/* FEC1 / I2C Port Output Data Register */
	u8 resv0[0x1];
	u8 podr_uart;		/* UART Port Output Data Register */
	u8 podr_dspi;		/* DSPI Port Output Data Register */
	u8 podr_timer;		/* Timer Port Output Data Register */
	u8 podr_pci;		/* PCI Port Output Data Register */
	u8 podr_usb;		/* USB Port Output Data Register */
	u8 podr_atah;		/* ATA High Port Output Data Register */
	u8 podr_atal;		/* ATA Low Port Output Data Register */
	u8 podr_fec1h;		/* FEC1 High Port Output Data Register */
	u8 podr_fec1l;		/* FEC1 Low Port Output Data Register */
	u8 resv1[0x2];
	u8 podr_fbadh;		/* Flexbus AD High Port Output Data Register */
	u8 podr_fbadmh;		/* Flexbus AD Med-High Port Output Data Register */
	u8 podr_fbadml;		/* Flexbus AD Med-Low Port Output Data Register */
	u8 podr_fbadl;		/* Flexbus AD Low Port Output Data Register */
	u8 pddr_fec0h;		/* FEC0 High Port Data Direction Register */
	u8 pddr_fec0l;		/* FEC0 Low Port Data Direction Register */
	u8 pddr_ssi;		/* SSI Port Data Direction Register */
	u8 pddr_fbctl;		/* Flexbus Control Port Data Direction Register */
	u8 pddr_be;		/* Flexbus Byte Enable Port Data Direction Register */
	u8 pddr_cs;		/* Flexbus Chip-Select Port Data Direction Register */
	u8 pddr_dma;		/* DMA Port Data Direction Register */
	u8 pddr_feci2c;		/* FEC1 / I2C Port Data Direction Register */
	u8 resv2[0x1];
	u8 pddr_uart;		/* UART Port Data Direction Register */
	u8 pddr_dspi;		/* DSPI Port Data Direction Register */
	u8 pddr_timer;		/* Timer Port Data Direction Register */
	u8 pddr_pci;		/* PCI Port Data Direction Register */
	u8 pddr_usb;		/* USB Port Data Direction Register */
	u8 pddr_atah;		/* ATA High Port Data Direction Register */
	u8 pddr_atal;		/* ATA Low Port Data Direction Register */
	u8 pddr_fec1h;		/* FEC1 High Port Data Direction Register */
	u8 pddr_fec1l;		/* FEC1 Low Port Data Direction Register */
	u8 resv3[0x2];
	u8 pddr_fbadh;		/* Flexbus AD High Port Data Direction Register */
	u8 pddr_fbadmh;		/* Flexbus AD Med-High Port Data Direction Register */
	u8 pddr_fbadml;		/* Flexbus AD Med-Low Port Data Direction Register */
	u8 pddr_fbadl;		/* Flexbus AD Low Port Data Direction Register */
	u8 ppdsdr_fec0h;	/* FEC0 High Port Pin Data/Set Data Register */
	u8 ppdsdr_fec0l;	/* FEC0 Low Port Clear Output Data Register */
	u8 ppdsdr_ssi;		/* SSI Port Pin Data/Set Data Register */
	u8 ppdsdr_fbctl;	/* Flexbus Control Port Pin Data/Set Data Register */
	u8 ppdsdr_be;		/* Flexbus Byte Enable Port Pin Data/Set Data Register */
	u8 ppdsdr_cs;		/* Flexbus Chip-Select Port Pin Data/Set Data Register */
	u8 ppdsdr_dma;		/* DMA Port Pin Data/Set Data Register */
	u8 ppdsdr_feci2c;	/* FEC1 / I2C Port Pin Data/Set Data Register */
	u8 resv4[0x1];
	u8 ppdsdr_uart;		/* UART Port Pin Data/Set Data Register */
	u8 ppdsdr_dspi;		/* DSPI Port Pin Data/Set Data Register */
	u8 ppdsdr_timer;	/* FTimer Port Pin Data/Set Data Register */
	u8 ppdsdr_pci;		/* PCI Port Pin Data/Set Data Register */
	u8 ppdsdr_usb;		/* USB Port Pin Data/Set Data Register */
	u8 ppdsdr_atah;		/* ATA High Port Pin Data/Set Data Register */
	u8 ppdsdr_atal;		/* ATA Low Port Pin Data/Set Data Register */
	u8 ppdsdr_fec1h;	/* FEC1 High Port Pin Data/Set Data Register */
	u8 ppdsdr_fec1l;	/* FEC1 Low Port Pin Data/Set Data Register */
	u8 resv5[0x2];
	u8 ppdsdr_fbadh;	/* Flexbus AD High Port Pin Data/Set Data Register */
	u8 ppdsdr_fbadmh;	/* Flexbus AD Med-High Port Pin Data/Set Data Register */
	u8 ppdsdr_fbadml;	/* Flexbus AD Med-Low Port Pin Data/Set Data Register */
	u8 ppdsdr_fbadl;	/* Flexbus AD Low Port Pin Data/Set Data Register */
	u8 pclrr_fec0h;		/* FEC0 High Port Clear Output Data Register */
	u8 pclrr_fec0l;		/* FEC0 Low Port Pin Data/Set Data Register */
	u8 pclrr_ssi;		/* SSI Port Clear Output Data Register */
	u8 pclrr_fbctl;		/* Flexbus Control Port Clear Output Data Register */
	u8 pclrr_be;		/* Flexbus Byte Enable Port Clear Output Data Register */
	u8 pclrr_cs;		/* Flexbus Chip-Select Port Clear Output Data Register */
	u8 pclrr_dma;		/* DMA Port Clear Output Data Register */
	u8 pclrr_feci2c;	/* FEC1 / I2C Port Clear Output Data Register */
	u8 resv6[0x1];
	u8 pclrr_uart;		/* UART Port Clear Output Data Register */
	u8 pclrr_dspi;		/* DSPI Port Clear Output Data Register */
	u8 pclrr_timer;		/* Timer Port Clear Output Data Register */
	u8 pclrr_pci;		/* PCI Port Clear Output Data Register */
	u8 pclrr_usb;		/* USB Port Clear Output Data Register */
	u8 pclrr_atah;		/* ATA High Port Clear Output Data Register */
	u8 pclrr_atal;		/* ATA Low Port Clear Output Data Register */
	u8 pclrr_fec1h;		/* FEC1 High Port Clear Output Data Register */
	u8 pclrr_fec1l;		/* FEC1 Low Port Clear Output Data Register */
	u8 resv7[0x2];
	u8 pclrr_fbadh;		/* Flexbus AD High Port Clear Output Data Register */
	u8 pclrr_fbadmh;	/* Flexbus AD Med-High Port Clear Output Data Register */
	u8 pclrr_fbadml;	/* Flexbus AD Med-Low Port Clear Output Data Register */
	u8 pclrr_fbadl;		/* Flexbus AD Low Port Clear Output Data Register */
	u8 par_fec;		/* FEC Pin Assignment Register */
	u8 par_dma;		/* DMA Pin Assignment Register */
	u8 par_fbctl;		/* Flexbus Control Pin Assignment Register */
	u8 par_dspi;		/* DSPI Pin Assignment Register */
	u8 par_be;		/* Flexbus Byte-Enable Pin Assignment Register */
	u8 par_cs;		/* Flexbus Chip-Select Pin Assignment Register */
	u8 par_timer;		/* Time Pin Assignment Register */
	u8 par_usb;		/* USB Pin Assignment Register */
	u8 resv8[0x1];
	u8 par_uart;		/* UART Pin Assignment Register */
	u16 par_feci2c;		/* FEC / I2C Pin Assignment Register */
	u16 par_ssi;		/* SSI Pin Assignment Register */
	u16 par_ata;		/* ATA Pin Assignment Register */
	u8 par_irq;		/* IRQ Pin Assignment Register */
	u8 resv9[0x1];
	u16 par_pci;		/* PCI Pin Assignment Register */
	u8 mscr_sdram;		/* SDRAM Mode Select Control Register */
	u8 mscr_pci;		/* PCI Mode Select Control Register */
	u8 resv10[0x2];
	u8 dscr_i2c;		/* I2C Drive Strength Control Register */
	u8 dscr_flexbus;	/* FLEXBUS Drive Strength Control Register */
	u8 dscr_fec;		/* FEC Drive Strength Control Register */
	u8 dscr_uart;		/* UART Drive Strength Control Register */
	u8 dscr_dspi;		/* DSPI Drive Strength Control Register */
	u8 dscr_timer;		/* TIMER Drive Strength Control Register */
	u8 dscr_ssi;		/* SSI Drive Strength Control Register */
	u8 dscr_dma;		/* DMA Drive Strength Control Register */
	u8 dscr_debug;		/* DEBUG Drive Strength Control Register */
	u8 dscr_reset;		/* RESET Drive Strength Control Register */
	u8 dscr_irq;		/* IRQ Drive Strength Control Register */
	u8 dscr_usb;		/* USB Drive Strength Control Register */
	u8 dscr_ata;		/* ATA Drive Strength Control Register */
} gpio_t;

/* SDRAM Controller (SDRAMC) */
typedef struct sdramc {
	u32 sdmr;		/* SDRAM Mode/Extended Mode Register */
	u32 sdcr;		/* SDRAM Control Register */
	u32 sdcfg1;		/* SDRAM Configuration Register 1 */
	u32 sdcfg2;		/* SDRAM Chip Select Register */
	u8 resv0[0x100];
	u32 sdcs0;		/* SDRAM Mode/Extended Mode Register */
	u32 sdcs1;		/* SDRAM Mode/Extended Mode Register */
} sdramc_t;

/* Phase Locked Loop (PLL) */
typedef struct pll {
	u32 pcr;		/* PLL Control Register */
	u32 psr;		/* PLL Status Register */
} pll_t;

typedef struct pci {
	u32 idr;		/* 0x00 Device Id / Vendor Id Register */
	u32 scr;		/* 0x04 Status / command Register */
	u32 ccrir;		/* 0x08 Class Code / Revision Id Register */
	u32 cr1;		/* 0x0c Configuration 1 Register */
	u32 bar0;		/* 0x10 Base address register 0 Register */
	u32 bar1;		/* 0x14 Base address register 1 Register */
	u32 bar2;		/* 0x18 Base address register 2 Register */
	u32 bar3;		/* 0x1c Base address register 3 Register */
	u32 bar4;		/* 0x20 Base address register 4 Register */
	u32 bar5;		/* 0x24 Base address register 5 Register */
	u32 ccpr;		/* 0x28 Cardbus CIS Pointer Register */
	u32 sid;		/* 0x2c Subsystem ID / Subsystem Vendor ID Register */
	u32 erbar;		/* 0x30 Expansion ROM Base Address Register */
	u32 cpr;		/* 0x34 Capabilities Pointer Register */
	u32 rsvd1;		/* 0x38 */
	u32 cr2;		/* 0x3c Configuration Register 2 */
	u32 rsvd2[8];		/* 0x40 - 0x5f */

	/* General control / status registers */
	u32 gscr;		/* 0x60 Global Status / Control Register */
	u32 tbatr0a;		/* 0x64 Target Base Address Translation Register  0 */
	u32 tbatr1a;		/* 0x68 Target Base Address Translation Register  1 */
	u32 tcr1;		/* 0x6c Target Control 1 Register */
	u32 iw0btar;		/* 0x70 Initiator Window 0 Base/Translation addr */
	u32 iw1btar;		/* 0x74 Initiator Window 1 Base/Translation addr */
	u32 iw2btar;		/* 0x78 Initiator Window 2 Base/Translation addr */
	u32 rsvd3;		/* 0x7c */
	u32 iwcr;		/* 0x80 Initiator Window Configuration Register */
	u32 icr;		/* 0x84 Initiator Control Register */
	u32 isr;		/* 0x88 Initiator Status Register */
	u32 tcr2;		/* 0x8c Target Control 2 Register */
	u32 tbatr0;		/* 0x90 Target Base Address Translation Register  0 */
	u32 tbatr1;		/* 0x94 Target Base Address Translation Register  1 */
	u32 tbatr2;		/* 0x98 Target Base Address Translation Register  2 */
	u32 tbatr3;		/* 0x9c Target Base Address Translation Register  3 */
	u32 tbatr4;		/* 0xa0 Target Base Address Translation Register  4 */
	u32 tbatr5;		/* 0xa4 Target Base Address Translation Register  5 */
	u32 intr;		/* 0xa8 Interrupt Register */
	u32 rsvd4[19];		/* 0xac - 0xf7 */
	u32 car;		/* 0xf8 Configuration Address Register */
} pci_t;

typedef struct pci_arbiter {
	/* Pci Arbiter Registers */
	union {
		u32 acr;	/* Arbiter Control Register */
		u32 asr;	/* Arbiter Status Register */
	};
} pciarb_t;

/* Register read/write struct */
typedef struct scm1 {
	u32 mpr;		/* 0x00 Master Privilege Register */
	u32 rsvd1[7];
	u32 pacra;		/* 0x20 Peripheral Access Control Register A */
	u32 pacrb;		/* 0x24 Peripheral Access Control Register B */
	u32 pacrc;		/* 0x28 Peripheral Access Control Register C */
	u32 pacrd;		/* 0x2C Peripheral Access Control Register D */
	u32 rsvd2[4];
	u32 pacre;		/* 0x40 Peripheral Access Control Register E */
	u32 pacrf;		/* 0x44 Peripheral Access Control Register F */
	u32 pacrg;		/* 0x48 Peripheral Access Control Register G */
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

typedef struct rtcex {
	u32 rsvd1[3];
	u32 gocu;
	u32 gocl;
} rtcex_t;
#endif				/* __IMMAP_5445X__ */
