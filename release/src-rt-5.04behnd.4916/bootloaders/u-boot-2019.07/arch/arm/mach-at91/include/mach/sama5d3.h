/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Chip-specific header file for the SAMA5D3 family
 *
 * (C) 2012 - 2013 Atmel Corporation.
 * Bo Shen <voice.shen@atmel.com>
 *
 * Definitions for the SoC:
 * SAMA5D3
 */

#ifndef SAMA5D3_H
#define SAMA5D3_H

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FIQ	0	/* Advanced Interrupt Controller (FIQ) */
#define ATMEL_ID_SYS	1	/* System Controller Interrupt */
#define ATMEL_ID_DBGU	2	/* Debug Unit Interrupt */
#define ATMEL_ID_PIT	3	/* Periodic Interval Timer Interrupt */
#define ATMEL_ID_WDT	4	/* Watchdog timer Interrupt */
#define ATMEL_ID_SMC	5	/* Multi-bit ECC Interrupt */
#define ATMEL_ID_PIOA	6	/* Parallel I/O Controller A */
#define ATMEL_ID_PIOB	7	/* Parallel I/O Controller B */
#define ATMEL_ID_PIOC	8	/* Parallel I/O Controller C */
#define ATMEL_ID_PIOD	9	/* Parallel I/O Controller D */
#define ATMEL_ID_PIOE	10	/* Parallel I/O Controller E */
#define ATMEL_ID_SMD	11	/* SMD Soft Modem */
#define ATMEL_ID_USART0	12	/* USART 0 */
#define ATMEL_ID_USART1	13	/* USART 1 */
#define ATMEL_ID_USART2	14	/* USART 2 */
#define ATMEL_ID_USART3	15	/* USART 3 */
#define ATMEL_ID_UART0	16
#define ATMEL_ID_UART1	17
#define ATMEL_ID_TWI0	18	/* Two-Wire Interface 0 */
#define ATMEL_ID_TWI1	19	/* Two-Wire Interface 1 */
#define ATMEL_ID_TWI2	20	/* Two-Wire Interface 2 */
#define ATMEL_ID_MCI0	21	/* High Speed Multimedia Card Interface 0 */
#define ATMEL_ID_MCI1	22	/*  */
#define ATMEL_ID_MCI2	23	/*  */
#define ATMEL_ID_SPI0	24	/* Serial Peripheral Interface 0 */
#define ATMEL_ID_SPI1	25	/* Serial Peripheral Interface 1 */
#define ATMEL_ID_TC0	26	/* */
#define ATMEL_ID_TC1	27	/* */
#define ATMEL_ID_PWMC	28	/* Pulse Width Modulation Controller */
#define ATMEL_ID_TSC	29	/* Touch Screen ADC Controller */
#define ATMEL_ID_DMA0	30	/* DMA Controller */
#define ATMEL_ID_DMA1	31	/* DMA Controller */
#define ATMEL_ID_UHPHS	32	/* USB Host High Speed */
#define ATMEL_ID_UDPHS	33	/* USB Device High Speed */
#define ATMEL_ID_GMAC	34
#define ATMEL_ID_EMAC	35	/* Ethernet MAC */
#define ATMEL_ID_LCDC	36	/* LCD Controller */
#define ATMEL_ID_ISI	37	/* Image Sensor Interface */
#define ATMEL_ID_SSC0	38	/* Synchronous Serial Controller 0 */
#define ATMEL_ID_SSC1	39	/* Synchronous Serial Controller 1 */
#define ATMEL_ID_CAN0	40
#define ATMEL_ID_CAN1	41
#define ATMEL_ID_SHA	42
#define ATMEL_ID_AES	43
#define ATMEL_ID_TDES	44
#define ATMEL_ID_TRNG	45
#define ATMEL_ID_ARM	46
#define ATMEL_ID_IRQ0	47	/* Advanced Interrupt Controller */
#define ATMEL_ID_FUSE	48
#define ATMEL_ID_MPDDRC	49

/* sama5d3 series chip id definitions */
#define ARCH_ID_SAMA5D3		0x8a5c07c0
#define ARCH_EXID_SAMA5D31	0x00444300
#define ARCH_EXID_SAMA5D33	0x00414300
#define ARCH_EXID_SAMA5D34	0x00414301
#define ARCH_EXID_SAMA5D35	0x00584300
#define ARCH_EXID_SAMA5D36	0x00004301

#define cpu_is_sama5d3()	(get_chip_id() == ARCH_ID_SAMA5D3)
#define cpu_is_sama5d31()	(cpu_is_sama5d3() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA5D31))
#define cpu_is_sama5d33()	(cpu_is_sama5d3() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA5D33))
#define cpu_is_sama5d34()	(cpu_is_sama5d3() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA5D34))
#define cpu_is_sama5d35()	(cpu_is_sama5d3() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA5D35))
#define cpu_is_sama5d36()	(cpu_is_sama5d3() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA5D36))

/*
 * User Peripherals physical base addresses.
 */
