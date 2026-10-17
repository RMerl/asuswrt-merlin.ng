/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Chip-specific header file for the SAM9X60 SoC.
 *
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries
 */

#ifndef __SAM9X60_H__
#define __SAM9X60_H__

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FIQ		0	/* Advanced Interrupt Controller */
#define ATMEL_ID_SYS		1	/* System Controller Interrupt */
#define ATMEL_ID_PIOA		2	/* Parallel I/O Controller A */
#define ATMEL_ID_PIOB		3	/* Parallel I/O Controller B */
#define ATMEL_ID_PIOC		4	/* Parallel I/O Controller C */
#define ATMEL_ID_FLEXCOM0	5	/* FLEXCOM 0 */
#define ATMEL_ID_FLEXCOM1	6	/* FLEXCOM 1 */
#define ATMEL_ID_FLEXCOM2	7	/* FLEXCOM 2 */
#define ATMEL_ID_FLEXCOM3	8	/* FLEXCOM 3 */
#define ATMEL_ID_FLEXCOM6	9	/* FLEXCOM 6 */
#define ATMEL_ID_FLEXCOM7	10	/* FLEXCOM 7 */
#define ATMEL_ID_FLEXCOM8	11	/* FLEXCOM 8 */
#define ATMEL_ID_SDMMC0		12	/* SDMMC 0 */
#define ATMEL_ID_FLEXCOM4	13	/* FLEXCOM 4 */
#define ATMEL_ID_FLEXCOM5	14	/* FLEXCOM 5 */
#define ATMEL_ID_FLEXCOM9	15	/* FLEXCOM 9 */
#define ATMEL_ID_FLEXCOM10	16	/* FLEXCOM 10 */
#define ATMEL_ID_TC01		17	/* Timer Counter 0, 1, 2, 3, 4 and 5 */
#define ATMEL_ID_PWM		18	/* Pulse Width Modulation Controller */
#define ATMEL_ID_ADC		19	/* ADC Controller */
#define ATMEL_ID_XDMAC0		20	/* XDMA Controller 0 */
#define ATMEL_ID_MATRIX		21	/* BUS Matrix */
#define ATMEL_ID_UHPHS		22	/* USB Host High Speed */
#define ATMEL_ID_UDPHS		23	/* USB Device High Speed */
#define ATMEL_ID_EMAC0		24	/* Ethernet MAC 0 */
#define ATMEL_ID_LCDC		25	/* LCD Controller */
#define ATMEL_ID_SDMMC1		26	/* SDMMC 1 */
#define ATMEL_ID_EMAC1		27	/* Ethernet MAC `1 */
#define ATMEL_ID_SSC		28	/* Synchronous Serial Controller */
#define ATMEL_ID_IRQ		31	/* Advanced Interrupt Controller */
#define ATMEL_ID_TRNG		38	/* True Random Number Generator */
#define ATMEL_ID_PIOD		44	/* Parallel I/O Controller D */
#define ATMEL_ID_DBGU		47	/* Debug unit */

/*
 * User Peripheral physical base addresses.
 */
#define ATMEL_BASE_FLEXCOM4	0xf0000000
#define ATMEL_BASE_FLEXCOM5	0xf0004000
#define ATMEL_BASE_XDMA0	0xf0008000
#define ATMEL_BASE_SSC		0xf0010000
#define ATMEL_BASE_QSPI		0xf0014000
#define ATMEL_BASE_CAN0		0xf8000000
#define ATMEL_BASE_CAN1		0xf8004000
#define ATMEL_BASE_TC0		0xf8008000
#define ATMEL_BASE_TC1		0xf8008040
#define ATMEL_BASE_TC2		0xf8008080
#define ATMEL_BASE_TC3		0xf800c000
#define ATMEL_BASE_TC4		0xf800c040
#define ATMEL_BASE_TC5		0xf800c080
#define ATMEL_BASE_FLEXCOM6	0xf8010000
#define ATMEL_BASE_FLEXCOM7	0xf8014000
#define ATMEL_BASE_FLEXCOM8	0xf8018000
#define ATMEL_BASE_FLEXCOM0	0xf801c000
#define ATMEL_BASE_FLEXCOM1	0xf8020000
#define ATMEL_BASE_FLEXCOM2	0xf8024000
#define ATMEL_BASE_FLEXCOM3	0xf8028000
#define ATMEL_BASE_EMAC0	0xf802c000
#define ATMEL_BASE_EMAC1	0xf8030000
#define ATMEL_BASE_PWM		0xf8034000
#define ATMEL_BASE_LCDC		0xf8038000
#define ATMEL_BASE_UDPHS	0xf803c000
#define ATMEL_BASE_FLEXCOM9	0xf8040000
#define ATMEL_BASE_FLEXCOM10 0xf8044000
#define ATMEL_BASE_ISI		0xf8048000
#define ATMEL_BASE_ADC		0xf804c000
#define ATMEL_BASE_SFR		0xf8050000
#define ATMEL_BASE_SYS		0xffffc000

