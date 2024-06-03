/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Chip-specific header file for the SAMA5D4 SoC
 *
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 */

#ifndef __SAMA5D4_H
#define __SAMA5D4_H

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FIQ	0	/* FIQ Interrupt */
#define ATMEL_ID_SYS	1	/* System Controller */
#define ATMEL_ID_ARM	2	/* Performance Monitor Unit */
#define ATMEL_ID_PIT	3	/* Periodic Interval Timer */
#define ATMEL_ID_WDT	4	/* Watchdog timer */
#define ATMEL_ID_PIOD	5	/* Parallel I/O Controller D */
#define ATMEL_ID_USART0	6	/* USART 0 */
#define ATMEL_ID_USART1	7	/* USART 1 */
#define ATMEL_ID_DMA0	8	/* DMA Controller 0 */
#define ATMEL_ID_ICM	9	/* Integrity Check Monitor */
#define ATMEL_ID_PKCC	10	/* Public Key Crypto Controller */
#define ATMEL_ID_AES	12	/* Advanced Encryption Standard */
#define ATMEL_ID_AESB	13	/* AES Bridge*/
#define ATMEL_ID_TDES	14	/* Triple Data Encryption Standard */
#define ATMEL_ID_SHA    15	/* SHA Signature */
#define ATMEL_ID_MPDDRC	16	/* MPDDR controller */
#define ATMEL_ID_MATRIX1	17	/* H32MX, 32-bit AHB Matrix */
#define ATMEL_ID_MATRIX0	18	/* H64MX, 64-bit AHB Matrix */
#define ATMEL_ID_VDEC	19	/* Video Decoder */
#define ATMEL_ID_SBM	20	/* Secure Box Module */
#define ATMEL_ID_SMC	22	/* Multi-bit ECC interrupt */
#define ATMEL_ID_PIOA	23	/* Parallel I/O Controller A */
#define ATMEL_ID_PIOB	24	/* Parallel I/O Controller B */
#define ATMEL_ID_PIOC	25	/* Parallel I/O Controller C */
#define ATMEL_ID_PIOE	26	/* Parallel I/O Controller E */
#define ATMEL_ID_UART0	27	/* UART 0 */
#define ATMEL_ID_UART1	28	/* UART 1 */
#define ATMEL_ID_USART2	29	/* USART 2 */
#define ATMEL_ID_USART3	30	/* USART 3 */
#define ATMEL_ID_USART4	31	/* USART 4 */
#define ATMEL_ID_TWI0	32	/* Two-Wire Interface 0 */
#define ATMEL_ID_TWI1	33	/* Two-Wire Interface 1 */
#define ATMEL_ID_TWI2	34	/* Two-Wire Interface 2 */
#define ATMEL_ID_MCI0	35	/* High Speed Multimedia Card Interface 0 */
#define ATMEL_ID_MCI1	36	/* High Speed Multimedia Card Interface 1 */
#define ATMEL_ID_SPI0	37	/* Serial Peripheral Interface 0 */
#define ATMEL_ID_SPI1	38	/* Serial Peripheral Interface 1 */
#define ATMEL_ID_SPI2	39	/* Serial Peripheral Interface 2 */
#define ATMEL_ID_TC0	40	/* Timer Counter 0 (ch. 0, 1, 2) */
#define ATMEL_ID_TC1	41	/* Timer Counter 1 (ch. 3, 4, 5) */
#define ATMEL_ID_TC2	42	/* Timer Counter 2 (ch. 6, 7, 8) */
#define ATMEL_ID_PWMC	43	/* Pulse Width Modulation Controller */
#define ATMEL_ID_ADC	44	/* Touch Screen ADC Controller */
#define ATMEL_ID_DBGU	45	/* Debug Unit Interrupt */
#define ATMEL_ID_UHPHS	46	/* USB Host High Speed */
#define ATMEL_ID_UDPHS	47	/* USB Device High Speed */
#define ATMEL_ID_SSC0	48	/* Synchronous Serial Controller 0 */
#define ATMEL_ID_SSC1	49	/* Synchronous Serial Controller 1 */
#define ATMEL_ID_XDMAC1	50	/* DMA Controller 1 */
#define ATMEL_ID_LCDC	51	/* LCD Controller */
#define ATMEL_ID_ISI	52	/* Image Sensor Interface */
#define ATMEL_ID_TRNG	53	/* True Random Number Generator */
#define ATMEL_ID_GMAC0	54	/* Ethernet MAC 0 */
#define ATMEL_ID_GMAC1	55	/* Ethernet MAC 1 */
#define ATMEL_ID_IRQ	56	/* IRQ Interrupt ID */
#define ATMEL_ID_SFC	57	/* Fuse Controller */
#define ATMEL_ID_SECURAM	59	/* Secured RAM */
#define ATMEL_ID_SMD	61	/* SMD Soft Modem */
#define ATMEL_ID_TWI3	62	/* Two-Wire Interface 3 */
#define ATMEL_ID_CATB	63	/* Capacitive Touch Controller */
#define ATMEL_ID_SFR	64	/* Special Funcion Register */
#define ATMEL_ID_AIC	65	/* Advanced Interrupt Controller */
#define ATMEL_ID_SAIC	66	/* Secured Advanced Interrupt Controller */
#define ATMEL_ID_L2CC	67	/* L2 Cache Controller */

