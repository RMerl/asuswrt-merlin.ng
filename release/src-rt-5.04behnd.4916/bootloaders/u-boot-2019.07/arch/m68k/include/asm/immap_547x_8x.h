/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF547x_8x Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __IMMAP_547x_8x__
#define __IMMAP_547x_8x__

#define MMAP_SIU	(CONFIG_SYS_MBAR + 0x00000000)
#define MMAP_SDRAM	(CONFIG_SYS_MBAR + 0x00000100)
#define MMAP_XARB	(CONFIG_SYS_MBAR + 0x00000240)
#define MMAP_FBCS	(CONFIG_SYS_MBAR + 0x00000500)
#define MMAP_INTC0	(CONFIG_SYS_MBAR + 0x00000700)
#define MMAP_GPTMR	(CONFIG_SYS_MBAR + 0x00000800)
#define MMAP_SLT0	(CONFIG_SYS_MBAR + 0x00000900)
#define MMAP_SLT1	(CONFIG_SYS_MBAR + 0x00000910)
#define MMAP_GPIO	(CONFIG_SYS_MBAR + 0x00000A00)
#define MMAP_PCI	(CONFIG_SYS_MBAR + 0x00000B00)
#define MMAP_PCIARB	(CONFIG_SYS_MBAR + 0x00000C00)
#define MMAP_EXTDMA	(CONFIG_SYS_MBAR + 0x00000D00)
#define MMAP_EPORT	(CONFIG_SYS_MBAR + 0x00000F00)
#define MMAP_CTM	(CONFIG_SYS_MBAR + 0x00007F00)
#define MMAP_MCDMA	(CONFIG_SYS_MBAR + 0x00008000)
#define MMAP_SCPCI	(CONFIG_SYS_MBAR + 0x00008400)
#define MMAP_UART0	(CONFIG_SYS_MBAR + 0x00008600)
#define MMAP_UART1	(CONFIG_SYS_MBAR + 0x00008700)
#define MMAP_UART2	(CONFIG_SYS_MBAR + 0x00008800)
#define MMAP_UART3	(CONFIG_SYS_MBAR + 0x00008900)
#define MMAP_DSPI	(CONFIG_SYS_MBAR + 0x00008A00)
#define MMAP_I2C	(CONFIG_SYS_MBAR + 0x00008F00)
#define MMAP_FEC0	(CONFIG_SYS_MBAR + 0x00009000)
#define MMAP_FEC1	(CONFIG_SYS_MBAR + 0x00009800)
#define MMAP_CAN0	(CONFIG_SYS_MBAR + 0x0000A000)
#define MMAP_CAN1	(CONFIG_SYS_MBAR + 0x0000A800)
#define MMAP_USBD	(CONFIG_SYS_MBAR + 0x0000B000)
#define MMAP_SRAM	(CONFIG_SYS_MBAR + 0x00010000)
#define MMAP_SRAMCFG	(CONFIG_SYS_MBAR + 0x0001FF00)
#define MMAP_SEC	(CONFIG_SYS_MBAR + 0x00020000)

#include <asm/coldfire/dspi.h>
#include <asm/coldfire/eport.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/flexcan.h>
#include <asm/coldfire/intctrl.h>

typedef struct siu {
	u32 mbar;		/* 0x00 */
	u32 drv;		/* 0x04 */
	u32 rsvd1[2];		/* 0x08 - 0x1F */
	u32 sbcr;		/* 0x10 */
	u32 rsvd2[3];		/* 0x14 - 0x1F */
	u32 cs0cfg;		/* 0x20 */
	u32 cs1cfg;		/* 0x24 */
	u32 cs2cfg;		/* 0x28 */
	u32 cs3cfg;		/* 0x2C */
	u32 rsvd3[2];		/* 0x30 - 0x37 */
	u32 secsacr;		/* 0x38 */
	u32 rsvd4[2];		/* 0x3C - 0x43 */
	u32 rsr;		/* 0x44 */
	u32 rsvd5[2];		/* 0x48 - 0x4F */
	u32 jtagid;		/* 0x50 */
} siu_t;

typedef struct sdram {
	u32 mode;		/* 0x00 */
	u32 ctrl;		/* 0x04 */
	u32 cfg1;		/* 0x08 */
	u32 cfg2;		/* 0x0c */
} sdram_t;

