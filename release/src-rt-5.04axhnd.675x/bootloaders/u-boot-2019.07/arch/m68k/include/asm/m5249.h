/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * mcf5249.h -- Definitions for Motorola Coldfire 5249
 *
 * Based on mcf5272sim.h of uCLinux distribution:
 *      (C) Copyright 1999, Greg Ungerer (gerg@snapgear.com)
 *      (C) Copyright 2000, Lineo Inc. (www.lineo.com)
 */

#ifndef	mcf5249_h
#define	mcf5249_h
/****************************************************************************/

/*
 * useful definitions for reading/writing MBAR offset memory
 */
#define mbar_readLong(x)	*((volatile unsigned long *) (CONFIG_SYS_MBAR + x))
#define mbar_writeLong(x,y)	*((volatile unsigned long *) (CONFIG_SYS_MBAR + x)) = y
#define mbar_writeShort(x,y)	*((volatile unsigned short *) (CONFIG_SYS_MBAR + x)) = y
#define mbar_writeByte(x,y)	*((volatile unsigned char *) (CONFIG_SYS_MBAR + x)) = y
#define mbar2_readLong(x)	*((volatile unsigned long *) (CONFIG_SYS_MBAR2 + x))
#define mbar2_writeLong(x,y)	*((volatile unsigned long *) (CONFIG_SYS_MBAR2 + x)) = y
#define mbar2_writeShort(x,y)	*((volatile unsigned short *) (CONFIG_SYS_MBAR2 + x)) = y
#define mbar2_writeByte(x,y)	*((volatile unsigned char *) (CONFIG_SYS_MBAR2 + x)) = y

/*
 * Size of internal RAM
 */

#define INT_RAM_SIZE 32768	/* RAMBAR0 - 32k */
#define INT_RAM_SIZE2 65536	/* RAMBAR1 - 64k */

/*
 *	Define the 5249 SIM register set addresses.
 */

/*****************
 ***** MBAR1 *****
 *****************/
#define	MCFSIM_RSR		0x00	/* Reset Status reg (r/w) */
#define	MCFSIM_SYPCR		0x01	/* System Protection reg (r/w) */
#define	MCFSIM_SWIVR		0x02	/* SW Watchdog intr reg (r/w) */
#define	MCFSIM_SWSR		0x03	/* SW Watchdog service (r/w) */
#define MCFSIM_MPARK		0x0c	/* Bus master park register (r/w) */

#define	MCFSIM_SIMR		0x00	/* SIM Config reg (r/w) */
#define	MCFSIM_ICR0		0x4c	/* Intr Ctrl reg 0 (r/w) */
#define	MCFSIM_ICR1		0x4d	/* Intr Ctrl reg 1 (r/w) */
#define	MCFSIM_ICR2		0x4e	/* Intr Ctrl reg 2 (r/w) */
#define	MCFSIM_ICR3		0x4f	/* Intr Ctrl reg 3 (r/w) */
#define	MCFSIM_ICR4		0x50	/* Intr Ctrl reg 4 (r/w) */
#define	MCFSIM_ICR5		0x51	/* Intr Ctrl reg 5 (r/w) */
#define	MCFSIM_ICR6		0x52	/* Intr Ctrl reg 6 (r/w) */
#define	MCFSIM_ICR7		0x53	/* Intr Ctrl reg 7 (r/w) */
#define	MCFSIM_ICR8		0x54	/* Intr Ctrl reg 8 (r/w) */
#define	MCFSIM_ICR9		0x55	/* Intr Ctrl reg 9 (r/w) */
#define	MCFSIM_ICR10		0x56	/* Intr Ctrl reg 10 (r/w) */
#define	MCFSIM_ICR11		0x57	/* Intr Ctrl reg 11 (r/w) */

#define MCFSIM_IPR		0x40	/* Interrupt Pend reg (r/w) */
#define MCFSIM_IMR		0x44	/* Interrupt Mask reg (r/w) */

#define MCFSIM_DCR		0x100	/* DRAM Control reg (r/w) */
#define MCFSIM_DACR0		0x108	/* DRAM 0 Addr and Ctrl (r/w) */
#define MCFSIM_DMR0		0x10c	/* DRAM 0 Mask reg (r/w) */
#define MCFSIM_DACR1		0x110	/* DRAM 1 Addr and Ctrl (r/w) */
#define MCFSIM_DMR1		0x114	/* DRAM 1 Mask reg (r/w) */

/*****************
 ***** MBAR2 *****
 *****************/

/*  GPIO Addresses
 *  Note: These are offset from MBAR2!
 */
