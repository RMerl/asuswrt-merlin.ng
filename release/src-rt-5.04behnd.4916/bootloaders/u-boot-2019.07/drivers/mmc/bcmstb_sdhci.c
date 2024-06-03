// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018  Cisco Systems, Inc.
 * (C) Copyright 2019  Synamedia
 *
 * Author: Thomas Fitzsimmons <fitzsim@fitzsim.org>
 */

#include <common.h>
#include <dm.h>
#include <mach/sdhci.h>
#include <malloc.h>
#include <sdhci.h>

/*
 * The BCMSTB SDHCI has a quirk in that its actual maximum frequency
 * capability is 100 MHz.  The divisor that is eventually written to
 * SDHCI_CLOCK_CONTROL is calculated based on what the MMC device
 * reports, and relative to this maximum frequency.
 *
 * This define used to be set to 52000000 (52 MHz), the desired
 * maximum frequency, but that would result in the communication
 * actually running at 100 MHz (seemingly without issue), which is
 * out-of-spec.
 *
 * Now, by setting this to 0 (auto-detect), 100 MHz will be read from
 * the capabilities register, and the resulting divisor will be
 * doubled, meaning that the clock control register will be set to the
 * in-spec 52 MHz value.
 */
#define BCMSTB_SDHCI_MAXIMUM_CLOCK_FREQUENCY	0
/*
 * When the minimum clock frequency is set to 0 (auto-detect), U-Boot
 * sets it to 100 MHz divided by SDHCI_MAX_DIV_SPEC_300, or 48,875 Hz,
 * which results in the controller timing out when trying to
 * communicate with the MMC device.  Hard-code this value to 400000
 * (400 kHz) to prevent this.
 */
#define BCMSTB_SDHCI_MINIMUM_CLOCK_FREQUENCY	400000

/*
 * This driver has only been tested with eMMC devices; SD devices may
 * not work.
 */
struct sdhci_bcmstb_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static int sdhci_bcmstb_bind(struct udevice *dev)
{
	struct sdhci_bcmstb_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static int sdhci_bcmstb_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sdhci_bcmstb_plat *plat = dev_get_platdata(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	fdt_addr_t base;
	int ret;

	base = devfdt_get_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	host->name = dev->name;
	host->ioaddr = (void *)base;

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	ret = sdhci_setup_cfg(&plat->cfg, host,
			      BCMSTB_SDHCI_MAXIMUM_CLOCK_FREQUENCY,
			      BCMSTB_SDHCI_MINIMUM_CLOCK_FREQUENCY);
	if (ret)
		return ret;

	upriv->mmc = &plat->mmc;
	host->mmc = &plat->mmc;
	host->mmc->priv = host;

	return sdhci_probe(dev);
}

static const struct udevice_id sdhci_bcmstb_match[] = {
	{ .compatible = "brcm,bcm7425-sdhci" },
	{ .compatible = "brcm,sdhci-brcmstb" },
	{ }
};

U_BOOT_DRIVER(sdhci_bcmstb) = {
	.name = "sdhci-bcmstb",
	.id = UCLASS_MMC,
	.of_match = sdhci_bcmstb_match,
	.ops = &sdhci_ops,
	.bind = sdhci_bcmstb_bind,
	.probe = sdhci_bcmstb_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct sdhci_bcmstb_plat),
};
