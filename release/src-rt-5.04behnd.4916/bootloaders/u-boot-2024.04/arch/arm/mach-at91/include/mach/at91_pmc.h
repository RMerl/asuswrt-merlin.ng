/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * [origin: Linux kernel arch/arm/mach-at91/include/mach/at91_pmc.h]
 *
 * Copyright (C) 2005 Ivan Kokshaysky
 * Copyright (C) SAN People
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 *
 * Power Management Controller (PMC) - System peripherals registers.
 * Based on AT91RM9200 datasheet revision E.
 */

#ifndef AT91_PMC_H
#define AT91_PMC_H

#ifdef __ASSEMBLY__

#define	AT91_ASM_PMC_MOR	(ATMEL_BASE_PMC + 0x20)
#define	AT91_ASM_PMC_PLLAR	(ATMEL_BASE_PMC + 0x28)
#define	AT91_ASM_PMC_PLLBR	(ATMEL_BASE_PMC + 0x2c)
#define AT91_ASM_PMC_MCKR	(ATMEL_BASE_PMC + 0x30)
#define AT91_ASM_PMC_SR		(ATMEL_BASE_PMC + 0x68)

#else

#include <asm/types.h>
#include <linux/bitops.h>

typedef struct at91_pmc {
	u32	scer;		/* 0x00 System Clock Enable Register */
	u32	scdr;		/* 0x04 System Clock Disable Register */
	u32	scsr;		/* 0x08 System Clock Status Register */
	u32	reserved0;
	u32	pcer;		/* 0x10 Peripheral Clock Enable Register */
	u32	pcdr;		/* 0x14 Peripheral Clock Disable Register */
	u32	pcsr;		/* 0x18 Peripheral Clock Status Register */
	u32	uckr;		/* 0x1C UTMI Clock Register */
	u32	mor;		/* 0x20 Main Oscilator Register */
	u32	mcfr;		/* 0x24 Main Clock Frequency Register */
	u32	pllar;		/* 0x28 PLL A Register */
	u32	pllbr;		/* 0x2C PLL B Register */
	u32	mckr;		/* 0x30 Master Clock Register */
	u32	reserved1;
	u32	usb;		/* 0x38 USB Clock Register */
	u32	reserved2;
	u32	pck[4];		/* 0x40 Programmable Clock Register 0 - 3 */
	u32	reserved3[4];
	u32	ier;		/* 0x60 Interrupt Enable Register */
	u32	idr;		/* 0x64 Interrupt Disable Register */
	u32	sr;		/* 0x68 Status Register */
	u32	imr;		/* 0x6C Interrupt Mask Register */
	u32	reserved4[4];
	u32	pllicpr;	/* 0x80 Change Pump Current Register (SAM9) */
	u32	reserved5[24];
	u32	wpmr;		/* 0xE4 Write Protect Mode Register (CAP0) */
	u32	wpsr;		/* 0xE8 Write Protect Status Register (CAP0) */
	u32	reserved6[5];
	u32	pcer1;		/* 0x100 Periperial Clock Enable Register 1 */
	u32	pcdr1;		/* 0x104 Periperial Clock Disable Register 1 */
	u32	pcsr1;		/* 0x108 Periperial Clock Status Register 1 */
	u32	pcr;		/* 0x10c Periperial Control Register */
	u32	ocr;		/* 0x110 Oscillator Calibration Register */
} at91_pmc_t;

#endif	/* end not assembly */

#define AT91_PMC_MOR_MOSCEN		0x01
#define AT91_PMC_MOR_OSCBYPASS		0x02
#define AT91_PMC_MOR_MOSCRCEN		0x08
#define AT91_PMC_MOR_OSCOUNT(x)		(((x) & 0xff) << 8)
#define AT91_PMC_MOR_KEY(x)		(((x) & 0xff) << 16)
#define AT91_PMC_MOR_MOSCSEL		(1 << 24)

#define AT91_PMC_PLLXR_DIV(x)		((x) & 0xFF)
#define AT91_PMC_PLLXR_PLLCOUNT(x)	(((x) & 0x3F) << 8)
#define AT91_PMC_PLLXR_OUT(x)		(((x) & 0x03) << 14)
#if defined(CONFIG_SAMA5D2) || defined(CONFIG_SAMA5D3) || \
	defined(CONFIG_SAMA5D4)
#define AT91_PMC_PLLXR_MUL(x)		(((x) & 0x7F) << 18)
#else
#define AT91_PMC_PLLXR_MUL(x)		(((x) & 0x7FF) << 16)
#endif
#define AT91_PMC_PLLAR_29		0x20000000
#define AT91_PMC_PLLBR_USBDIV_1		0x00000000
#define AT91_PMC_PLLBR_USBDIV_2		0x10000000
#define AT91_PMC_PLLBR_USBDIV_4		0x20000000