typedef struct xlb_arb {
	u32 cfg;		/* 0x240 */
	u32 ver;		/* 0x244 */
	u32 sr;			/* 0x248 */
	u32 imr;		/* 0x24c */
	u32 adrcap;		/* 0x250 */
	u32 sigcap;		/* 0x254 */
	u32 adrto;		/* 0x258 */
	u32 datto;		/* 0x25c */
	u32 busto;		/* 0x260 */
	u32 prien;		/* 0x264 */
	u32 pri;		/* 0x268 */
} xlbarb_t;

typedef struct gptmr {
	u8 ocpw;
	u8 octict;
	u8 ctrl;
	u8 mode;

	u16 pre;		/* Prescale */
	u16 cnt;

	u16 pwmwidth;
	u8 pwmop;		/* Output Polarity */
	u8 pwmld;		/* Immediate Update */

	u16 cap;		/* Capture internal counter */
	u8 ovfpin;		/* Ovf and Pin */
	u8 intr;		/* Interrupts */
} gptmr_t;

typedef struct canex_ctrl {
	can_msg_t msg[16];	/* 0x00 Message Buffer 0-15 */
} canex_t;


typedef struct slt {
	u32 tcnt;		/* 0x00 */
	u32 cr;			/* 0x04 */
	u32 cnt;		/* 0x08 */
	u32 sr;			/* 0x0C */
} slt_t;

typedef struct gpio {
	/* Port Output Data Registers */
	u8 podr_fbctl;		/*0x00 */
	u8 podr_fbcs;		/*0x01 */
	u8 podr_dma;		/*0x02 */
	u8 rsvd1;		/*0x03 */
	u8 podr_fec0h;		/*0x04 */
	u8 podr_fec0l;		/*0x05 */
	u8 podr_fec1h;		/*0x06 */
	u8 podr_fec1l;		/*0x07 */
	u8 podr_feci2c;		/*0x08 */
	u8 podr_pcibg;		/*0x09 */
	u8 podr_pcibr;		/*0x0A */
	u8 rsvd2;		/*0x0B */
	u8 podr_psc3psc2;	/*0x0C */
	u8 podr_psc1psc0;	/*0x0D */
	u8 podr_dspi;		/*0x0E */
	u8 rsvd3;		/*0x0F */

	/* Port Data Direction Registers */
	u8 pddr_fbctl;		/*0x10 */
	u8 pddr_fbcs;		/*0x11 */
	u8 pddr_dma;		/*0x12 */
	u8 rsvd4;		/*0x13 */
	u8 pddr_fec0h;		/*0x14 */
	u8 pddr_fec0l;		/*0x15 */
	u8 pddr_fec1h;		/*0x16 */
	u8 pddr_fec1l;		/*0x17 */
	u8 pddr_feci2c;		/*0x18 */
	u8 pddr_pcibg;		/*0x19 */
	u8 pddr_pcibr;		/*0x1A */
	u8 rsvd5;		/*0x1B */
	u8 pddr_psc3psc2;	/*0x1C */
	u8 pddr_psc1psc0;	/*0x1D */
	u8 pddr_dspi;		/*0x1E */
	u8 rsvd6;		/*0x1F */

	/* Port Pin Data/Set Data Registers */
	u8 ppdsdr_fbctl;	/*0x20 */
	u8 ppdsdr_fbcs;		/*0x21 */
	u8 ppdsdr_dma;		/*0x22 */
	u8 rsvd7;		/*0x23 */
	u8 ppdsdr_fec0h;	/*0x24 */
	u8 ppdsdr_fec0l;	/*0x25 */
	u8 ppdsdr_fec1h;	/*0x26 */
	u8 ppdsdr_fec1l;	/*0x27 */
	u8 ppdsdr_feci2c;	/*0x28 */
	u8 ppdsdr_pcibg;	/*0x29 */
	u8 ppdsdr_pcibr;	/*0x2A */
	u8 rsvd8;		/*0x2B */
	u8 ppdsdr_psc3psc2;	/*0x2C */
	u8 ppdsdr_psc1psc0;	/*0x2D */
	u8 ppdsdr_dspi;		/*0x2E */
	u8 rsvd9;		/*0x2F */

	/* Port Clear Output Data Registers */
	u8 pclrr_fbctl;		/*0x30 */
	u8 pclrr_fbcs;		/*0x31 */
	u8 pclrr_dma;		/*0x32 */
	u8 rsvd10;		/*0x33 */
	u8 pclrr_fec0h;		/*0x34 */
	u8 pclrr_fec0l;		/*0x35 */
	u8 pclrr_fec1h;		/*0x36 */
	u8 pclrr_fec1l;		/*0x37 */
	u8 pclrr_feci2c;	/*0x38 */
	u8 pclrr_pcibg;		/*0x39 */
	u8 pclrr_pcibr;		/*0x3A */
	u8 rsvd11;		/*0x3B */
	u8 pclrr_psc3psc2;	/*0x3C */
	u8 pclrr_psc1psc0;	/*0x3D */
	u8 pclrr_dspi;		/*0x3E */
	u8 rsvd12;		/*0x3F */

	/* Pin Assignment Registers */
	u16 par_fbctl;		/*0x40 */
	u8 par_fbcs;		/*0x42 */
	u8 par_dma;		/*0x43 */
	u16 par_feci2cirq;	/*0x44 */
	u16 rsvd13;		/*0x46 */
	u16 par_pcibg;		/*0x48 */
	u16 par_pcibr;		/*0x4A */
	u8 par_psc3;		/*0x4C */
	u8 par_psc2;		/*0x4D */
	u8 par_psc1;		/*0x4E */
	u8 par_psc0;		/*0x4F */
	u16 par_dspi;		/*0x50 */
	u8 par_timer;		/*0x52 */
	u8 rsvd14;		/*0x53 */
} gpio_t;

