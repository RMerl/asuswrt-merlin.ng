// SPDX-License-Identifier: GPL-2.0+
/*
 * Support of SDHCI for Microchip PIC32 SoC.
 *
 * Copyright (C) 2015 Microchip Technology Inc.
 * Andrei Pistirica <andrei.pistirica@microchip.com>
 */

#include <common.h>
#include <dm.h>
#include <sdhci.h>
#include <linux/errno.h>
#include <mach/pic32.h>

DECLARE_GLOBAL_DATA_PTR;

static int pic32_sdhci_get_cd(struct sdhci_host *host)
{
	/* PIC32 SDHCI CD errata:
	 * - set CD_TEST and clear CD_TEST_INS bit
	 */
	sdhci_writeb(host, SDHCI_CTRL_CD_TEST, SDHCI_HOST_CONTROL);

	return 0;
}

static const struct sdhci_ops pic32_sdhci_ops = {
	.get_cd	= pic32_sdhci_get_cd,
};

static int pic32_sdhci_probe(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;
	u32 f_min_max[2];
	fdt_addr_t addr;
	fdt_size_t size;
	int ret;

	addr = fdtdec_get_addr_size(fdt, dev_of_offset(dev), "reg", &size);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	host->ioaddr	= ioremap(addr, size);
	host->name	= dev->name;
	host->quirks	= SDHCI_QUIRK_NO_HISPD_BIT;
	host->bus_width	= fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"bus-width", 4);
	host->ops = &pic32_sdhci_ops;

	ret = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(dev),
				   "clock-freq-min-max", f_min_max, 2);
	if (ret) {
		printf("sdhci: clock-freq-min-max not found\n");
		return ret;
	}

	host->max_clk   = f_min_max[1];

	ret = add_sdhci(host, 0, f_min_max[0]);
	if (ret)
		return ret;
	host->mmc->dev = dev;

	return 0;
}

static const struct udevice_id pic32_sdhci_ids[] = {
	{ .compatible = "microchip,pic32mzda-sdhci" },
	{ }
};

U_BOOT_DRIVER(pic32_sdhci_drv) = {
	.name			= "pic32_sdhci",
	.id			= UCLASS_MMC,
	.of_match		= pic32_sdhci_ids,
	.probe			= pic32_sdhci_probe,
	.priv_auto_alloc_size	= sizeof(struct sdhci_host),
};
