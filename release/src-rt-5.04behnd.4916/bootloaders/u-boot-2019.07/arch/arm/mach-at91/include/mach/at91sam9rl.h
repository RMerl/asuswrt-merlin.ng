/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91sam9rl.h]
 *
 *  Copyright (C) 2007 Atmel Corporation
 *
 * Common definitions.
 * Based on AT91SAM9RL datasheet revision A. (Preliminary)
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive for
 * more details.
 */

#ifndef AT91SAM9RL_H
#define AT91SAM9RL_H

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FIQ	0	/* Advanced Interrupt Controller (FIQ) */
#define ATMEL_ID_SYS	1	/* System Peripherals */
#define ATMEL_ID_PIOA	2	/* Parallel IO Controller A */
#define ATMEL_ID_PIOB	3	/* Parallel IO Controller B */
#define ATMEL_ID_PIOC	4	/* Parallel IO Controller C */
#define ATMEL_ID_PIOD	5	/* Parallel IO Controller D */
#define ATMEL_ID_USART0	6	/* USART 0 */
#define ATMEL_ID_USART1	7	/* USART 1 */
#define ATMEL_ID_USART2	8	/* USART 2 */
#define ATMEL_ID_USART3	9	/* USART 3 */
#define ATMEL_ID_MCI	10	/* Multimedia Card Interface */
#define ATMEL_ID_TWI0	11	/* TWI 0 */
#define ATMEL_ID_TWI1	12	/* TWI 1 */
#define ATMEL_ID_SPI	13	/* Serial Peripheral Interface */
#define ATMEL_ID_SSC0	14	/* Serial Synchronous Controller 0 */
#define ATMEL_ID_SSC1	15	/* Serial Synchronous Controller 1 */
#define ATMEL_ID_TC0	16	/* Timer Counter 0 */
#define ATMEL_ID_TC1	17	/* Timer Counter 1 */
#define ATMEL_ID_TC2	18	/* Timer Counter 2 */
#define ATMEL_ID_PWMC	19	/* Pulse Width Modulation Controller */
#define ATMEL_ID_TSC	20	/* Touch Screen Controller */
#define ATMEL_ID_DMA	21	/* DMA Controller */
#define ATMEL_ID_UDPHS	22	/* USB Device HS */
#define ATMEL_ID_LCDC	23	/* LCD Controller */
#define ATMEL_ID_AC97C	24	/* AC97 Controller */
#define ATMEL_ID_IRQ0	31	/* Advanced Interrupt Controller (IRQ0) */

/*
 * User Peripheral physical base addresses.
 */
#define ATMEL_BASE_TCB0		0xfffa0000
#define ATMEL_BASE_TC0		0xfffa0000
#define ATMEL_BASE_TC1		0xfffa0040
#define ATMEL_BASE_TC2		0xfffa0080
#define ATMEL_BASE_MCI		0xfffa4000
#define ATMEL_BASE_TWI0		0xfffa8000
#define ATMEL_BASE_TWI1		0xfffac000
#define ATMEL_BASE_USART0	0xfffb0000
#define ATMEL_BASE_USART1	0xfffb4000
#define ATMEL_BASE_USART2	0xfffb8000
#define ATMEL_BASE_USART3	0xfffbc000
#define ATMEL_BASE_SSC0		0xfffc0000
#define ATMEL_BASE_SSC1		0xfffc4000
#define ATMEL_BASE_PWMC		0xfffc8000
#define ATMEL_BASE_SPI0		0xfffcc000
#define ATMEL_BASE_TSC		0xfffd0000
#define ATMEL_BASE_UDPHS	0xfffd4000
#define ATMEL_BASE_AC97C	0xfffd8000
#define ATMEL_BASE_SYS		0xffffc000

/*
 * System Peripherals
 */
#define ATMEL_BASE_DMA		0xffffe600
#define ATMEL_BASE_ECC		0xffffe800
#define ATMEL_BASE_SDRAMC	0xffffea00
#define ATMEL_BASE_SMC		0xffffec00
#define ATMEL_BASE_MATRIX	0xffffee00
#define ATMEL_BASE_CCFG		0xffffef10
#define ATMEL_BASE_AIC		0xfffff000
#define ATMEL_BASE_DBGU		0xfffff200
#define ATMEL_BASE_PIOA		0xfffff400
#define ATMEL_BASE_PIOB		0xfffff600
#define ATMEL_BASE_PIOC		0xfffff800
#define ATMEL_BASE_PIOD		0xfffffa00
#define ATMEL_BASE_PMC		0xfffffc00
#define ATMEL_BASE_RSTC		0xfffffd00
#define ATMEL_BASE_SHDWC	0xfffffd10
#define ATMEL_BASE_RTT		0xfffffd20
#define ATMEL_BASE_PIT		0xfffffd30
#define ATMEL_BASE_WDT		0xfffffd40
#define ATMEL_BASE_SCKCR	0xfffffd50
#define ATMEL_BASE_GPBR		0xfffffd60
#define ATMEL_BASE_RTC		0xfffffe00

/*
 * Internal Memory.
 */
#define ATMEL_BASE_SRAM		0x00300000	/* Internal SRAM base address */
#define ATMEL_BASE_ROM		0x00400000	/* Internal ROM base address */

#define ATMEL_BASE_LCDC		0x00500000	/* LCD Controller */
#define ATMEL_UHP_BASE		0x00600000	/* USB Device HS controller */

/*
 * External memory
 */
#define ATMEL_BASE_CS0		0x10000000
#define ATMEL_BASE_CS1		0x20000000	/* SDRAM */
#define ATMEL_BASE_CS2		0x30000000
#define ATMEL_BASE_CS3		0x40000000	/* NAND */
#define ATMEL_BASE_CS4		0x50000000	/* Compact Flash Slot 0 */
#define ATMEL_BASE_CS5		0x60000000	/* Compact Flash Slot 1 */

/* Timer */
#define CONFIG_SYS_TIMER_COUNTER	0xfffffd3c

/*
 * Other misc defines
 */
#define ATMEL_PIO_PORTS		4		/* this SoC has 4 PIO */
#define ATMEL_BASE_PIO		ATMEL_BASE_PIOA

/*
 * Cpu Name
 */
#define ATMEL_CPU_NAME		"AT91SAM9RL"

#endif
