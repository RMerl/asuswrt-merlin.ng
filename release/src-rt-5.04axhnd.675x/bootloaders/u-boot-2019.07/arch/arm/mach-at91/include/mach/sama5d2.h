/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Chip-specific header file for the SAMA5D2 SoC
 *
 * Copyright (C) 2015 Atmel
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 */

#ifndef __SAMA5D2_H
#define __SAMA5D2_H

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FIQ		0	/* FIQ Interrupt ID */
/* 1 */
#define ATMEL_ID_ARM		2	/* Performance Monitor Unit */
#define ATMEL_ID_PIT		3	/* Periodic Interval Timer Interrupt */
#define ATMEL_ID_WDT		4	/* Watchdog Timer Interrupt */
#define ATMEL_ID_GMAC		5	/* Ethernet MAC */
#define ATMEL_ID_XDMAC0		6	/* DMA Controller 0 */
#define ATMEL_ID_XDMAC1		7	/* DMA Controller 1 */
#define ATMEL_ID_ICM		8	/* Integrity Check Monitor */
#define ATMEL_ID_AES		9	/* Advanced Encryption Standard */
#define ATMEL_ID_AESB		10	/* AES bridge */
#define ATMEL_ID_TDES		11	/* Triple Data Encryption Standard */
#define ATMEL_ID_SHA		12	/* SHA Signature */
#define ATMEL_ID_MPDDRC		13	/* MPDDR Controller */
#define ATMEL_ID_MATRIX1	14	/* H32MX, 32-bit AHB Matrix */
#define ATMEL_ID_MATRIX0	15	/* H64MX, 64-bit AHB Matrix */
#define ATMEL_ID_SECUMOD	16	/* Secure Module */
#define ATMEL_ID_HSMC		17	/* Multi-bit ECC interrupt */
#define ATMEL_ID_PIOA		18	/* Parallel I/O Controller A */
#define ATMEL_ID_FLEXCOM0	19	/* FLEXCOM0 */
#define ATMEL_ID_FLEXCOM1	20	/* FLEXCOM1 */
#define ATMEL_ID_FLEXCOM2	21	/* FLEXCOM2 */
#define ATMEL_ID_FLEXCOM3	22	/* FLEXCOM3 */
#define ATMEL_ID_FLEXCOM4	23	/* FLEXCOM4 */
#define ATMEL_ID_UART0		24	/* UART0 */
#define ATMEL_ID_UART1		25	/* UART1 */
#define ATMEL_ID_UART2		26	/* UART2 */
#define ATMEL_ID_UART3		27	/* UART3 */
#define ATMEL_ID_UART4		28	/* UART4 */
#define ATMEL_ID_TWIHS0		29	/* Two-wire Interface 0 */
#define ATMEL_ID_TWIHS1		30	/* Two-wire Interface 1 */
#define ATMEL_ID_SDMMC0		31	/* Secure Data Memory Card Controller 0 */
#define ATMEL_ID_SDMMC1		32	/* Secure Data Memory Card Controller 1 */
#define ATMEL_ID_SPI0		33	/* Serial Peripheral Interface 0 */
#define ATMEL_ID_SPI1		34	/* Serial Peripheral Interface 1 */
#define ATMEL_ID_TC0		35	/* Timer Counter 0 (ch.0,1,2) */
#define ATMEL_ID_TC1		36	/* Timer Counter 1 (ch.3,4,5) */
/* 37 */
#define ATMEL_ID_PWM		38	/* PWMController0 (ch. 0,1,2,3) */
/* 39 */
#define ATMEL_ID_ADC		40	/* Touch Screen ADC Controller */
#define ATMEL_ID_UHPHS		41	/* USB Host High Speed */
#define ATMEL_ID_UDPHS		42	/* USB Device High Speed */
#define ATMEL_ID_SSC0		43	/* Serial Synchronous Controller 0 */
#define ATMEL_ID_SSC1		44	/* Serial Synchronous Controller 1 */
#define ATMEL_ID_LCDC		45	/* LCD Controller */
#define ATMEL_ID_ISI		46	/* Image Sensor Controller, for A5D2, named after ISC */
#define ATMEL_ID_TRNG		47	/* True Random Number Generator */
#define ATMEL_ID_PDMIC		48	/* PDM Interface Controller */
#define ATMEL_ID_AIC_IRQ	49	/* IRQ Interrupt ID */
#define ATMEL_ID_SFC		50	/* Fuse Controller */
#define ATMEL_ID_SECURAM	51	/* Secure RAM */
#define ATMEL_ID_QSPI0		52	/* QSPI0 */
#define ATMEL_ID_QSPI1		53	/* QSPI1 */
#define ATMEL_ID_I2SC0		54	/* Inter-IC Sound Controller 0 */
#define ATMEL_ID_I2SC1		55	/* Inter-IC Sound Controller 1 */
#define ATMEL_ID_CAN0_INT0	56	/* MCAN 0 Interrupt0 */
#define ATMEL_ID_CAN1_INT0	57	/* MCAN 1 Interrupt0 */
/* 58 */
#define ATMEL_ID_CLASSD		59	/* Audio Class D Amplifier */
#define ATMEL_ID_SFR		60	/* Special Function Register */
#define ATMEL_ID_SAIC		61	/* Secured AIC */
#define ATMEL_ID_AIC		62	/* Advanced Interrupt Controller */
#define ATMEL_ID_L2CC		63	/* L2 Cache Controller */
#define ATMEL_ID_CAN0_INT1	64	/* MCAN 0 Interrupt1 */
#define ATMEL_ID_CAN1_INT1	65	/* MCAN 1 Interrupt1 */
#define ATMEL_ID_GMAC_Q1	66	/* GMAC Queue 1 Interrupt */
#define ATMEL_ID_GMAC_Q2	67	/* GMAC Queue 2 Interrupt */
#define ATMEL_ID_PIOB		68	/* Parallel I/O Controller B */
#define ATMEL_ID_PIOC		69	/* Parallel I/O Controller C */
#define ATMEL_ID_PIOD		70	/* Parallel I/O Controller D */
#define ATMEL_ID_SDMMC0_TIMER	71	/* Secure Data Memory Card Controller 0 (TIMER) */
#define ATMEL_ID_SDMMC1_TIMER	72	/* Secure Data Memory Card Controller 1 (TIMER) */
/* 73 */
#define ATMEL_ID_SYS		74	/* System Controller Interrupt */
#define ATMEL_ID_ACC		75	/* Analog Comparator */
#define ATMEL_ID_RXLP		76	/* UART Low-Power */
#define ATMEL_ID_SFRBU		77	/* Special Function Register BackUp */
#define ATMEL_ID_CHIPID		78	/* Chip ID */

