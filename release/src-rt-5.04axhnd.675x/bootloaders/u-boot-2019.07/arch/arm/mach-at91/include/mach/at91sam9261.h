/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91sam9261.h]
 *
 * Copyright (C) SAN People
 * (C) Copyright 2010
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
 *
 * Definitions for the SoCs:
 * AT91SAM9261, AT91SAM9G10
 *
 * Note that those SoCs are mostly software and pin compatible,
 * therefore this file applies to all of them. Differences between
 * those SoCs are concentrated at the end of this file.
 */

#ifndef AT91SAM9261_H
#define AT91SAM9261_H

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FIQ	0	/* Advanced Interrupt Controller (FIQ) */
#define ATMEL_ID_SYS	1	/* System Peripherals */
#define ATMEL_ID_PIOA	2	/* Parallel IO Controller A */
#define ATMEL_ID_PIOB	3	/* Parallel IO Controller B */
#define ATMEL_ID_PIOC	4	/* Parallel IO Controller C */
/* Reserved:		5 */
#define ATMEL_ID_USART0	6	/* USART 0 */
#define ATMEL_ID_USART1	7	/* USART 1 */
#define ATMEL_ID_USART2	8	/* USART 2 */
#define ATMEL_ID_MCI	9	/* Multimedia Card Interface */
#define ATMEL_ID_UDP	10	/* USB Device Port */
#define ATMEL_ID_TWI0	11	/* Two-Wire Interface 0 */
#define ATMEL_ID_SPI0	12	/* Serial Peripheral Interface 0 */
#define ATMEL_ID_SPI1	13	/* Serial Peripheral Interface 1 */
#define ATMEL_ID_SSC0	14	/* Serial Synchronous Controller 0 */
#define ATMEL_ID_SSC1	15	/* Serial Synchronous Controller 1 */
#define ATMEL_ID_SSC2	16	/* Serial Synchronous Controller 2 */
#define ATMEL_ID_TC0	17	/* Timer Counter 0 */
#define ATMEL_ID_TC1	18	/* Timer Counter 1 */
#define ATMEL_ID_TC2	19	/* Timer Counter 2 */
#define ATMEL_ID_UHP	20	/* USB Host port */
#define ATMEL_ID_LCDC	21	/* LDC Controller */
/* Reserved:		22-28 */
#define ATMEL_ID_IRQ0	29	/* Advanced Interrupt Controller (IRQ0) */
#define ATMEL_ID_IRQ1	30	/* Advanced Interrupt Controller (IRQ1) */
#define ATMEL_ID_IRQ2	31	/* Advanced Interrupt Controller (IRQ2) */

/*
 * User Peripherals physical base addresses.
 */
#define ATMEL_BASE_TCB0		0xfffa0000
#define ATMEL_BASE_TC0		0xfffa0000
#define ATMEL_BASE_TC1		0xfffa0040
#define ATMEL_BASE_TC2		0xfffa0080
#define ATMEL_BASE_UDP0		0xfffa4000
#define ATMEL_BASE_MCI		0xfffa8000
#define ATMEL_BASE_TWI0		0xfffac000
#define ATMEL_BASE_USART0	0xfffb0000
#define ATMEL_BASE_USART1	0xfffb4000
#define ATMEL_BASE_USART2	0xfffb8000
#define ATMEL_BASE_SSC0		0xfffbc000
#define ATMEL_BASE_SSC1		0xfffc0000
#define ATMEL_BASE_SSC2		0xfffc4000
#define ATMEL_BASE_SPI0		0xfffc8000
#define ATMEL_BASE_SPI1		0xfffcc000
/* Reserved:	0xfffc4000 - 0xffffe9ff */

/*
 * System Peripherals physical base addresses.
 */
#define ATMEL_BASE_SYS		0xffffea00
#define ATMEL_BASE_SDRAMC	0xffffea00
#define ATMEL_BASE_SMC		0xffffec00
#define ATMEL_BASE_MATRIX	0xffffee00
#define ATMEL_BASE_AIC		0xfffff000
#define ATMEL_BASE_DBGU		0xfffff200
#define ATMEL_BASE_PIOA		0xfffff400
#define ATMEL_BASE_PIOB		0xfffff600
#define ATMEL_BASE_PIOC		0xfffff800
#define ATMEL_BASE_PMC		0xfffffc00
#define ATMEL_BASE_RSTC		0xfffffd00
#define ATMEL_BASE_SHDWN	0xfffffd10
#define ATMEL_BASE_RTT		0xfffffd20
#define ATMEL_BASE_PIT		0xfffffd30
#define ATMEL_BASE_WDT		0xfffffd40
#define ATMEL_BASE_GPBR		0xfffffd50

/*
 * Internal Memory common on all these SoCs
 */
#define ATMEL_BASE_SRAM		0x00300000	/* Internal SRAM base address */
#define ATMEL_SIZE_SRAM		0x00028000	/* Internal SRAM size (160Kb) */

#define ATMEL_BASE_ROM		0x00400000	/* Internal ROM base address */
#define ATMEL_SIZE_ROM		0x00008000	/* Internal ROM size (32Kb) */

#define ATMEL_BASE_UHP		0x00500000	/* USB Host controller */
#define ATMEL_BASE_LCDC		0x00600000	/* LDC controller */

/*
 * External memory
 */
#define ATMEL_BASE_CS0		0x10000000	/* typically NOR */
#define ATMEL_BASE_CS1		0x20000000	/* SDRAM */
#define ATMEL_BASE_CS2		0x30000000
#define ATMEL_BASE_CS3		0x40000000	/* typically NAND */
#define ATMEL_BASE_CS4		0x50000000
#define ATMEL_BASE_CS5		0x60000000
#define ATMEL_BASE_CS6		0x70000000
#define ATMEL_BASE_CS7		0x80000000

/* Timer */
#define CONFIG_SYS_TIMER_COUNTER	0xfffffd3c

/*
 * Other misc defines
 */
#define ATMEL_PIO_PORTS		3		/* theese SoCs have 3 PIO */
#define ATMEL_PMC_UHP		AT91SAM926x_PMC_UHP
#define ATMEL_BASE_PIO		ATMEL_BASE_PIOA

/*
 * SoC specific defines
 */
#if defined(CONFIG_AT91SAM9261)
# define ATMEL_CPU_NAME		"AT91SAM9261"
#elif defined(CONFIG_AT91SAM9G10)
# define ATMEL_CPU_NAME		"AT91SAM9G10"
#endif

#endif