/*
 * User Peripherals physical base addresses.
 */
#define ATMEL_BASE_LCDC		0xf0000000
#define ATMEL_BASE_DMAC1	0xf0004000
#define ATMEL_BASE_ISI		0xf0008000
#define ATMEL_BASE_PKCC		0xf000C000
#define ATMEL_BASE_MPDDRC	0xf0010000
#define ATMEL_BASE_DMAC0	0xf0014000
#define ATMEL_BASE_PMC		0xf0018000
#define ATMEL_BASE_MATRIX0	0xf001c000
#define ATMEL_BASE_AESB		0xf0020000
/* Reserved: 0xf0024000 - 0xf8000000 */
#define ATMEL_BASE_MCI0		0xf8000000
#define ATMEL_BASE_UART0	0xf8004000
#define ATMEL_BASE_SSC0		0xf8008000
#define ATMEL_BASE_PWMC		0xf800c000
#define ATMEL_BASE_SPI0		0xf8010000
#define ATMEL_BASE_TWI0		0xf8014000
#define ATMEL_BASE_TWI1		0xf8018000
#define ATMEL_BASE_TC0		0xf801c000
#define ATMEL_BASE_GMAC0	0xf8020000
#define ATMEL_BASE_TWI2		0xf8024000
#define ATMEL_BASE_SFR		0xf8028000
#define ATMEL_BASE_USART0	0xf802c000
#define ATMEL_BASE_USART1	0xf8030000
/* Reserved:	0xf8034000 - 0xfc000000 */
#define ATMEL_BASE_MCI1		0xfc000000
#define ATMEL_BASE_UART1	0xfc004000
#define ATMEL_BASE_USART2	0xfc008000
#define ATMEL_BASE_USART3	0xfc00c000
#define ATMEL_BASE_USART4	0xfc010000
#define ATMEL_BASE_SSC1		0xfc014000
#define ATMEL_BASE_SPI1		0xfc018000
#define ATMEL_BASE_SPI2		0xfc01c000
#define ATMEL_BASE_TC1		0xfc020000
#define ATMEL_BASE_TC2		0xfc024000
#define ATMEL_BASE_GMAC1	0xfc028000
#define ATMEL_BASE_UDPHS	0xfc02c000
#define ATMEL_BASE_TRNG		0xfc030000
#define ATMEL_BASE_ADC		0xfc034000
#define ATMEL_BASE_TWI3		0xfc038000

#define ATMEL_BASE_MATRIX1	0xfc054000

#define ATMEL_BASE_SMC		0xfc05c000
#define ATMEL_BASE_PMECC	(ATMEL_BASE_SMC + 0x070)
#define ATMEL_BASE_PMERRLOC	(ATMEL_BASE_SMC + 0x500)

#define ATMEL_BASE_PIOD		0xfc068000
#define ATMEL_BASE_RSTC		0xfc068600
#define ATMEL_BASE_PIT		0xfc068630
#define ATMEL_BASE_WDT		0xfc068640

#define ATMEL_BASE_DBGU		0xfc069000
#define ATMEL_BASE_PIOA		0xfc06a000
#define ATMEL_BASE_PIOB		0xfc06b000
#define ATMEL_BASE_PIOC		0xfc06c000
#define ATMEL_BASE_PIOE		0xfc06d000
#define ATMEL_BASE_AIC		0xfc06e000

#define ATMEL_CHIPID_CIDR	0xfc069040
#define ATMEL_CHIPID_EXID	0xfc069044

/*
 * Internal Memory.
 */
