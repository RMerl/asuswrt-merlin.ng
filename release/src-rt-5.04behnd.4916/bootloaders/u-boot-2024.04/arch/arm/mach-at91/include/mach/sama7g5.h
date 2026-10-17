/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Chip-specific header file for the SAMA7G5 SoC
 *
 * Copyright (C) 2020 Microchip Technology, Inc. and its subsidiaries
 *		      Eugen Hristev <eugen.hristev@microchip.com>
 */

#ifndef __SAMA7G5_H__
#define __SAMA7G5_H__

/*
 * Peripheral identifiers/interrupts.
 */
#define ATMEL_ID_FLEXCOM0	38
#define ATMEL_ID_FLEXCOM1	39
#define ATMEL_ID_FLEXCOM2	40
#define ATMEL_ID_FLEXCOM3	41

#define ATMEL_ID_SDMMC0		80
#define ATMEL_ID_SDMMC1		81

#define ATMEL_ID_PIT64B0	70
#define ATMEL_ID_PIT64B		ATMEL_ID_PIT64B0

#define ATMEL_CHIPID_CIDR	0xe0020000
#define ATMEL_CHIPID_EXID	0xe0020004
/*
 * User Peripherals physical base addresses.
 */
#define ATMEL_BASE_PIOA		0xe0014000
#define ATMEL_BASE_PIOB		(ATMEL_BASE_PIOA + 0x40)
#define ATMEL_BASE_PIOC		(ATMEL_BASE_PIOB + 0x40)
#define ATMEL_BASE_PIOD		(ATMEL_BASE_PIOC + 0x40)
#define ATMEL_BASE_PIOE		(ATMEL_BASE_PIOD + 0x40)

#define ATMEL_PIO_PORTS		5

#define CPU_HAS_PCR

#define ATMEL_BASE_PMC		0xe0018000

#define ATMEL_BASE_WDT		0xe001c000
#define ATMEL_BASE_RSTC		0xe001d000
#define ATMEL_BASE_WDTS		0xe001d180
#define ATMEL_BASE_SCKCR	0xe001d050

#define ATMEL_BASE_SDMMC0	0xe1204000
#define ATMEL_BASE_SDMMC1	0xe1208000

#define ATMEL_BASE_PIT64B0	0xe1800000

#define ATMEL_BASE_FLEXCOM0	0xe1818000
#define ATMEL_BASE_FLEXCOM1	0xe181c000
#define ATMEL_BASE_FLEXCOM2	0xe1820000
#define ATMEL_BASE_FLEXCOM3	0xe1824000
#define ATMEL_BASE_FLEXCOM4	0xe2018000

#define ATMEL_BASE_TZC400	0xe3000000

#define ATMEL_BASE_UMCTL2	0xe3800000
#define ATMEL_BASE_UMCTL2_MP	0xe38003f8
#define ATMEL_BASE_PUBL		0xe3804000

#define ATMEL_NUM_FLEXCOM	12
#define ATMEL_PIO_PORTS		5

#define ATMEL_BASE_PIT64BC	ATMEL_BASE_PIT64B0

/* SAMA7G5 series chip id definitions */
#define ARCH_ID_SAMA7G5		0x80162100
#define ARCH_EXID_SAMA7G51	0x00000003
#define ARCH_EXID_SAMA7G52	0x00000002
#define ARCH_EXID_SAMA7G53	0x00000001
#define ARCH_EXID_SAMA7G54	0x00000000
#define ARCH_EXID_SAMA7G54_D1G	0x00000018
#define ARCH_EXID_SAMA7G54_D2G	0x00000020
#define ARCH_EXID_SAMA7G54_D4G	0x00000028

#define cpu_is_sama7g5()	(get_chip_id() == ARCH_ID_SAMA7G5)
#define cpu_is_sama7g51()	(cpu_is_sama7g5() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7G51))
#define cpu_is_sama7g52()	(cpu_is_sama7g5() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7G52))
#define cpu_is_sama7g53()	(cpu_is_sama7g5() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7G53))
#define cpu_is_sama7g54()	(cpu_is_sama7g5() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7G54))
#define cpu_is_sama7g54d1g()	(cpu_is_sama7g5() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7G54_D1G))
#define cpu_is_sama7g54d2g()	(cpu_is_sama7g5() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7G54_D2G))
#define cpu_is_sama7g54d4g()	(cpu_is_sama7g5() && \
		(get_extension_chip_id() == ARCH_EXID_SAMA7G54_D4G))

#ifndef __ASSEMBLY__
unsigned int get_chip_id(void);
unsigned int get_extension_chip_id(void);
char *get_cpu_name(void);
#endif

#endif /* #ifndef __SAMA7G5_H__ */