/*
 * User Peripherals physical base addresses.
 */
#define ATMEL_BASE_LCDC		0xf0000000
#define ATMEL_BASE_XDMAC1	0xf0004000
#define ATMEL_BASE_MPDDRC	0xf000c000
#define ATMEL_BASE_XDMAC0	0xf0010000
#define ATMEL_BASE_PMC		0xf0014000
#define ATMEL_BASE_MATRIX0	0xf0018000
#define ATMEL_BASE_QSPI0	0xf0020000
#define ATMEL_BASE_QSPI1	0xf0024000
#define ATMEL_BASE_SPI0		0xf8000000
#define ATMEL_BASE_GMAC		0xf8008000
#define ATMEL_BASE_TC0		0xf800c000
#define ATMEL_BASE_TC1		0xf8010000
#define ATMEL_BASE_HSMC		0xf8014000
#define ATMEL_BASE_UART0	0xf801c000
#define ATMEL_BASE_UART1	0xf8020000
#define ATMEL_BASE_UART2	0xf8024000
#define ATMEL_BASE_TWI0		0xf8028000
#define ATMEL_BASE_SFR		0xf8030000
#define ATMEL_BASE_SYSC		0xf8048000
#define ATMEL_BASE_SPI1		0xfc000000
#define ATMEL_BASE_UART3	0xfc008000
#define ATMEL_BASE_UART4	0xfc00c000
#define ATMEL_BASE_TWI1		0xfc028000
#define ATMEL_BASE_UDPHS	0xfc02c000

#define ATMEL_BASE_PIOA		0xfc038000
#define ATMEL_BASE_MATRIX1	0xfc03c000

#define ATMEL_CHIPID_CIDR	0xfc069000
#define ATMEL_CHIPID_EXID	0xfc069004

/*
 * Address Memory Space
 */
#define ATMEL_BASE_CS0			0x10000000
#define ATMEL_BASE_DDRCS		0x20000000
#define ATMEL_BASE_CS1			0x60000000
#define ATMEL_BASE_CS2			0x70000000
#define ATMEL_BASE_CS3			0x80000000
#define ATMEL_BASE_QSPI0_AES_MEM	0x90000000
#define ATMEL_BASE_QSPI1_AES_MEM	0x98000000
#define ATMEL_BASE_SDMMC0		0xa0000000
#define ATMEL_BASE_SDMMC1		0xb0000000
#define ATMEL_BASE_QSPI0_MEM		0xd0000000
#define ATMEL_BASE_QSPI1_MEM		0xd8000000

/*
 * Internal Memories
 */
#define ATMEL_BASE_UDPHS_FIFO	0x00300000	/* USB Device HS controller */
#define ATMEL_BASE_OHCI		0x00400000	/* USB Host controller (OHCI) */
#define ATMEL_BASE_EHCI		0x00500000	/* USB Host controller (EHCI) */

/*
 * SYSC Spawns
 */
#define ATMEL_BASE_RSTC		ATMEL_BASE_SYSC
#define ATMEL_BASE_SHDWC	(ATMEL_BASE_SYSC + 0x10)
#define ATMEL_BASE_PIT		(ATMEL_BASE_SYSC + 0x30)
#define ATMEL_BASE_WDT		(ATMEL_BASE_SYSC + 0x40)
#define ATMEL_BASE_SCKC		(ATMEL_BASE_SYSC + 0x50)
#define ATMEL_BASE_RTC		(ATMEL_BASE_SYSC + 0xb0)

