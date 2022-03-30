/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Chip-specific header file for the AT91SAM9x5 family
 *
 *  Copyright (C) 2012-2013 Atmel Corporation.
 *
 * Definitions for the SoC:
 * AT91SAM9x5 & AT91SAM9N12
 */

#ifndef __AT91SAM9X5_H__
#define __AT91SAM9X5_H__

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FIQ	0	/* Advanced Interrupt Controller (FIQ) */
#define ATMEL_ID_SYS	1	/* System Controller Interrupt */
#define ATMEL_ID_PIOAB	2	/* Parallel I/O Controller A and B */
#define ATMEL_ID_PIOCD	3	/* Parallel I/O Controller C and D */
#define ATMEL_ID_SMD	4	/* SMD Soft Modem (SMD), only for AT91SAM9X5 */
#define ATMEL_ID_FUSE	4	/* FUSE Controller, only for AT91SAM9N12 */
#define ATMEL_ID_USART0	5	/* USART 0 */
#define ATMEL_ID_USART1	6	/* USART 1 */
#define ATMEL_ID_USART2	7	/* USART 2 */
#define ATMEL_ID_USART3	8	/* USART 3 */
#define ATMEL_ID_TWI0	9	/* Two-Wire Interface 0 */
#define ATMEL_ID_TWI1	10	/* Two-Wire Interface 1 */
#define ATMEL_ID_TWI2	11	/* Two-Wire Interface 2 */
#define ATMEL_ID_HSMCI0	12	/* High Speed Multimedia Card Interface 0 */
#define ATMEL_ID_SPI0	13	/* Serial Peripheral Interface 0 */
#define ATMEL_ID_SPI1	14	/* Serial Peripheral Interface 1 */
#define ATMEL_ID_UART0	15	/* UART 0 */
#define ATMEL_ID_UART1	16	/* UART 1 */
#define ATMEL_ID_TC01	17	/* Timer Counter 0, 1, 2, 3, 4 and 5 */
#define ATMEL_ID_PWM	18	/* Pulse Width Modulation Controller */
#define ATMEL_ID_ADC	19	/* ADC Controller */
#define ATMEL_ID_DMAC0	20	/* DMA Controller 0 */
#define ATMEL_ID_DMAC1	21	/* DMA Controller 1 */
#define ATMEL_ID_UHPHS	22	/* USB Host High Speed */
#define ATMEL_ID_UDPHS	23	/* USB Device High Speed */
#define ATMEL_ID_EMAC0	24	/* Ethernet MAC0 */
#define ATMEL_ID_LCDC	25	/* LCD Controller */
#define ATMEL_ID_HSMCI1	26	/* High Speed Multimedia Card Interface 1 */
#define ATMEL_ID_EMAC1	27	/* Ethernet MAC1 */
#define ATMEL_ID_SSC	28	/* Synchronous Serial Controller */
#define ATMEL_ID_TRNG	30	/* True Random Number Generator */
#define ATMEL_ID_IRQ	31	/* Advanced Interrupt Controller */

/*
 * User Peripheral physical base addresses.
 */
#define ATMEL_BASE_SPI0		0xf0000000
#define ATMEL_BASE_SPI1		0xf0004000
#define ATMEL_BASE_HSMCI0	0xf0008000
#define ATMEL_BASE_HSMCI1	0xf000c000
#define ATMEL_BASE_SSC		0xf0010000
#define ATMEL_BASE_CAN0		0xf8000000
#define ATMEL_BASE_CAN1		0xf8004000
#define ATMEL_BASE_TC0		0xf8008000
#define ATMEL_BASE_TC1		0xf8008040
#define ATMEL_BASE_TC2		0xf8008080
#define ATMEL_BASE_TC3		0xf800c000
#define ATMEL_BASE_TC4		0xf800c040
#define ATMEL_BASE_TC5		0xf800c080
#define ATMEL_BASE_TWI0		0xf8010000
#define ATMEL_BASE_TWI1		0xf8014000
#define ATMEL_BASE_TWI2		0xf8018000
#define ATMEL_BASE_USART0	0xf801c000
#define ATMEL_BASE_USART1	0xf8020000
#define ATMEL_BASE_USART2	0xf8024000
#define ATMEL_BASE_USART3	0xf8028000
#define ATMEL_BASE_EMAC0	0xf802c000
#define ATMEL_BASE_EMAC1	0xf8030000
#define ATMEL_BASE_PWM		0xf8034000
#define ATMEL_BASE_LCDC		0xf8038000
#define ATMEL_BASE_UDPHS	0xf803c000
#define ATMEL_BASE_UART0	0xf8040000
#define ATMEL_BASE_UART1	0xf8044000
#define ATMEL_BASE_ISI		0xf8048000
#define ATMEL_BASE_ADC		0xf804c000
#define ATMEL_BASE_SYS		0xffffc000

/*
 * System Peripherals
 */
