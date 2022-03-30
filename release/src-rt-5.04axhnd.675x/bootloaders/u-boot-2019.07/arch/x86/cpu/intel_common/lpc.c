// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <pch.h>
#include <pci.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/lpc_common.h>

DECLARE_GLOBAL_DATA_PTR;

/* Enable Prefetching and Caching */
static void enable_spi_prefetch(struct udevice *pch)
{
	u8 reg8;

	dm_pci_read_config8(pch, 0xdc, &reg8);
	reg8 &= ~(3 << 2);
	reg8 |= (2 << 2); /* Prefetching and Caching Enabled */
	dm_pci_write_config8(pch, 0xdc, reg8);
}

static void enable_port80_on_lpc(struct udevice *pch)
{
	/* Enable port 80 POST on LPC */
	dm_pci_write_config32(pch, PCH_RCBA_BASE, RCB_BASE_ADDRESS | 1);
	clrbits_le32(RCB_REG(GCS), 4);
}

/**
 * lpc_early_init() - set up LPC serial ports and other early things
 *
 * @dev:	LPC device
 * @return 0 if OK, -ve on error
 */
int lpc_common_early_init(struct udevice *dev)
{
	struct udevice *pch = dev->parent;
	struct reg_info {
		u32 base;
		u32 size;
	} values[4], *ptr;
	int count;
	int i;

	count = fdtdec_get_int_array_count(gd->fdt_blob, dev_of_offset(dev),
			"intel,gen-dec", (u32 *)values,
			sizeof(values) / sizeof(u32));
	if (count < 0)
		return -EINVAL;

	/* Set COM1/COM2 decode range */
	dm_pci_write_config16(pch, LPC_IO_DEC, 0x0010);

	/* Enable PS/2 Keyboard/Mouse, EC areas and COM1 */
	dm_pci_write_config16(pch, LPC_EN, KBC_LPC_EN | MC_LPC_EN |
			      GAMEL_LPC_EN | COMA_LPC_EN);

	/* Write all registers but use 0 if we run out of data */
	count = count * sizeof(u32) / sizeof(values[0]);
	for (i = 0, ptr = values; i < ARRAY_SIZE(values); i++, ptr++) {
		u32 reg = 0;

		if (i < count)
			reg = ptr->base | PCI_COMMAND_IO | (ptr->size << 16);
		dm_pci_write_config32(pch, LPC_GENX_DEC(i), reg);
	}

	enable_spi_prefetch(pch);

	/* This is already done in start.S, but let's do it in C */
	enable_port80_on_lpc(pch);

	return 0;
}

int lpc_set_spi_protect(struct udevice *dev, int bios_ctrl, bool protect)
{
	uint8_t bios_cntl;

	/* Adjust the BIOS write protect and SMM BIOS Write Protect Disable */
	dm_pci_read_config8(dev, bios_ctrl, &bios_cntl);
	if (protect) {
		bios_cntl &= ~BIOS_CTRL_BIOSWE;
		bios_cntl |= BIT(5);
	} else {
		bios_cntl |= BIOS_CTRL_BIOSWE;
		bios_cntl &= ~BIT(5);
	}
	dm_pci_write_config8(dev, bios_ctrl, bios_cntl);

	return 0;
}
