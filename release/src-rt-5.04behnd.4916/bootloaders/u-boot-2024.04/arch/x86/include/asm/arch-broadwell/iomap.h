/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * From Coreboot soc/intel/broadwell/include/soc/iomap.h
 *
 * Copyright (C) 2016 Google Inc.
 */

#ifndef __asm_arch_iomap_h
#define __asm_arch_iomap_h

#define MCFG_BASE_ADDRESS	0xf0000000
#define MCFG_BASE_SIZE		0x4000000

#define HPET_BASE_ADDRESS	0xfed00000

#define MCH_BASE_ADDRESS	0xfed10000
#define MCH_BASE_SIZE		0x8000

#define DMI_BASE_ADDRESS	0xfed18000
#define DMI_BASE_SIZE		0x1000

#define EP_BASE_ADDRESS		0xfed19000
#define EP_BASE_SIZE		0x1000

#define EDRAM_BASE_ADDRESS	0xfed80000
#define EDRAM_BASE_SIZE		0x4000

#define GDXC_BASE_ADDRESS	0xfed84000
#define GDXC_BASE_SIZE		0x1000

#define RCBA_BASE_ADDRESS	0xfed1c000
#define RCBA_BASE_SIZE		0x4000

#define HPET_BASE_ADDRESS	0xfed00000

#define ACPI_BASE_ADDRESS	0x1000
#define ACPI_BASE_SIZE		0x100

#define GPIO_BASE_ADDRESS	0x1400
#define GPIO_BASE_SIZE		0x400

#define SMBUS_BASE_ADDRESS	0x0400
#define SMBUS_BASE_SIZE		0x10

/* Temporary addresses used before relocation */
#define EARLY_GTT_BAR		0xe0000000
#define EARLY_XHCI_BAR		0xd7000000
#define EARLY_EHCI_BAR		0xd8000000
#define EARLY_UART_BAR		0x3f8
#define EARLY_TEMP_MMIO		0xfed08000

#endif
