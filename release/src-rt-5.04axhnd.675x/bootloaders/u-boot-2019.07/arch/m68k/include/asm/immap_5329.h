/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MCF5329 Internal Memory Map
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __IMMAP_5329__
#define __IMMAP_5329__

#define MMAP_SCM1	0xEC000000
#define MMAP_MDHA	0xEC080000
#define MMAP_SKHA	0xEC084000
#define MMAP_RNG	0xEC088000
#define MMAP_SCM2	0xFC000000
#define MMAP_XBS	0xFC004000
#define MMAP_FBCS	0xFC008000
#define MMAP_CAN	0xFC020000
#define MMAP_FEC	0xFC030000
#define MMAP_SCM3	0xFC040000
#define MMAP_EDMA	0xFC044000
#define MMAP_TCD	0xFC045000
#define MMAP_INTC0	0xFC048000
#define MMAP_INTC1	0xFC04C000
#define MMAP_INTCACK	0xFC054000
#define MMAP_I2C	0xFC058000
#define MMAP_QSPI	0xFC05C000
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
#define MMAP_PWM	0xFC090000
#define MMAP_EPORT	0xFC094000
#define MMAP_WDOG	0xFC098000
#define MMAP_RCM	0xFC0A0000
#define MMAP_CCM	0xFC0A0004
#define MMAP_GPIO	0xFC0A4000
#define MMAP_RTC	0xFC0A8000
#define MMAP_LCDC	0xFC0AC000
#define MMAP_USBOTG	0xFC0B0000
#define MMAP_USBH	0xFC0B4000
#define MMAP_SDRAM	0xFC0B8000
#define MMAP_SSI	0xFC0BC000
#define MMAP_PLL	0xFC0C0000

#include <asm/coldfire/crossbar.h>
#include <asm/coldfire/edma.h>
#include <asm/coldfire/eport.h>
#include <asm/coldfire/qspi.h>
#include <asm/coldfire/flexbus.h>
#include <asm/coldfire/flexcan.h>
#include <asm/coldfire/intctrl.h>
#include <asm/coldfire/lcd.h>
#include <asm/coldfire/mdha.h>
#include <asm/coldfire/pwm.h>
#include <asm/coldfire/ssi.h>
#include <asm/coldfire/skha.h>

/* System control module registers */
typedef struct scm1_ctrl {
	u32 mpr0;		/* 0x00 Master Privilege Register 0 */
	u32 res1[15];		/* 0x04 - 0x3F */
	u32 pacrh;		/* 0x40 Peripheral Access Control Register H */
	u32 res2[3];		/* 0x44 - 0x53 */
	u32 bmt0;		/*0x54 Bus Monitor Timeout 0 */
} scm1_t;

/* System control module registers 2 */
typedef struct scm2_ctrl {
	u32 mpr1;		/* 0x00 Master Privilege Register */
	u32 res1[7];		/* 0x04 - 0x1F */
	u32 pacra;		/* 0x20 Peripheral Access Control Register A */
	u32 pacrb;		/* 0x24 Peripheral Access Control Register B */
	u32 pacrc;		/* 0x28 Peripheral Access Control Register C */
	u32 pacrd;		/* 0x2C Peripheral Access Control Register D */
	u32 res2[4];		/* 0x30 - 0x3F */
	u32 pacre;		/* 0x40 Peripheral Access Control Register E */
	u32 pacrf;		/* 0x44 Peripheral Access Control Register F */
	u32 pacrg;		/* 0x48 Peripheral Access Control Register G */
	u32 res3[2];		/* 0x4C - 0x53 */
	u32 bmt1;		/* 0x54 Bus Monitor Timeout 1 */
} scm2_t;

/* System Control Module register 3 */
typedef struct scm3_ctrl {
	u8 res1[19];		/* 0x00 - 0x12 */
	u8 wcr;			/* 0x13 wakeup control register */
	u16 res2;		/* 0x14 - 0x15 */
	u16 cwcr;		/* 0x16 Core Watchdog Control Register */
	u8 res3[3];		/* 0x18 - 0x1A */
	u8 cwsr;		/* 0x1B Core Watchdog Service Register */
	u8 res4[2];		/* 0x1C - 0x1D */
	u8 scmisr;		/* 0x1F Interrupt Status Register */
	u32 res5;		/* 0x20 */
	u32 bcr;		/* 0x24 Burst Configuration Register */
	u32 res6[18];		/* 0x28 - 0x6F */
	u32 cfadr;		/* 0x70 Core Fault Address Register */
	u8 res7[4];		/* 0x71 - 0x74 */
	u8 cfier;		/* 0x75 Core Fault Interrupt Enable Register */
	u8 cfloc;		/* 0x76 Core Fault Location Register */
	u8 cfatr;		/* 0x77 Core Fault Attributes Register */
	u32 res8;		/* 0x78 */
	u32 cfdtr;		/* 0x7C Core Fault Data Register */
} scm3_t;

