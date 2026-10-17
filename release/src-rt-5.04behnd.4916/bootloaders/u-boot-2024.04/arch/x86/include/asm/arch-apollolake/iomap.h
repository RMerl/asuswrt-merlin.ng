/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2017 Intel Corporation.
 * Take from coreboot project file of the same name
 */

#ifndef _ASM_ARCH_IOMAP_H
#define _ASM_ARCH_IOMAP_H

#define R_ACPI_PM1_TMR			0x8

/* Put p2sb at 0xd0000000 in TPL */
#define IOMAP_P2SB_BAR		0xd0000000
#define IOMAP_P2SB_SIZE		0x10000000

#define IOMAP_SPI_BASE		0xfe010000

#define IOMAP_ACPI_BASE		0x400
#define IOMAP_ACPI_SIZE		0x100
#define ACPI_BASE_ADDRESS	IOMAP_ACPI_BASE

#define PMC_BAR0		0xfe042000

#define MCH_BASE_ADDRESS	0xfed10000
#define MCH_SIZE		0x8000

#ifdef __ACPI__
#define HPET_BASE_ADDRESS	0xfed00000

#define SRAM_BASE_0		0xfe900000
#define SRAM_SIZE_0		(8 * KiB)
#define SRAM_BASE_2		0xfe902000
#define SRAM_SIZE_2		(4 * KiB)
#endif

/* Early address for I2C port 2 */
#define IOMAP_I2C2_BASE		(0xfe020000 + 2 * 0x1000)

/*
 * Use UART2. To use UART1 you need to set '2' to '1', change device tree serial
 * node name and 'reg' property, and update CONFIG_DEBUG_UART_BASE.
 */
#define PCH_DEV_UART		PCI_BDF(0, 0x18, 2)

#define PCH_DEV_LPC		PCI_BDF(0, 0x1f, 0)
#define PCH_DEV_SPI		PCI_BDF(0, 0x0d, 2)

#endif