/*
 * Other misc definitions
 */
#define ATMEL_BASE_PMECC	(ATMEL_BASE_HSMC + 0x70)
#define ATMEL_BASE_PMERRLOC	(ATMEL_BASE_HSMC + 0x500)
#define ATMEL_BASE_SMC		(ATMEL_BASE_HSMC + 0x700)

#define ATMEL_BASE_PIOB		(ATMEL_BASE_PIOA + 0x40)
#define ATMEL_BASE_PIOC		(ATMEL_BASE_PIOB + 0x40)
#define ATMEL_BASE_PIOD		(ATMEL_BASE_PIOC + 0x40)

#define ATMEL_PIO_PORTS		4
#define CPU_HAS_PCR
#define CPU_HAS_H32MXDIV

/* AICREDIR Unlock Key */
#define ATMEL_SFR_AICREDIR_KEY		0xB6D81C4D

/* MATRIX0(H64MX) slave id definitions */
#define H64MX_SLAVE_AXIMX_BRIDGE	0	/* Bridge from H64MX to AXIMX */
#define H64MX_SLAVE_PERIPH_BRIDGE	1	/* H64MX Peripheral Bridge */
#define H64MX_SLAVE_DDRC_PORT0		2	/* DDR2 Port0-AESOTF */
#define H64MX_SLAVE_DDRC_PORT1		3	/* DDR2 Port1 */
#define H64MX_SLAVE_DDRC_PORT2		4	/* DDR2 Port2 */
#define H64MX_SLAVE_DDRC_PORT3		5	/* DDR2 Port3 */
#define H64MX_SLAVE_DDRC_PORT4		6	/* DDR2 Port4 */
#define H64MX_SLAVE_DDRC_PORT5		7	/* DDR2 Port5 */
#define H64MX_SLAVE_DDRC_PORT6		8	/* DDR2 Port6 */
#define H64MX_SLAVE_DDRC_PORT7		9	/* DDR2 Port7 */
#define H64MX_SLAVE_SRAM		10	/* Internal SRAM 128K */
#define H64MX_SLAVE_CACHE_L2		11	/* Internal SRAM 128K(L2) */
#define H64MX_SLAVE_QSPI0		12	/* QSPI0 */
#define H64MX_SLAVE_QSPI1		13	/* QSPI1 */
#define H64MX_SLAVE_AESB		14	/* AESB */

/* MATRIX1(H32MX) slave id definitions */
#define H32MX_SLAVE_H64MX_BRIDGE	0	/* Bridge from H32MX to H64MX */
#define H32MX_SLAVE_PERIPH_BRIDGE0	1	/* H32MX Peripheral Bridge 0 */
#define H32MX_SLAVE_PERIPH_BRIDGE1	2	/* H32MX Peripheral Bridge 1 */
#define H32MX_SLAVE_EBI			3	/* External Bus Interface */
#define H32MX_SLAVE_NFC_CMD		3	/* NFC command Register */
#define H32MX_SLAVE_NFC_SRAM		4	/* NFC SRAM */
#define H32MX_SLAVE_USB			5	/* USB Device & Host */

/* SAMA5D2 series chip id definitions */
#define ARCH_ID_SAMA5D2		0x8a5c08c0
#define ARCH_EXID_SAMA5D21CU	0x0000005a
#define ARCH_EXID_SAMA5D22CU	0x00000059
#define ARCH_EXID_SAMA5D22CN	0x00000069
#define ARCH_EXID_SAMA5D23CU	0x00000058
#define ARCH_EXID_SAMA5D24CX	0x00000004
#define ARCH_EXID_SAMA5D24CU	0x00000014
#define ARCH_EXID_SAMA5D26CU	0x00000012
#define ARCH_EXID_SAMA5D27CU	0x00000011
#define ARCH_EXID_SAMA5D27CN	0x00000021
#define ARCH_EXID_SAMA5D28CU	0x00000010
#define ARCH_EXID_SAMA5D28CN	0x00000020

#define ARCH_ID_SAMA5D2_SIP		0x8a5c08c2
#define ARCH_EXID_SAMA5D225C_D1M	0x00000053
#define ARCH_EXID_SAMA5D27C_D5M		0x00000032
#define ARCH_EXID_SAMA5D27C_D1G		0x00000033
#define ARCH_EXID_SAMA5D28C_D1G		0x00000013

/* Checked if defined in ethernet driver macb */
#define cpu_is_sama5d2	_cpu_is_sama5d2

/* PIT Timer(PIT_PIIR) */
#define CONFIG_SYS_TIMER_COUNTER	0xf804803c

/* No PMECC Galois table in ROM */
#define NO_GALOIS_TABLE_IN_ROM

#ifndef __ASSEMBLY__
unsigned int get_chip_id(void);
unsigned int get_extension_chip_id(void);
int _cpu_is_sama5d2(void);
unsigned int has_lcdc(void);
char *get_cpu_name(void);
#endif

#endif