typedef struct canex_ctrl {
	can_msg_t msg[16];	/* 0x00 Message Buffer 0-15 */
} canex_t;

/* Watchdog registers */
typedef struct wdog_ctrl {
	u16 cr;			/* 0x00 Control register */
	u16 mr;			/* 0x02 Modulus register */
	u16 cntr;		/* 0x04 Count register */
	u16 sr;			/* 0x06 Service register */
} wdog_t;

/*Chip configuration module registers */
typedef struct ccm_ctrl {
	u16 ccr;		/* 0x00 Chip configuration register */
	u16 res2;		/* 0x02 */
	u16 rcon;		/* 0x04 Rreset configuration register */
	u16 cir;		/* 0x06 Chip identification register */
	u32 res3;		/* 0x08 */
	u16 misccr;		/* 0x0A Miscellaneous control register */
	u16 cdr;		/* 0x0C Clock divider register */
	u16 uhcsr;		/* 0x10 USB Host controller status register */
	u16 uocsr;		/* 0x12 USB On-the-Go Controller Status Reg */
} ccm_t;

typedef struct rcm {
	u8 rcr;
	u8 rsr;
} rcm_t;

/* GPIO port registers */
typedef struct gpio_ctrl {
	/* Port Output Data Registers */
#ifdef CONFIG_M5329
	u8 podr_fech;		/* 0x00 */
	u8 podr_fecl;		/* 0x01 */
#else
	u16 res00;		/* 0x00 - 0x01 */
#endif
	u8 podr_ssi;		/* 0x02 */
	u8 podr_busctl;		/* 0x03 */
	u8 podr_be;		/* 0x04 */
	u8 podr_cs;		/* 0x05 */
	u8 podr_pwm;		/* 0x06 */
	u8 podr_feci2c;		/* 0x07 */
	u8 res08;		/* 0x08 */
	u8 podr_uart;		/* 0x09 */
	u8 podr_qspi;		/* 0x0A */
	u8 podr_timer;		/* 0x0B */
#ifdef CONFIG_M5329
	u8 res0C;		/* 0x0C */
	u8 podr_lcddatah;	/* 0x0D */
	u8 podr_lcddatam;	/* 0x0E */
	u8 podr_lcddatal;	/* 0x0F */
	u8 podr_lcdctlh;	/* 0x10 */
	u8 podr_lcdctll;	/* 0x11 */
#else
	u16 res0C;		/* 0x0C - 0x0D */
	u8 podr_fech;		/* 0x0E */
	u8 podr_fecl;		/* 0x0F */
	u16 res10[3];		/* 0x10 - 0x15 */
#endif

	/* Port Data Direction Registers */
#ifdef CONFIG_M5329
	u16 res12;		/* 0x12 - 0x13 */
	u8 pddr_fech;		/* 0x14 */
	u8 pddr_fecl;		/* 0x15 */
#endif
	u8 pddr_ssi;		/* 0x16 */
	u8 pddr_busctl;		/* 0x17 */
	u8 pddr_be;		/* 0x18 */
	u8 pddr_cs;		/* 0x19 */
	u8 pddr_pwm;		/* 0x1A */
	u8 pddr_feci2c;		/* 0x1B */
	u8 res1C;		/* 0x1C */
	u8 pddr_uart;		/* 0x1D */
	u8 pddr_qspi;		/* 0x1E */
	u8 pddr_timer;		/* 0x1F */
#ifdef CONFIG_M5329
	u8 res20;		/* 0x20 */
	u8 pddr_lcddatah;	/* 0x21 */
	u8 pddr_lcddatam;	/* 0x22 */
	u8 pddr_lcddatal;	/* 0x23 */
	u8 pddr_lcdctlh;	/* 0x24 */
	u8 pddr_lcdctll;	/* 0x25 */
	u16 res26;		/* 0x26 - 0x27 */
#else
	u16 res20;		/* 0x20 - 0x21 */
	u8 pddr_fech;		/* 0x22 */
	u8 pddr_fecl;		/* 0x23 */
	u16 res24[3];		/* 0x24 - 0x29 */
#endif

	/* Port Data Direction Registers */
#ifdef CONFIG_M5329
	u8 ppd_fech;		/* 0x28 */
	u8 ppd_fecl;		/* 0x29 */
#endif
	u8 ppd_ssi;		/* 0x2A */
	u8 ppd_busctl;		/* 0x2B */
	u8 ppd_be;		/* 0x2C */
	u8 ppd_cs;		/* 0x2D */
	u8 ppd_pwm;		/* 0x2E */
	u8 ppd_feci2c;		/* 0x2F */
	u8 res30;		/* 0x30 */
	u8 ppd_uart;		/* 0x31 */
	u8 ppd_qspi;		/* 0x32 */
	u8 ppd_timer;		/* 0x33 */
#ifdef CONFIG_M5329
	u8 res34;		/* 0x34 */
	u8 ppd_lcddatah;	/* 0x35 */
	u8 ppd_lcddatam;	/* 0x36 */
	u8 ppd_lcddatal;	/* 0x37 */
	u8 ppd_lcdctlh;		/* 0x38 */
	u8 ppd_lcdctll;		/* 0x39 */
	u16 res3A;		/* 0x3A - 0x3B */
#else
	u16 res34;		/* 0x34 - 0x35 */
	u8 ppd_fech;		/* 0x36 */
	u8 ppd_fecl;		/* 0x37 */
	u16 res38[3];		/* 0x38 - 0x3D */
#endif

	/* Port Clear Output Data Registers */
#ifdef CONFIG_M5329
	u8 res3C;		/* 0x3C */
	u8 pclrr_fech;		/* 0x3D */
	u8 pclrr_fecl;		/* 0x3E */
#else
	u8 pclrr_ssi;		/* 0x3E */
#endif
	u8 pclrr_busctl;	/* 0x3F */
	u8 pclrr_be;		/* 0x40 */
	u8 pclrr_cs;		/* 0x41 */
	u8 pclrr_pwm;		/* 0x42 */
	u8 pclrr_feci2c;	/* 0x43 */
	u8 res44;		/* 0x44 */
	u8 pclrr_uart;		/* 0x45 */
	u8 pclrr_qspi;		/* 0x46 */
	u8 pclrr_timer;		/* 0x47 */
#ifdef CONFIG_M5329
	u8 pclrr_lcddatah;	/* 0x48 */
	u8 pclrr_lcddatam;	/* 0x49 */
	u8 pclrr_lcddatal;	/* 0x4A */
	u8 pclrr_ssi;		/* 0x4B */
	u8 pclrr_lcdctlh;	/* 0x4C */
	u8 pclrr_lcdctll;	/* 0x4D */
	u16 res4E;		/* 0x4E - 0x4F */
#else
	u16 res48;		/* 0x48 - 0x49 */
	u8 pclrr_fech;		/* 0x4A */
	u8 pclrr_fecl;		/* 0x4B */
	u8 res4C[5];		/* 0x4C - 0x50 */
#endif

	/* Pin Assignment Registers */
#ifdef CONFIG_M5329
	u8 par_fec;		/* 0x50 */
#endif
	u8 par_pwm;		/* 0x51 */
	u8 par_busctl;		/* 0x52 */
	u8 par_feci2c;		/* 0x53 */
	u8 par_be;		/* 0x54 */
	u8 par_cs;		/* 0x55 */
	u16 par_ssi;		/* 0x56 */
	u16 par_uart;		/* 0x58 */
	u16 par_qspi;		/* 0x5A */
	u8 par_timer;		/* 0x5C */
#ifdef CONFIG_M5329
	u8 par_lcddata;		/* 0x5D */
	u16 par_lcdctl;		/* 0x5E */
#else
	u8 par_fec;		/* 0x5D */
	u16 res5E;		/* 0x5E - 0x5F */
#endif
	u16 par_irq;		/* 0x60 */
	u16 res62;		/* 0x62 - 0x63 */

	/* Mode Select Control Registers */
	u8 mscr_flexbus;	/* 0x64 */
	u8 mscr_sdram;		/* 0x65 */
	u16 res66;		/* 0x66 - 0x67 */

	/* Drive Strength Control Registers */
	u8 dscr_i2c;		/* 0x68 */
	u8 dscr_pwm;		/* 0x69 */
	u8 dscr_fec;		/* 0x6A */
	u8 dscr_uart;		/* 0x6B */
	u8 dscr_qspi;		/* 0x6C */
	u8 dscr_timer;		/* 0x6D */
	u8 dscr_ssi;		/* 0x6E */
#ifdef CONFIG_M5329
	u8 dscr_lcd;		/* 0x6F */
#else
	u8 res6F;		/* 0x6F */
#endif
	u8 dscr_debug;		/* 0x70 */
	u8 dscr_clkrst;		/* 0x71 */
	u8 dscr_irq;		/* 0x72 */
} gpio_t;