#define ATMEL_BASE_FUSE		0xffffdc00
#define ATMEL_BASE_MATRIX	0xffffde00
#define ATMEL_BASE_PMECC	0xffffe000
#define ATMEL_BASE_PMERRLOC	0xffffe600
#define ATMEL_BASE_DDRSDRC	0xffffe800
#define ATMEL_BASE_SMC		0xffffea00
#define ATMEL_BASE_DMAC0	0xffffec00
#define ATMEL_BASE_DMAC1	0xffffee00
#define ATMEL_BASE_AIC		0xfffff000
#define ATMEL_BASE_DBGU		0xfffff200
#define ATMEL_BASE_PIOA		0xfffff400
#define ATMEL_BASE_PIOB		0xfffff600
#define ATMEL_BASE_PIOC		0xfffff800
#define ATMEL_BASE_PIOD		0xfffffa00
#define ATMEL_BASE_PMC		0xfffffc00
#define ATMEL_BASE_RSTC		0xfffffe00
#define ATMEL_BASE_SHDWC	0xfffffe10
#define ATMEL_BASE_PIT		0xfffffe30
#define ATMEL_BASE_WDT		0xfffffe40
#define ATMEL_BASE_GPBR		0xfffffe60
#define ATMEL_BASE_RTC		0xfffffeb0

/*
 * Internal Memory.
 */
#define ATMEL_BASE_ROM		0x00100000 /* Internal ROM base address */
#define ATMEL_BASE_SRAM		0x00300000 /* Internal SRAM base address */

#ifdef CONFIG_AT91SAM9N12
#define ATMEL_BASE_OHCI		0x00500000 /* USB Host controller */
#else	/* AT91SAM9X5 */
#define ATMEL_BASE_SMD		0x00400000 /* SMD Controller */
#define ATMEL_BASE_UDPHS_FIFO	0x00500000 /* USB Device HS controller */
#define ATMEL_BASE_OHCI		0x00600000 /* USB Host controller (OHCI) */
#define ATMEL_BASE_EHCI		0x00700000 /* USB Host controller (EHCI) */
#endif

/*
 * External memory
 */
#define ATMEL_BASE_CS0		0x10000000
#define ATMEL_BASE_CS1		0x20000000
#define ATMEL_BASE_CS2		0x30000000
#define ATMEL_BASE_CS3		0x40000000
#define ATMEL_BASE_CS4		0x50000000
#define ATMEL_BASE_CS5		0x60000000

/* 9x5 series chip id definitions */
#define ARCH_ID_AT91SAM9X5	0x819a05a0
#define ARCH_ID_VERSION_MASK	0x1f
#define ARCH_EXID_AT91SAM9G15	0x00000000
#define ARCH_EXID_AT91SAM9G35	0x00000001
#define ARCH_EXID_AT91SAM9X35	0x00000002
#define ARCH_EXID_AT91SAM9G25	0x00000003
#define ARCH_EXID_AT91SAM9X25	0x00000004

#define cpu_is_at91sam9x5()	(get_chip_id() == ARCH_ID_AT91SAM9X5)
#define cpu_is_at91sam9g15()	(cpu_is_at91sam9x5() && \
			(get_extension_chip_id() == ARCH_EXID_AT91SAM9G15))
#define cpu_is_at91sam9g25()	(cpu_is_at91sam9x5() && \
			(get_extension_chip_id() == ARCH_EXID_AT91SAM9G25))
#define cpu_is_at91sam9g35()	(cpu_is_at91sam9x5() && \
			(get_extension_chip_id() == ARCH_EXID_AT91SAM9G35))
#define cpu_is_at91sam9x25()	(cpu_is_at91sam9x5() && \
			(get_extension_chip_id() == ARCH_EXID_AT91SAM9X25))
#define cpu_is_at91sam9x35()	(cpu_is_at91sam9x5() && \
			(get_extension_chip_id() == ARCH_EXID_AT91SAM9X35))

/*
 * Cpu Name
 */
#ifdef CONFIG_AT91SAM9N12
#define ATMEL_CPU_NAME	"AT91SAM9N12"
#else	/* AT91SAM9X5 */
#define ATMEL_CPU_NAME	get_cpu_name()
#endif

/* Timer */
#define CONFIG_SYS_TIMER_COUNTER	0xfffffe3c

/*
 * Other misc defines
 */
#define ATMEL_PIO_PORTS         4
#define ATMEL_PMC_UHP		AT91SAM926x_PMC_UHP
#define ATMEL_ID_UHP		ATMEL_ID_UHPHS

/*
 * PMECC table in ROM
 */
#define ATMEL_PMECC_INDEX_OFFSET_512	0x8000
#define ATMEL_PMECC_INDEX_OFFSET_1024	0x10000

/*
 * at91sam9x5 specific prototypes
 */
#ifndef __ASSEMBLY__
unsigned int get_chip_id(void);
unsigned int get_extension_chip_id(void);
unsigned int has_emac1(void);
unsigned int has_emac0(void);
unsigned int has_lcdc(void);
char *get_cpu_name(void);
#endif

#endif