#define AT91_PMC_MCFR_MAINRDY		0x00010000
#define AT91_PMC_MCFR_MAINF_MASK	0x0000FFFF
#define AT91_PMC_MCFR_RCMEAS		0x00100000
#define AT91_PMC_MCFR_CCSS_XTAL_OSC	0x01000000

#define AT91_PMC_MCKR_CSS_SLOW		0x00000000
#define AT91_PMC_MCKR_CSS_MAIN		0x00000001
#define AT91_PMC_MCKR_CSS_PLLA		0x00000002
#define AT91_PMC_MCKR_CSS_PLLB		0x00000003
#define AT91_PMC_MCKR_CSS_MASK		0x00000003

#if defined(CONFIG_SAMA5D2) || defined(CONFIG_SAMA5D3) || \
	defined(CONFIG_SAMA5D4) || \
	defined(CONFIG_AT91SAM9X5) || defined(CONFIG_AT91SAM9N12)
#define AT91_PMC_MCKR_PRES_1		0x00000000
#define AT91_PMC_MCKR_PRES_2		0x00000010
#define AT91_PMC_MCKR_PRES_4		0x00000020
#define AT91_PMC_MCKR_PRES_8		0x00000030
#define AT91_PMC_MCKR_PRES_16		0x00000040
#define AT91_PMC_MCKR_PRES_32		0x00000050
#define AT91_PMC_MCKR_PRES_64		0x00000060
#define AT91_PMC_MCKR_PRES_MASK		0x00000070
#else
#define AT91_PMC_MCKR_PRES_1		0x00000000
#define AT91_PMC_MCKR_PRES_2		0x00000004
#define AT91_PMC_MCKR_PRES_4		0x00000008
#define AT91_PMC_MCKR_PRES_8		0x0000000C
#define AT91_PMC_MCKR_PRES_16		0x00000010
#define AT91_PMC_MCKR_PRES_32		0x00000014
#define AT91_PMC_MCKR_PRES_64		0x00000018
#define AT91_PMC_MCKR_PRES_MASK		0x0000001C
#endif

#ifdef CONFIG_AT91RM9200
#define AT91_PMC_MCKR_MDIV_1		0x00000000
#define AT91_PMC_MCKR_MDIV_2		0x00000100
#define AT91_PMC_MCKR_MDIV_3		0x00000200
#define AT91_PMC_MCKR_MDIV_4		0x00000300
#define AT91_PMC_MCKR_MDIV_MASK		0x00000300
#else
#define AT91_PMC_MCKR_MDIV_1		0x00000000
#define AT91_PMC_MCKR_MDIV_2		0x00000100
#define AT91_PMC_MCKR_MDIV_3		0x00000300
#define AT91_PMC_MCKR_MDIV_4		0x00000200
#define AT91_PMC_MCKR_MDIV_MASK		0x00000300
#endif

#define AT91_PMC_MCKR_PLLADIV_MASK	0x00003000
#define AT91_PMC_MCKR_PLLADIV_1		0x00000000
#define AT91_PMC_MCKR_PLLADIV_2		0x00001000

#define AT91_PMC_MCKR_H32MXDIV		0x01000000

#define AT91_PMC_IXR_MOSCS		0x00000001
#define AT91_PMC_IXR_LOCKA		0x00000002
#define AT91_PMC_IXR_LOCKB		0x00000004
#define AT91_PMC_IXR_MCKRDY		0x00000008
#define AT91_PMC_IXR_LOCKU		0x00000040
#define AT91_PMC_IXR_PCKRDY0		0x00000100
#define AT91_PMC_IXR_PCKRDY1		0x00000200
#define AT91_PMC_IXR_PCKRDY2		0x00000400
#define AT91_PMC_IXR_PCKRDY3		0x00000800
#define AT91_PMC_IXR_MOSCSELS		0x00010000