/* USB OTG module registers */
typedef struct usb_otg {
	u32 id;			/* 0x000 Identification Register */
	u32 hwgeneral;		/* 0x004 General HW Parameters */
	u32 hwhost;		/* 0x008 Host HW Parameters */
	u32 hwdev;		/* 0x00C Device HW parameters */
	u32 hwtxbuf;		/* 0x010 TX Buffer HW Parameters */
	u32 hwrxbuf;		/* 0x014 RX Buffer HW Parameters */
	u32 res1[58];		/* 0x18 - 0xFF */
	u8 caplength;		/* 0x100 Capability Register Length */
	u8 res2;		/* 0x101 */
	u16 hciver;		/* 0x102 Host Interface Version Number */
	u32 hcsparams;		/* 0x104 Host Structural Parameters */
	u32 hccparams;		/* 0x108 Host Capability Parameters */
	u32 res3[5];		/* 0x10C - 0x11F */
	u16 dciver;		/* 0x120 Device Interface Version Number */
	u16 res4;		/* 0x122 */
	u32 dccparams;		/* 0x124 Device Capability Parameters */
	u32 res5[6];		/* 0x128 - 0x13F */
	u32 cmd;		/* 0x140 USB Command */
	u32 sts;		/* 0x144 USB Status */
	u32 intr;		/* 0x148 USB Interrupt Enable */
	u32 frindex;		/* 0x14C USB Frame Index */
	u32 res6;		/* 0x150 */
	u32 prd_dev;		/* 0x154 Periodic Frame List Base or Device Address */
	u32 aync_ep;		/* 0x158 Current Asynchronous List or Address at Endpoint List Address */
	u32 ttctrl;		/* 0x15C Host TT Asynchronous Buffer Control */
	u32 burstsize;		/* 0x160 Master Interface Data Burst Size */
	u32 txfill;		/* 0x164 Host Transmit FIFO Tuning Control */
	u32 res7[6];		/* 0x168 - 0x17F */
	u32 cfgflag;		/* 0x180 Configure Flag Register */
	u32 portsc1;		/* 0x184 Port Status/Control */
	u32 res8[7];		/* 0x188 - 0x1A3 */
	u32 otgsc;		/* 0x1A4 On The Go Status and Control */
	u32 mode;		/* 0x1A8 USB mode register */
	u32 eptsetstat;		/* 0x1AC Endpoint Setup status */
	u32 eptprime;		/* 0x1B0 Endpoint initialization */
	u32 eptflush;		/* 0x1B4 Endpoint de-initialize */
	u32 eptstat;		/* 0x1B8 Endpoint status */
	u32 eptcomplete;	/* 0x1BC Endpoint Complete */
	u32 eptctrl0;		/* 0x1C0 Endpoint control 0 */
	u32 eptctrl1;		/* 0x1C4 Endpoint control 1 */
	u32 eptctrl2;		/* 0x1C8 Endpoint control 2 */
	u32 eptctrl3;		/* 0x1CC Endpoint control 3 */
} usbotg_t;

/* SDRAM controller registers */
typedef struct sdram_ctrl {
	u32 mode;		/* 0x00 Mode/Extended Mode register */
	u32 ctrl;		/* 0x04 Control register */
	u32 cfg1;		/* 0x08 Configuration register 1 */
	u32 cfg2;		/* 0x0C Configuration register 2 */
	u32 res1[64];		/* 0x10 - 0x10F */
	u32 cs0;		/* 0x110 Chip Select 0 Configuration */
	u32 cs1;		/* 0x114 Chip Select 1 Configuration */
} sdram_t;

/* Clock Module registers */
typedef struct pll_ctrl {
	u8 podr;		/* 0x00 Output Divider Register */
	u8 res1[3];
	u8 pcr;			/* 0x04 Control Register */
	u8 res2[3];
	u8 pmdr;		/* 0x08 Modulation Divider Register */
	u8 res3[3];
	u8 pfdr;		/* 0x0C Feedback Divider Register */
	u8 res4[3];
} pll_t;

#endif				/* __IMMAP_5329__ */
