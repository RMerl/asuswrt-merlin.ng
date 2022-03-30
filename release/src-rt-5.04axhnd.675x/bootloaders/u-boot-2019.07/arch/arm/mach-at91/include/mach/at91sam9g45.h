/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Chip-specific header file for the AT91SAM9M1x family
 *
 * (C) 2008 Atmel Corporation.
 *
 * Definitions for the SoC:
 * AT91SAM9G45
 */

#ifndef AT91SAM9G45_H
#define AT91SAM9G45_H

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FIQ	0	/* Advanced Interrupt Controller (FIQ) */
#define ATMEL_ID_SYS	1	/* System Controller Interrupt */
#define ATMEL_ID_PIOA	2	/* Parallel I/O Controller A */
#define ATMEL_ID_PIOB	3	/* Parallel I/O Controller B */
#define ATMEL_ID_PIOC	4	/* Parallel I/O Controller C */
#define ATMEL_ID_PIODE	5	/* Parallel I/O Controller D and E */
#define ATMEL_ID_TRNG	6	/* True Random Number Generator */
#define ATMEL_ID_USART0	7	/* USART 0 */
#define ATMEL_ID_USART1	8	/* USART 1 */
#define ATMEL_ID_USART2	9	/* USART 2 */
#define ATMEL_ID_USART3	10	/* USART 3 */
#define ATMEL_ID_MCI0	11	/* High Speed Multimedia Card Interface 0 */
#define ATMEL_ID_TWI0	12	/* Two-Wire Interface 0 */
#define ATMEL_ID_TWI1	13	/* Two-Wire Interface 1 */
#define ATMEL_ID_SPI0	14	/* Serial Peripheral Interface 0 */
#define ATMEL_ID_SPI1	15	/* Serial Peripheral Interface 1 */
#define ATMEL_ID_SSC0	16	/* Synchronous Serial Controller 0 */
#define ATMEL_ID_SSC1	17	/* Synchronous Serial Controller 1 */
#define ATMEL_ID_TCB	18	/* Timer Counter 0, 1, 2, 3, 4 and 5 */
#define ATMEL_ID_PWMC	19	/* Pulse Width Modulation Controller */
#define ATMEL_ID_TSC	20	/* Touch Screen ADC Controller */
#define ATMEL_ID_DMA	21	/* DMA Controller */
#define ATMEL_ID_UHPHS	22	/* USB Host High Speed */
#define ATMEL_ID_LCDC	23	/* LCD Controller */
#define ATMEL_ID_AC97C	24	/* AC97 Controller */
#define ATMEL_ID_EMAC	25	/* Ethernet MAC */
#define ATMEL_ID_ISI	26	/* Image Sensor Interface */
#define ATMEL_ID_UDPHS	27	/* USB Device High Speed */
#define ATMEL_ID_AESTDESSHA 28	/* AES + T-DES + SHA */
#define ATMEL_ID_MCI1	29	/* High Speed Multimedia Card Interface 1 */
#define ATMEL_ID_VDEC	30	/* Video Decoder */
#define ATMEL_ID_IRQ0	31	/* Advanced Interrupt Controller */

/*
 * User Peripherals physical base addresses.
 */
#define ATMEL_BASE_UDPHS	0xfff78000
#define ATMEL_BASE_TC0		0xfff7c000
#define ATMEL_BASE_TC1		0xfff7c040
#define ATMEL_BASE_TC2		0xfff7c080
#define ATMEL_BASE_MCI0		0xfff80000
#define ATMEL_BASE_TWI0		0xfff84000
#define ATMEL_BASE_TWI1		0xfff88000
#define ATMEL_BASE_USART0	0xfff8c000
#define ATMEL_BASE_USART1	0xfff90000
#define ATMEL_BASE_USART2	0xfff94000
#define ATMEL_BASE_USART3	0xfff98000
#define ATMEL_BASE_SSC0		0xfff9c000
#define ATMEL_BASE_SSC1		0xfffa0000
#define ATMEL_BASE_SPI0		0xfffa4000
#define ATMEL_BASE_SPI1		0xfffa8000
#define ATMEL_BASE_AC97C	0xfffac000
#define ATMEL_BASE_TSC		0xfffb0000
#define ATMEL_BASE_ISI		0xfffb4000
#define ATMEL_BASE_PWMC		0xfffb8000
#define ATMEL_BASE_EMAC		0xfffbc000
#define ATMEL_BASE_AES		0xfffc0000
#define ATMEL_BASE_TDES		0xfffc4000
#define ATMEL_BASE_SHA		0xfffc8000
#define ATMEL_BASE_TRNG		0xfffcc000
#define ATMEL_BASE_MCI1		0xfffd0000
#define ATMEL_BASE_TC3		0xfffd4000
#define ATMEL_BASE_TC4		0xfffd4040
#define ATMEL_BASE_TC5		0xfffd4080
/* Reserved:	0xfffd8000 - 0xffffe1ff */