typedef struct pci {
	u32 idr;		/* 0x00 Device Id / Vendor Id */
	u32 scr;		/* 0x04 Status / command */
	u32 ccrir;		/* 0x08 Class Code / Revision Id */
	u32 cr1;		/* 0x0c Configuration 1 */
	u32 bar0;		/* 0x10 Base address register 0 */
	u32 bar1;		/* 0x14 Base address register 1 */
	u32 bar2;		/* 0x18 NA */
	u32 bar3;		/* 0x1c NA */
	u32 bar4;		/* 0x20 NA */
	u32 bar5;		/* 0x24 NA */
	u32 ccpr;		/* 0x28 Cardbus CIS Pointer */
	u32 sid;		/* 0x2c Subsystem ID / Subsystem Vendor ID */
	u32 erbar;		/* 0x30 Expansion ROM Base Address */
	u32 cpr;		/* 0x34 Capabilities Pointer */
	u32 rsvd1;		/* 0x38 */
	u32 cr2;		/* 0x3c Configuration 2 */
	u32 rsvd2[8];		/* 0x40 - 0x5f */

	/* General control / status registers */
	u32 gscr;		/* 0x60 Global Status / Control */
	u32 tbatr0a;		/* 0x64 Target Base Adr Translation 0 */
	u32 tbatr1a;		/* 0x68 Target Base Adr Translation 1 */
	u32 tcr1;		/* 0x6c Target Control 1 Register */
	u32 iw0btar;		/* 0x70 Initiator Win 0 Base/Translation adr */
	u32 iw1btar;		/* 0x74 Initiator Win 1 Base/Translation adr */
	u32 iw2btar;		/* 0x78 NA */
	u32 rsvd3;		/* 0x7c */
	u32 iwcr;		/* 0x80 Initiator Window Configuration */
	u32 icr;		/* 0x84 Initiator Control */
	u32 isr;		/* 0x88 Initiator Status */
	u32 tcr2;		/* 0x8c NA */
	u32 tbatr0;		/* 0x90 NA */
	u32 tbatr1;		/* 0x94 NA */
	u32 tbatr2;		/* 0x98 NA */
	u32 tbatr3;		/* 0x9c NA */
	u32 tbatr4;		/* 0xa0 NA */
	u32 tbatr5;		/* 0xa4 NA */
	u32 intr;		/* 0xa8 NA */
	u32 rsvd4[19];		/* 0xac - 0xf7 */
	u32 car;		/* 0xf8 Configuration Address */
} pci_t;

typedef struct pci_arbiter {
	/* Pci Arbiter Registers */
	union {
		u32 acr;	/* Arbiter Control */
		u32 asr;	/* Arbiter Status */
	};
} pciarb_t;
#endif				/* __IMMAP_547x_8x__ */
