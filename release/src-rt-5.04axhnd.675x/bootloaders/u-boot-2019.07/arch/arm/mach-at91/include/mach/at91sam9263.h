/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91sam9263.h]
 *
 * (C) 2007 Atmel Corporation.
 * (C) Copyright 2010
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
 *
 * Definitions for the SoC:
 * AT91SAM9263
 */

#ifndef AT91SAM9263_H
#define AT91SAM9263_H

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FIQ	0	/* Advanced Interrupt Controller (FIQ) */
#define ATMEL_ID_SYS	1	/* System Peripherals */
#define ATMEL_ID_PIOA	2	/* Parallel IO Controller A */
#define ATMEL_ID_PIOB	3	/* Parallel IO Controller B */
#define ATMEL_ID_PIOCDE	4	/* Parallel IO Controller C, D and E */
/* Reserved:		5 */
/* Reserved:		6 */
#define ATMEL_ID_USART0	7	/* USART 0 */
#define ATMEL_ID_USART1	8	/* USART 1 */
#define ATMEL_ID_USART2	9	/* USART 2 */
#define ATMEL_ID_MCI0	10	/* Multimedia Card Interface 0 */
#define ATMEL_ID_MCI1	11	/* Multimedia Card Interface 1 */
#define ATMEL_ID_CAN	12	/* CAN */
#define ATMEL_ID_TWI	13	/* Two-Wire Interface */
#define ATMEL_ID_SPI0	14	/* Serial Peripheral Interface 0 */
#define ATMEL_ID_SPI1	15	/* Serial Peripheral Interface 1 */
#define ATMEL_ID_SSC0	16	/* Serial Synchronous Controller 0 */
#define ATMEL_ID_SSC1	17	/* Serial Synchronous Controller 1 */
#define ATMEL_ID_AC97C	18	/* AC97 Controller */
#define ATMEL_ID_TCB	19	/* Timer Counter 0, 1 and 2 */
#define ATMEL_ID_PWMC	20	/* Pulse Width Modulation Controller */
#define ATMEL_ID_EMAC	21	/* Ethernet */
/* Reserved:		22 */
#define ATMEL_ID_2DGE	23	/* 2D Graphic Engine */
#define ATMEL_ID_UDP	24	/* USB Device Port */
#define ATMEL_ID_ISI	25	/* Image Sensor Interface */
#define ATMEL_ID_LCDC	26	/* LCD Controller */
#define ATMEL_ID_DMA	27	/* DMA Controller */
/* Reserved:		28 */
#define ATMEL_ID_UHP	29	/* USB Host port */
#define ATMEL_ID_IRQ0	30	/* Advanced Interrupt Controller (IRQ0) */
#define ATMEL_ID_IRQ1	31	/* Advanced Interrupt Controller (IRQ1) */

/*
 * User Peripherals physical base addresses.
 */
#define ATMEL_BASE_UDP		0xfff78000
#define ATMEL_BASE_TCB0		0xfff7c000
#define ATMEL_BASE_TC0		0xfff7c000
#define ATMEL_BASE_TC1		0xfff7c040
#define ATMEL_BASE_TC2		0xfff7c080
#define ATMEL_BASE_MCI0		0xfff80000
#define ATMEL_BASE_MCI1		0xfff84000
#define ATMEL_BASE_TWI		0xfff88000
#define ATMEL_BASE_USART0	0xfff8c000
#define ATMEL_BASE_USART1	0xfff90000
#define ATMEL_BASE_USART2	0xfff94000
#define ATMEL_BASE_SSC0		0xfff98000
#define ATMEL_BASE_SSC1		0xfff9c000
#define ATMEL_BASE_AC97C	0xfffa0000
#define ATMEL_BASE_SPI0		0xfffa4000
#define ATMEL_BASE_SPI1		0xfffa8000
#define ATMEL_BASE_CAN		0xfffac000
#define ATMEL_BASE_PWMC		0xfffb8000
#define ATMEL_BASE_EMAC		0xfffbc000
#define ATMEL_BASE_ISI		0xfffc4000
#define ATMEL_BASE_2DGE		0xfffc8000

/*
 * System Peripherals physical base addresses.
 */
#define ATMEL_BASE_ECC0		0xffffe000
#define ATMEL_BASE_SDRAMC0	0xffffe200
#define ATMEL_BASE_SMC0		0xffffe400
#define ATMEL_BASE_ECC1		0xffffe600
#define ATMEL_BASE_SDRAMC1	0xffffe800
#define ATMEL_BASE_SMC1		0xffffea00
#define ATMEL_BASE_MATRIX	0xffffec00
#define ATMEL_BASE_CCFG		0xffffed10
#define ATMEL_BASE_DBGU		0xffffee00
#define ATMEL_BASE_AIC		0xfffff000
#define ATMEL_BASE_PIOA		0xfffff200
#define ATMEL_BASE_PIOB		0xfffff400
#define ATMEL_BASE_PIOC		0xfffff600
#define ATMEL_BASE_PIOD		0xfffff800
#define ATMEL_BASE_PIOE		0xfffffa00
#define ATMEL_BASE_PMC		0xfffffc00
#define ATMEL_BASE_RSTC		0xfffffd00
#define ATMEL_BASE_SHDWC	0xfffffd10
#define ATMEL_BASE_RTT0		0xfffffd20
#define ATMEL_BASE_PIT		0xfffffd30
#define ATMEL_BASE_WDT		0xfffffd40
#define ATMEL_BASE_RTT1		0xfffffd50
#define ATMEL_BASE_GPBR		0xfffffd60

/*
 * Internal Memory.
 */
#define ATMEL_BASE_SRAM0	0x00300000	/* Internal SRAM 0 */

#define ATMEL_BASE_ROM		0x00400000	/* Internal ROM */

#define ATMEL_BASE_SRAM1	0x00500000	/* Internal SRAM 1 */

#define ATMEL_BASE_LCDC		0x00700000	/* LCD Controller */
#define ATMEL_BASE_DMAC		0x00800000	/* DMA Controller */
#define ATMEL_BASE_UHP		0x00a00000	/* USB Host controller */

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
#define ATMEL_PIO_PORTS		5		/* this SoCs has 5 PIO */
#define ATMEL_BASE_PIO		ATMEL_BASE_PIOA
#define ATMEL_PMC_UHP		AT91SAM926x_PMC_UHP

/*
 * Cpu Name
 */
#define ATMEL_CPU_NAME		"AT91SAM9263"

#endif