#define ATMEL_BASE_ROM		0x00000000	/* Internal ROM base address */
#define ATMEL_BASE_NFC		0x00100000	/* NFC SRAM */
#define ATMEL_BASE_SRAM		0x00200000	/* Internal ROM base address */
#define ATMEL_BASE_VDEC		0x00300000	/* Video Decoder Controller */
#define ATMEL_BASE_UDPHS_FIFO	0x00400000	/* USB Device HS controller */
#define ATMEL_BASE_OHCI		0x00500000	/* USB Host controller (OHCI) */
#define ATMEL_BASE_EHCI		0x00600000	/* USB Host controller (EHCI) */
#define ATMEL_BASE_AXI		0x00700000
#define ATMEL_BASE_DAP		0x00800000
#define ATMEL_BASE_SMD		0x00900000

/*
 * External memory
 */
#define ATMEL_BASE_CS0		0x10000000
#define ATMEL_BASE_DDRCS	0x20000000
#define ATMEL_BASE_CS1		0x60000000
#define ATMEL_BASE_CS2		0x70000000
#define ATMEL_BASE_CS3		0x80000000

/*
 * Other misc defines
 */
#define ATMEL_PIO_PORTS		5
#define CPU_HAS_PCR
#define CPU_HAS_H32MXDIV

/* MATRIX0(H64MX) slave id definitions */
#define H64MX_SLAVE_AXIMX_BRIDGE	0	/* Bridge from H64MX to AXIMX */
#define H64MX_SLAVE_PERIPH_BRIDGE	1	/* H64MX Peripheral Bridge */
#define H64MX_SLAVE_VDEC		2	/* Video Decoder */
#define H64MX_SLAVE_DDRC_PORT0		3	/* DDR2 Port0-AESOTF */
#define H64MX_SLAVE_DDRC_PORT1		4	/* DDR2 Port1 */
#define H64MX_SLAVE_DDRC_PORT2		5	/* DDR2 Port2 */
#define H64MX_SLAVE_DDRC_PORT3		6	/* DDR2 Port3 */
#define H64MX_SLAVE_DDRC_PORT4		7	/* DDR2 Port4 */
#define H64MX_SLAVE_DDRC_PORT5		8	/* DDR2 Port5 */
#define H64MX_SLAVE_DDRC_PORT6		9	/* DDR2 Port6 */
#define H64MX_SLAVE_DDRC_PORT7		10	/* DDR2 Port7 */
#define H64MX_SLAVE_SRAM		11	/* Internal SRAM 128K */
#define H64MX_SLAVE_H32MX_BRIDGE	12	/* Bridge from H64MX to H32MX */

/* MATRIX1(H32MX) slave id definitions */
#define H32MX_SLAVE_H64MX_BRIDGE	0	/* Bridge from H32MX to H64MX */
#define H32MX_SLAVE_PERIPH_BRIDGE0	1	/* H32MX Peripheral Bridge 0 */
#define H32MX_SLAVE_PERIPH_BRIDGE1	2	/* H32MX Peripheral Bridge 1 */
#define H32MX_SLAVE_EBI			3	/* External Bus Interface */
#define H32MX_SLAVE_NFC_CMD		3	/* NFC command Register */
#define H32MX_SLAVE_NFC_SRAM		4	/* NFC SRAM */
#define H32MX_SLAVE_USB			5	/* USB Device & Host */
#define H32MX_SLAVE_SMD			6	/* Soft Modem (SMD) */

/* AICREDIR Unlock Key */
#define ATMEL_SFR_AICREDIR_KEY		0x5F67B102

/* sama5d4 series chip id definitions */
#define ARCH_ID_SAMA5D4		0x8a5c07c0
#define ARCH_EXID_SAMA5D41	0x00000001
#define ARCH_EXID_SAMA5D42	0x00000002
#define ARCH_EXID_SAMA5D43	0x00000003
#define ARCH_EXID_SAMA5D44	0x00000004

#define cpu_is_sama5d4()	(get_chip_id() == ARCH_ID_SAMA5D4)
#define cpu_is_sama5d41()	(cpu_is_sama5d4() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA5D41))
#define cpu_is_sama5d42()	(cpu_is_sama5d4() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA5D42))
#define cpu_is_sama5d43()	(cpu_is_sama5d4() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA5D43))
#define cpu_is_sama5d44()	(cpu_is_sama5d4() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA5D44))

/* Timer */
#define CONFIG_SYS_TIMER_COUNTER	0xfc06863c

/*
 * No PMECC Galois table in ROM
 */
#define NO_GALOIS_TABLE_IN_ROM

#ifndef __ASSEMBLY__
unsigned int get_chip_id(void);
unsigned int get_extension_chip_id(void);
unsigned int has_lcdc(void);
char *get_cpu_name(void);
#endif

#endif