#define AT91_PMC_PCR_PID_MASK		(0x3f)
#define AT91_PMC_PCR_GCKCSS		(0x7 << 8)
#define AT91_PMC_PCR_GCKCSS_MASK	0x07
#define AT91_PMC_PCR_GCKCSS_OFFSET	8
#define AT91_PMC_PCR_GCKCSS_(x)		((x & 0x07) << 8)
#define		AT91_PMC_PCR_GCKCSS_SLOW_CLK	(0x0 << 8)
#define		AT91_PMC_PCR_GCKCSS_MAIN_CLK	(0x1 << 8)
#define		AT91_PMC_PCR_GCKCSS_PLLA_CLK	(0x2 << 8)
#define		AT91_PMC_PCR_GCKCSS_UPLL_CLK	(0x3 << 8)
#define		AT91_PMC_PCR_GCKCSS_MCK_CLK	(0x4 << 8)
#define		AT91_PMC_PCR_GCKCSS_AUDIO_CLK	(0x5 << 8)
#define AT91_PMC_PCR_CMD_WRITE		(0x1 << 12)
#define AT91_PMC_PCR_DIV		(0x3 << 16)
#define AT91_PMC_PCR_GCKDIV		(0xff << 20)
#define AT91_PMC_PCR_GCKDIV_MASK	0xff
#define AT91_PMC_PCR_GCKDIV_OFFSET	20
#define AT91_PMC_PCR_GCKDIV_(x)		((x & 0xff) << 20)
#define AT91_PMC_PCR_EN			(0x1 << 28)
#define AT91_PMC_PCR_GCKEN		(0x1 << 29)

#define		AT91_PMC_PCK		(1 <<  0)		/* Processor Clock */
#define		AT91RM9200_PMC_UDP	(1 <<  1)		/* USB Devcice Port Clock [AT91RM9200 only] */
#define		AT91_PMC_DDR		(1 <<  2)		/* DDR Clock */
#define		AT91RM9200_PMC_MCKUDP	(1 <<  2)		/* USB Device Port Master Clock Automatic Disable on Suspend [AT91RM9200 only] */
#define		AT91RM9200_PMC_UHP	(1 <<  4)		/* USB Host Port Clock [AT91RM9200 only] */
#define		AT91SAM926x_PMC_UHP	(1 <<  6)		/* USB Host Port Clock [AT91SAM926x only] */
#define		AT91SAM926x_PMC_UDP	(1 <<  7)		/* USB Devcice Port Clock [AT91SAM926x only] */
#define		AT91_PMC_PCK0		(1 <<  8)		/* Programmable Clock 0 */
#define		AT91_PMC_PCK1		(1 <<  9)		/* Programmable Clock 1 */
#define		AT91_PMC_PCK2		(1 << 10)		/* Programmable Clock 2 */
#define		AT91_PMC_PCK3		(1 << 11)		/* Programmable Clock 3 */
#define		AT91_PMC_HCK0		(1 << 16)		/* AHB Clock (USB host) [AT91SAM9261 only] */
#define		AT91_PMC_HCK1		(1 << 17)		/* AHB Clock (LCD) [AT91SAM9261 only] */

#define		AT91_PMC_UPLLEN		(1   << 16)		/* UTMI PLL Enable */
#define		AT91_PMC_UPLLCOUNT	(0xf << 20)		/* UTMI PLL Start-up Time */
#define		AT91_PMC_BIASEN		(1   << 24)		/* UTMI BIAS Enable */
#define		AT91_PMC_BIASCOUNT	(0xf << 28)		/* UTMI PLL Start-up Time */

#define		AT91_PMC_MOSCEN		(1    << 0)		/* Main Oscillator Enable */
#define		AT91_PMC_OSCBYPASS	(1    << 1)		/* Oscillator Bypass [SAM9x] */
#define		AT91_PMC_OSCOUNT	(0xff << 8)		/* Main Oscillator Start-up Time */

#define		AT91_PMC_MAINF		(0xffff <<  0)		/* Main Clock Frequency */
#define		AT91_PMC_MAINRDY	(1	<< 16)		/* Main Clock Ready */

#define		AT91_PMC_DIV		(0xff  <<  0)		/* Divider */
#define		AT91_PMC_PLLCOUNT	(0x3f  <<  8)		/* PLL Counter */
#define		AT91_PMC_OUT		(3     << 14)		/* PLL Clock Frequency Range */
#define		AT91_PMC_MUL		(0x7ff << 16)		/* PLL Multiplier */
#define		AT91_PMC_USBDIV		(3     << 28)		/* USB Divisor (PLLB only) */
#define			AT91_PMC_USBDIV_1		(0 << 28)
#define			AT91_PMC_USBDIV_2		(1 << 28)
#define			AT91_PMC_USBDIV_4		(2 << 28)
#define		AT91_PMC_USB96M		(1     << 28)		/* Divider by 2 Enable (PLLB only) */
#define		AT91_PMC_PLLA_WR_ERRATA	(1     << 29)		/* Bit 29 must always be set to 1 when programming the CKGR_PLLAR register */

