/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Intel Corporation.
 */

#ifndef ASM_FAST_SPI_H
#define ASM_FAST_SPI_H

#include <pci.h>
#include <linux/bitops.h>

/* Register offsets from the MMIO region base (PCI_BASE_ADDRESS_0) */
struct fast_spi_regs {
	u32 bfp;
	u32 hsfsts_ctl;
	u32 faddr;
	u32 dlock;

	u32 fdata[0x10];

	u32 fracc;
	u32 freg[12];
	u32 fpr[5];
	u32 gpr0;
	u32 spare2;
	u32 sts_ctl;
	u16 preop;
	u16 optype;
	u8 opmenu[8];

	u32 spare3;
	u32 fdoc;
	u32 fdod;
	u32 spare4;
	u32 afc;
	u32 vscc[2];
	u32 ptinx;
	u32 ptdata;
};
check_member(fast_spi_regs, ptdata, 0xd0);

/* Bit definitions for BFPREG (0x00) register */
#define SPIBAR_BFPREG_PRB_MASK		0x7fff
#define SPIBAR_BFPREG_PRL_SHIFT		16
#define SPIBAR_BFPREG_PRL_MASK		(0x7fff << SPIBAR_BFPREG_PRL_SHIFT)

/* PCI configuration registers */
#define SPIBAR_BIOS_CONTROL			0xdc
#define SPIBAR_BIOS_CONTROL_WPD			BIT(0)
#define SPIBAR_BIOS_CONTROL_LOCK_ENABLE		BIT(1)
#define SPIBAR_BIOS_CONTROL_CACHE_DISABLE	BIT(2)
#define SPIBAR_BIOS_CONTROL_PREFETCH_ENABLE	BIT(3)
#define SPIBAR_BIOS_CONTROL_EISS		BIT(5)
#define SPIBAR_BIOS_CONTROL_BILD		BIT(7)

/**
 * fast_spi_get_bios_mmap() - Get memory map for SPI flash
 *
 * @pdev:	PCI device to use (this is the Fast SPI device)
 * @map_basep:	Returns base memory address for mapped SPI
 * @map_sizep:	Returns size of mapped SPI
 * @offsetp:	Returns start offset of SPI flash where the map works
 *	correctly (offsets before this are not visible)
 * Return: 0 (always)
 */
int fast_spi_get_bios_mmap(pci_dev_t pdev, ulong *map_basep, uint *map_sizep,
			   uint *offsetp);

/**
 * fast_spi_get_bios_mmap_regs() - Get memory map for SPI flash given regs
 *
 * @regs:	SPI registers to use
 * @map_basep:	Returns base memory address for mapped SPI
 * @map_sizep:	Returns size of mapped SPI
 * @offsetp:	Returns start offset of SPI flash where the map works
 *	correctly (offsets before this are not visible)
 * Return: 0 (always)
 */
int fast_spi_get_bios_mmap_regs(struct fast_spi_regs *regs, ulong *map_basep,
				uint *map_sizep, uint *offsetp);

/**
 * fast_spi_early_init() - Set up a BAR to use SPI early in U-Boot
 *
 * @pdev:	PCI device to use (this is the Fast SPI device)
 * @mmio_base:	MMIO base to use to access registers
 */
int fast_spi_early_init(pci_dev_t pdev, ulong mmio_base);

#endif	/* ASM_FAST_SPI_H */