#define MCFSIM_GPIO_READ	0x00	/* Read-Only access to gpio 0-31 (MBAR2) (r) */
#define MCFSIM_GPIO_OUT		0x04	/* Output register for gpio 0-31 (MBAR2) (r/w) */
#define MCFSIM_GPIO_EN		0x08	/* gpio 0-31 enable (r/w) */
#define MCFSIM_GPIO_FUNC	0x0c	/* gpio 0-31 function select (r/w) */
#define MCFSIM_GPIO1_READ	0xb0	/* Read-Only access to gpio 32-63 (MBAR2) (r) */
#define MCFSIM_GPIO1_OUT	0xb4	/* Output register for gpio 32-63 (MBAR2) (r/w) */
#define MCFSIM_GPIO1_EN		0xb8	/* gpio 32-63 enable (r/w) */
#define MCFSIM_GPIO1_FUNC	0xbc	/* gpio 32-63 function select (r/w) */

#define MCFSIM_GPIO_INT_STAT	0xc0	/* Secondary Interrupt status (r) */
#define MCFSIM_GPIO_INT_CLEAR	0xc0	/* Secondary Interrupt status (w) */
#define MCFSIM_GPIO_INT_EN	0xc4	/* Secondary Interrupt status (r/w) */

#define MCFSIM_INT_STAT3	0xe0	/* 3rd Interrupt ctrl status (r) */
#define MCFSIM_INT_CLEAR3	0xe0	/* 3rd Interrupt ctrl clear (w) */
#define MCFSIM_INT_EN3		0xe4	/* 3rd Interrupt ctrl enable (r/w) */

#define MCFSIM_INTLEV1		0x140	/* Interrupts 0 - 7 (r/w) */
#define MCFSIM_INTLEV2		0x144	/* Interrupts 8 -15 (r/w) */
#define MCFSIM_INTLEV3		0x148	/* Interrupts 16-23 (r/w) */
#define MCFSIM_INTLEV4		0x14c	/* Interrupts 24-31 (r/w) */
#define MCFSIM_INTLEV5		0x150	/* Interrupts 32-39 (r/w) */
#define MCFSIM_INTLEV6		0x154	/* Interrupts 40-47 (r/w) */
#define MCFSIM_INTLEV7		0x158	/* Interrupts 48-55 (r/w) */
#define MCFSIM_INTLEV8		0x15c	/* Interrupts 56-63 (r/w) */

#define MCFSIM_SPURVEC		0x167	/* Spurious Vector Register (r/w) */
#define MCFSIM_INTBASE		0x16b	/* Software interrupt base address (r/w) */

#define MCFSIM_IDECONFIG1	0x18c	/* IDE config register 1 (r/w) */
#define MCFSIM_IDECONFIG2	0x190	/* IDE config register 1 (r/w) */

#define MCFSIM_PLLCR		0x180	/* PLL Control register */

/*
 *  Some symbol defines for the above...
 */
#define	MCFSIM_SWDICR		MCFSIM_ICR0	/* Watchdog timer ICR */
#define	MCFSIM_TIMER1ICR	MCFSIM_ICR1	/* Timer 1 ICR */
#define	MCFSIM_TIMER2ICR	MCFSIM_ICR2	/* Timer 2 ICR */
#define	MCFSIM_I2CICR		MCFSIM_ICR3	/* I2C ICR */
#define	MCFSIM_UART1ICR		MCFSIM_ICR4	/* UART 1 ICR */
#define	MCFSIM_UART2ICR		MCFSIM_ICR5	/* UART 2 ICR */
/* XXX - If needed, DMA ICRs go here */
#define	MCFSIM_QSPIICR		MCFSIM_ICR10	/* QSPI ICR */

/*
 *	Bit definitions for the ICR family of registers.
 */
#define	MCFSIM_ICR_AUTOVEC	0x80	/* Auto-vectored intr */
#define	MCFSIM_ICR_LEVEL0	0x00	/* Level 0 intr */
#define	MCFSIM_ICR_LEVEL1	0x04	/* Level 1 intr */
#define	MCFSIM_ICR_LEVEL2	0x08	/* Level 2 intr */
#define	MCFSIM_ICR_LEVEL3	0x0c	/* Level 3 intr */
#define	MCFSIM_ICR_LEVEL4	0x10	/* Level 4 intr */
#define	MCFSIM_ICR_LEVEL5	0x14	/* Level 5 intr */
#define	MCFSIM_ICR_LEVEL6	0x18	/* Level 6 intr */
#define	MCFSIM_ICR_LEVEL7	0x1c	/* Level 7 intr */

#define	MCFSIM_ICR_PRI0		0x00	/* Priority 0 intr */
#define	MCFSIM_ICR_PRI1		0x01	/* Priority 1 intr */
#define	MCFSIM_ICR_PRI2		0x02	/* Priority 2 intr */
#define	MCFSIM_ICR_PRI3		0x03	/* Priority 3 intr */

/*
 *  Macros to read/set IMR register. It is 32 bits on the 5249.
 */

#define	mcf_getimr()		\
	*((volatile unsigned long *) (MCF_MBAR + MCFSIM_IMR))

#define	mcf_setimr(imr)		\
	*((volatile unsigned long *) (MCF_MBAR + MCFSIM_IMR)) = (imr);

#endif				/* mcf5249_h */
