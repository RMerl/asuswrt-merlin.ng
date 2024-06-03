/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Google Inc.
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/soc/intel/baytrail/include/soc/pci_devs.h
 */

#ifndef _DEVICE_H_
#define _DEVICE_H_

/*
 * Internal PCI device numbers within the SoC.
 *
 * Note it must start with 0x_ prefix, as the device number macro will be
 * included in the ACPI ASL files (see irq_helper.h and irq_route.h).
 */

/* SoC transaction router */
#define SOC_DEV		0x00

/* Graphics and Display */
#define GFX_DEV		0x02

/* MIPI */
#define MIPI_DEV	0x03

/* EMMC Port */
#define EMMC_DEV	0x10

/* SDIO Port */
#define SDIO_DEV	0x11

/* SD Port */
#define SD_DEV		0x12

/* SATA */
#define SATA_DEV	0x13

/* xHCI */
#define XHCI_DEV	0x14

/* LPE Audio */
#define LPE_DEV		0x15

/* OTG */
#define OTG_DEV		0x16

/* MMC45 Port */
#define MMC45_DEV	0x17

/* Serial IO 1 */
#define SIO1_DEV	0x18

/* Trusted Execution Engine */
#define TXE_DEV		0x1a

/* HD Audio */
#define HDA_DEV		0x1b

/* PCIe Ports */
#define PCIE_DEV	0x1c

/* EHCI */
#define EHCI_DEV	0x1d

/* Serial IO 2 */
#define SIO2_DEV	0x1e

/* Platform Controller Unit */
#define PCU_DEV		0x1f

#endif /* _DEVICE_H_ */
