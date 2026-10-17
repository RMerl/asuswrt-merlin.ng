// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <asm/io.h>
#include <asm/cpu_common.h>
#include <asm/fast_spi.h>
#include <asm/pci.h>

/*
 * Returns bios_start and fills in size of the BIOS region.
 */
static ulong fast_spi_get_bios_region(struct fast_spi_regs *regs,
				      uint *bios_size)
{
	ulong bios_start, bios_end;

	/*
	 * BIOS_BFPREG provides info about BIOS-Flash Primary Region Base and
	 * Limit. Base and Limit fields are in units of 4K.
	 */
	u32 val = readl(&regs->bfp);

	bios_start = (val & SPIBAR_BFPREG_PRB_MASK) << 12;
	bios_end = (((val & SPIBAR_BFPREG_PRL_MASK) >>
		     SPIBAR_BFPREG_PRL_SHIFT) + 1) << 12;
	*bios_size = bios_end - bios_start;

	return bios_start;
}

int fast_spi_get_bios_mmap_regs(struct fast_spi_regs *regs, ulong *map_basep,
				uint *map_sizep, uint *offsetp)
{
	ulong base;

	base = fast_spi_get_bios_region(regs, map_sizep);
	*map_basep = (u32)-*map_sizep - base;
	*offsetp = base;

	return 0;
}

int fast_spi_get_bios_mmap(pci_dev_t pdev, ulong *map_basep, uint *map_sizep,
			   uint *offsetp)
{
	struct fast_spi_regs *regs;
	ulong bar, mmio_base;

	/* Special case to find mapping without probing the device */
	pci_x86_read_config(pdev, PCI_BASE_ADDRESS_0, &bar, PCI_SIZE_32);
	mmio_base = bar & PCI_BASE_ADDRESS_MEM_MASK;
	regs = (struct fast_spi_regs *)mmio_base;

	return fast_spi_get_bios_mmap_regs(regs, map_basep, map_sizep, offsetp);
}

int fast_spi_early_init(pci_dev_t pdev, ulong mmio_base)
{
	/* Program Temporary BAR for SPI */
	pci_x86_write_config(pdev, PCI_BASE_ADDRESS_0,
			     mmio_base | PCI_BASE_ADDRESS_SPACE_MEMORY,
			     PCI_SIZE_32);

	/* Enable Bus Master and MMIO Space */
	pci_x86_clrset_config(pdev, PCI_COMMAND, 0, PCI_COMMAND_MASTER |
			      PCI_COMMAND_MEMORY, PCI_SIZE_8);

	/*
	 * Disable the BIOS write protect so write commands are allowed.
	 * Enable Prefetching and caching.
	 */
	pci_x86_clrset_config(pdev, SPIBAR_BIOS_CONTROL,
			      SPIBAR_BIOS_CONTROL_EISS |
			      SPIBAR_BIOS_CONTROL_CACHE_DISABLE,
			      SPIBAR_BIOS_CONTROL_WPD |
			      SPIBAR_BIOS_CONTROL_PREFETCH_ENABLE, PCI_SIZE_8);

	return 0;
}