#define		AT91_PMC_CSS		(3 <<  0)		/* Master Clock Selection */
#define			AT91_PMC_CSS_SLOW		(0 << 0)
#define			AT91_PMC_CSS_MAIN		(1 << 0)
#define			AT91_PMC_CSS_PLLA		(2 << 0)
#define			AT91_PMC_CSS_PLLB		(3 << 0)
#define		AT91_PMC_PRES		(7 <<  2)		/* Master Clock Prescaler */
#define			AT91_PMC_PRES_1			(0 << 2)
#define			AT91_PMC_PRES_2			(1 << 2)
#define			AT91_PMC_PRES_4			(2 << 2)
#define			AT91_PMC_PRES_8			(3 << 2)
#define			AT91_PMC_PRES_16		(4 << 2)
#define			AT91_PMC_PRES_32		(5 << 2)
#define			AT91_PMC_PRES_64		(6 << 2)
#define		AT91_PMC_MDIV		(3 <<  8)		/* Master Clock Division */
#define			AT91RM9200_PMC_MDIV_1		(0 << 8)	/* [AT91RM9200 only] */
#define			AT91RM9200_PMC_MDIV_2		(1 << 8)
#define			AT91RM9200_PMC_MDIV_3		(2 << 8)
#define			AT91RM9200_PMC_MDIV_4		(3 << 8)
#define			AT91SAM9_PMC_MDIV_1		(0 << 8)	/* [SAM9 only] */
#define			AT91SAM9_PMC_MDIV_2		(1 << 8)
#define			AT91SAM9_PMC_MDIV_4		(2 << 8)
#define			AT91SAM9_PMC_MDIV_3		(3 << 8)	/* [some SAM9 only] */
#define			AT91SAM9_PMC_MDIV_6		(3 << 8)
#define		AT91_PMC_PDIV		(1 << 12)		/* Processor Clock Division [some SAM9 only] */
#define			AT91_PMC_PDIV_1			(0 << 12)
#define			AT91_PMC_PDIV_2			(1 << 12)

#define AT91_PMC_USB_USBS_MASK		0x1
#define AT91_PMC_USB_USBS_OFFSET		0
#define AT91_PMC_USB_USBS_(x)		(x & 0x1)
#define		AT91_PMC_USBS_USB_PLLA		(0x0)		/* USB Clock Input is PLLA */
#define		AT91_PMC_USBS_USB_UPLL		(0x1)		/* USB Clock Input is UPLL */
#define		AT91_PMC_USBS_USB_PLLB		(0x1)		/* USB Clock Input is PLLB, AT91SAM9N12 only */
#define AT91_PMC_USB_DIV_MASK		0xf
#define AT91_PMC_USB_DIV_OFFSET		8
#define AT91_PMC_USB_DIV_(x)		((x & 0xf) << 8)
#define		AT91_PMC_USB_DIV_2		(0x1 <<  8)	/* USB Clock divided by 2 */
#define		AT91_PMC_USBDIV_8		(0x7 <<  8)	/* USB Clock divided by 8 */
#define		AT91_PMC_USBDIV_10		(0x9 <<  8)	/* USB Clock divided by 10 */

#define		AT91_PMC_MOSCS		(1 <<  0)		/* MOSCS Flag */
#define		AT91_PMC_LOCKA		(1 <<  1)		/* PLLA Lock */
#define		AT91_PMC_LOCKB		(1 <<  2)		/* PLLB Lock */
#define		AT91_PMC_MCKRDY		(1 <<  3)		/* Master Clock */
#define		AT91_PMC_LOCKU		(1 <<  6)		/* UPLL Lock */
#define		AT91_PMC_PCK0RDY	(1 <<  8)		/* Programmable Clock 0 */
#define		AT91_PMC_PCK1RDY	(1 <<  9)		/* Programmable Clock 1 */
#define		AT91_PMC_PCK2RDY	(1 << 10)		/* Programmable Clock 2 */
#define		AT91_PMC_PCK3RDY	(1 << 11)		/* Programmable Clock 3 */
#define		AT91_PMC_MOSCSELS	BIT(16)			/* Main Oscillator Selection Status */
#define		AT91_PMC_MOSCRCS	BIT(17)			/* 12 MHz RC Oscillator Status */
#define		AT91_PMC_GCKRDY		(1 << 24)
#define		AT91_PMC_PROTKEY	0x504d4301	/* Activation Code */

/* PLL Charge Pump Current Register (PMC_PLLICPR) */
#define AT91_PMC_ICP_PLLA(x)		(((x) & 0x3) << 0)
#define AT91_PMC_IPLL_PLLA(x)		(((x) & 0x7) << 8)
#define AT91_PMC_ICP_PLLU(x)		(((x) & 0x3) << 16)
#define AT91_PMC_IVCO_PLLU(x)		(((x) & 0x3) << 24)

#endif