/*
 * System Peripherals
 */
#define ATMEL_BASE_MATRIX	0xffffde00
#define ATMEL_BASE_PMECC	0xffffe000
#define ATMEL_BASE_PMERRLOC	0xffffe600
#define ATMEL_BASE_MPDDRC	0xffffe800
#define ATMEL_BASE_SMC		0xffffea00
#define ATMEL_BASE_SDRAMC	0xffffec00
#define ATMEL_BASE_AIC		0xfffff100
#define ATMEL_BASE_DBGU		0xfffff200
#define ATMEL_BASE_PIOA		0xfffff400
#define ATMEL_BASE_PIOB		0xfffff600
#define ATMEL_BASE_PIOC		0xfffff800
#define ATMEL_BASE_PIOD		0xfffffa00
#define ATMEL_BASE_PMC		0xfffffc00
#define ATMEL_BASE_RSTC		0xfffffe00
#define ATMEL_BASE_SHDWC	0xfffffe10
#define ATMEL_BASE_PIT		0xfffffe40
#define ATMEL_BASE_GPBR		0xfffffe60
#define ATMEL_BASE_RTC		0xfffffea8
#define ATMEL_BASE_WDT		0xffffff80

/*
 * Internal Memory.
 */
#define ATMEL_BASE_ROM		0x00100000 /* Internal ROM base address */
#define ATMEL_BASE_SRAM		0x00300000 /* Internal SRAM base address */
#define ATMEL_BASE_UDPHS_FIFO	0x00500000 /* USB Device HS controller */
#define ATMEL_BASE_OHCI		0x00600000 /* USB Host controller (OHCI) */
#define ATMEL_BASE_EHCI		0x00700000 /* USB Host controller (EHCI) */

/*
 * External memory
 */
#define ATMEL_BASE_CS0		0x10000000
#define ATMEL_BASE_CS1		0x20000000
#define ATMEL_BASE_CS2		0x30000000
#define ATMEL_BASE_CS3		0x40000000
#define ATMEL_BASE_CS4		0x50000000
#define ATMEL_BASE_CS5		0x60000000
#define ATMEL_BASE_SDMMC0	0x80000000
#define ATMEL_BASE_SDMMC1	0x90000000

/* 9x60 series chip id definitions */
#define ARCH_ID_SAM9X60		0x819b35a0
#define ARCH_ID_VERSION_MASK	0x1f
#define ARCH_EXID_SAM9X60	0x00000000
#define ARCH_EXID_SAM9X60_D6K	0x00000011
#define ARCH_EXID_SAM9X60_D5M	0x00000001
#define ARCH_EXID_SAM9X60_D1G	0x00000010

#define cpu_is_sam9x60()	(get_chip_id() == ARCH_ID_SAM9X60)

/*
 * Cpu Name
 */
#define ATMEL_CPU_NAME	get_cpu_name()

/* Timer */
#define CFG_SYS_TIMER_COUNTER	0xfffffe4c

/*
 * Other misc defines
 */
#define ATMEL_PIO_PORTS	4
#define CPU_HAS_PCR
#define CPU_NO_PLLB
#define PLL_ID_PLLA 0
#define PLL_ID_UPLL 1

/*
 * PMECC table in ROM
 */
#define ATMEL_PMECC_INDEX_OFFSET_512	0x0000
#define ATMEL_PMECC_INDEX_OFFSET_1024	0x8000

/*
 * SAM9X60 specific prototypes
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