#define ATMEL_BASE_MCI0		0xf0000000
#define ATMEL_BASE_SPI0		0xf0004000
#define ATMEL_BASE_SSC0		0xf000C000
#define ATMEL_BASE_TC2		0xf0010000
#define ATMEL_BASE_TWI0		0xf0014000
#define ATMEL_BASE_TWI1		0xf0018000
#define ATMEL_BASE_USART0	0xf001c000
#define ATMEL_BASE_USART1	0xf0020000
#define ATMEL_BASE_UART0	0xf0024000
#define ATMEL_BASE_GMAC		0xf0028000
#define ATMEL_BASE_PWMC		0xf002c000
#define ATMEL_BASE_LCDC		0xf0030000
#define ATMEL_BASE_ISI		0xf0034000
#define ATMEL_BASE_SFR		0xf0038000
/* Reserved: 0xf003c000 - 0xf8000000 */
#define ATMEL_BASE_MCI1		0xf8000000
#define ATMEL_BASE_MCI2		0xf8004000
#define ATMEL_BASE_SPI1		0xf8008000
#define ATMEL_BASE_SSC1		0xf800c000
#define ATMEL_BASE_CAN1		0xf8010000
#define ATMEL_BASE_TC3		0xf8014000
#define ATMEL_BASE_TSADC	0xf8018000
#define ATMEL_BASE_TWI2		0xf801c000
#define ATMEL_BASE_USART2	0xf8020000
#define ATMEL_BASE_USART3	0xf8024000
#define ATMEL_BASE_UART1	0xf8028000
#define ATMEL_BASE_EMAC		0xf802c000
#define ATMEL_BASE_UDPHS	0xf8030000
#define ATMEL_BASE_SHA		0xf8034000
#define ATMEL_BASE_AES		0xf8038000
#define ATMEL_BASE_TDES		0xf803c000
#define ATMEL_BASE_TRNG		0xf8040000
/* Reserved:	0xf804400 - 0xffffc00 */

/*
 * System Peripherals physical base addresses.
 */
#define ATMEL_BASE_SYS		0xffffc000
#define ATMEL_BASE_SMC		0xffffc000
#define ATMEL_BASE_PMECC	(ATMEL_BASE_SMC + 0x070)
#define ATMEL_BASE_PMERRLOC	(ATMEL_BASE_SMC + 0x500)
#define ATMEL_BASE_FUSE		0xffffe400
#define ATMEL_BASE_DMAC0	0xffffe600
#define ATMEL_BASE_DMAC1	0xffffe800
#define ATMEL_BASE_MPDDRC	0xffffea00
#define ATMEL_BASE_MATRIX	0xffffec00
#define ATMEL_BASE_DBGU		0xffffee00
#define ATMEL_BASE_AIC		0xfffff000
#define ATMEL_BASE_PIOA		0xfffff200
#define ATMEL_BASE_PIOB		0xfffff400
#define ATMEL_BASE_PIOC		0xfffff600
#define ATMEL_BASE_PIOD		0xfffff800
#define ATMEL_BASE_PIOE		0xfffffa00
#define ATMEL_BASE_PMC		0xfffffc00
#define ATMEL_BASE_RSTC		0xfffffe00
#define ATMEL_BASE_SHDWN	0xfffffe10
#define ATMEL_BASE_PIT		0xfffffe30
#define ATMEL_BASE_WDT		0xfffffe40
#define ATMEL_BASE_SCKCR	0xfffffe50
#define ATMEL_BASE_GPBR		0xfffffe60
#define ATMEL_BASE_RTC		0xfffffeb0
/* Reserved:	0xfffffee0 - 0xffffffff */

#define ATMEL_CHIPID_CIDR	0xffffee40
#define ATMEL_CHIPID_EXID	0xffffee44

/*
 * Internal Memory.
 */
#define ATMEL_BASE_ROM		0x00100000	/* Internal ROM base address */
#define ATMEL_BASE_SRAM		0x00200000	/* Internal ROM base address */
#define ATMEL_BASE_SRAM0	0x00300000	/* Internal SRAM base address */
#define ATMEL_BASE_SRAM1	0x00310000	/* Internal SRAM base address */
#define ATMEL_BASE_SMD		0x00400000	/* Internal ROM base address */
#define ATMEL_BASE_UDPHS_FIFO	0x00500000	/* USB Device HS controller */
#define ATMEL_BASE_OHCI		0x00600000	/* USB Host controller (OHCI) */
#define ATMEL_BASE_EHCI		0x00700000	/* USB Host controller (EHCI) */
#define ATMEL_BASE_AXI		0x00800000	/* Video Decoder Controller */
#define ATMEL_BASE_DAP		0x00900000	/* Video Decoder Controller */

/*
 * External memory
 */
#define ATMEL_BASE_CS0		0x10000000
#define ATMEL_BASE_DDRCS	0x20000000
#define ATMEL_BASE_CS1		0x40000000
#define ATMEL_BASE_CS2		0x50000000
#define ATMEL_BASE_CS3		0x60000000

/*
 * Other misc defines
 */
#define ATMEL_PIO_PORTS		5
#define CPU_HAS_PCR

/* Timer */
#define CONFIG_SYS_TIMER_COUNTER	0xfffffe3c

/*
 * PMECC table in ROM
 */
#define ATMEL_PMECC_INDEX_OFFSET_512	0x10000
#define ATMEL_PMECC_INDEX_OFFSET_1024	0x18000

/*
 * SAMA5D3 specific prototypes
 */
#ifndef __ASSEMBLY__
unsigned int get_chip_id(void);
unsigned int get_extension_chip_id(void);
unsigned int has_emac(void);
unsigned int has_gmac(void);
unsigned int has_lcdc(void);
char *get_cpu_name(void);
#endif

#endif
