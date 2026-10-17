// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 *
 * From coreboot broadwell support
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <pch.h>
#include <asm/global_data.h>
#include <asm/intel_regs.h>
#include <asm/io.h>
#include <asm/lpc_common.h>
#include <asm/arch/pch.h>
#include <asm/arch/spi.h>

static void set_spi_speed(void)
{
	u32 fdod;
	u8 ssfc;

	/* Observe SPI Descriptor Component Section 0 */
	writel(0x1000, SPI_REG(SPIBAR_FDOC));

	/* Extract the Write/Erase SPI Frequency from descriptor */
	fdod = readl(SPI_REG(SPIBAR_FDOD));
	fdod >>= 24;
	fdod &= 7;

	/* Set Software Sequence frequency to match */
	ssfc = readb(SPI_REG(SPIBAR_SSFC + 2));
	ssfc &= ~7;
	ssfc |= fdod;
	writeb(ssfc, SPI_REG(SPIBAR_SSFC + 2));
}

static int broadwell_lpc_early_init(struct udevice *dev)
{
	set_spi_speed();

	return 0;
}

static int lpc_init_extra(struct udevice *dev)
{
	return 0;
}

static int broadwell_lpc_probe(struct udevice *dev)
{
	int ret;

	if (!(gd->flags & GD_FLG_RELOC)) {
		ret = lpc_common_early_init(dev);
		if (ret) {
			debug("%s: lpc_early_init() failed\n", __func__);
			return ret;
		}

		return broadwell_lpc_early_init(dev);
	}

	return lpc_init_extra(dev);
}

static const struct udevice_id broadwell_lpc_ids[] = {
	{ .compatible = "intel,broadwell-lpc" },
	{ }
};

U_BOOT_DRIVER(broadwell_lpc_drv) = {
	.name		= "lpc",
	.id		= UCLASS_LPC,
	.of_match	= broadwell_lpc_ids,
	.probe		= broadwell_lpc_probe,
};