/*
 * System Peripherals physical base addresses.
 */
#define ATMEL_BASE_SYS		0xffffe200
#define ATMEL_BASE_ECC		0xffffe200
#define ATMEL_BASE_DDRSDRC1	0xffffe400
#define ATMEL_BASE_DDRSDRC0	0xffffe600
#define ATMEL_BASE_SMC		0xffffe800
#define ATMEL_BASE_MATRIX	0xffffea00
#define ATMEL_BASE_DMA		0xffffec00
#define ATMEL_BASE_DBGU		0xffffee00
#define ATMEL_BASE_AIC		0xfffff000
#define ATMEL_BASE_PIOA		0xfffff200
#define ATMEL_BASE_PIOB		0xfffff400
#define ATMEL_BASE_PIOC		0xfffff600
#define ATMEL_BASE_PIOD		0xfffff800
#define ATMEL_BASE_PIOE		0xfffffa00
#define ATMEL_BASE_PMC		0xfffffc00
#define ATMEL_BASE_RSTC		0xfffffd00
#define ATMEL_BASE_SHDWN	0xfffffd10
#define ATMEL_BASE_RTT		0xfffffd20
#define ATMEL_BASE_PIT		0xfffffd30
#define ATMEL_BASE_WDT		0xfffffd40
#define ATMEL_BASE_SCKCR	0xfffffd50
#define ATMEL_BASE_GPBR		0xfffffd60
#define ATMEL_BASE_RTC		0xfffffdb0
/* Reserved:	0xfffffdc0 - 0xffffffff */

/*
 * Internal Memory.
 */
#define ATMEL_BASE_SRAM		0x00300000	/* Internal SRAM base address */
#define ATMEL_BASE_ROM		0x00400000	/* Internal ROM base address */
#define ATMEL_BASE_LCDC		0x00500000	/* LCD Controller */
#define ATMEL_BASE_UDPHS_FIFO	0x00600000	/* USB Device HS controller */
#define ATMEL_BASE_HCI		0x00700000	/* USB Host controller (OHCI) */
#define ATMEL_BASE_EHCI		0x00800000	/* USB Host controller (EHCI) */
#define ATMEL_BASE_VDEC		0x00900000	/* Video Decoder Controller */

/*
 * External memory
 */
#define ATMEL_BASE_CS0		0x10000000
#define ATMEL_BASE_CS1		0x20000000
#define ATMEL_BASE_CS2		0x30000000
#define ATMEL_BASE_CS3		0x40000000
#define ATMEL_BASE_CS4		0x50000000
#define ATMEL_BASE_CS5		0x60000000
#define ATMEL_BASE_CS6		0x70000000
#define ATMEL_BASE_CS7		0x80000000

/* Timer */
#define CONFIG_SYS_TIMER_COUNTER	0xfffffd3c

/*
 * Other misc defines
 */
#define ATMEL_PIO_PORTS		5		/* this SoCs has 5 PIO */
#define ATMEL_BASE_PIO		ATMEL_BASE_PIOA
#define ATMEL_PMC_UHP		AT91SAM926x_PMC_UHP
#define ATMEL_ID_UHP		ATMEL_ID_UHPHS
/*
 * Cpu Name
 */
#define ATMEL_CPU_NAME		"AT91SAM9G45"

#endif
