// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <dm.h>
#include <mmc.h>
#include <reset-uclass.h>
#include <sdhci.h>
#include <asm/arch/sdhci.h>

DECLARE_GLOBAL_DATA_PTR;

struct sti_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	struct reset_ctl reset;
	int instance;
};

/**
 * sti_mmc_core_config: configure the Arasan HC
 * @dev : udevice
 *
 * Description: this function is to configure the Arasan MMC HC.
 * This should be called when the system starts in case of, on the SoC,
 * it is needed to configure the host controller.
 * This happens on some SoCs, i.e. StiH410, where the MMC0 inside the flashSS
 * needs to be configured as MMC 4.5 to have full capabilities.
 * W/o these settings the SDHCI could configure and use the embedded controller
 * with limited features.
 */
static int sti_mmc_core_config(struct udevice *dev)
{
	struct sti_sdhci_plat *plat = dev_get_platdata(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;

	/* only MMC1 has a reset line */
	if (plat->instance) {
		ret = reset_deassert(&plat->reset);
		if (ret < 0) {
			pr_err("MMC1 deassert failed: %d", ret);
			return ret;
		}
	}

	writel(STI_FLASHSS_MMC_CORE_CONFIG_1,
	       host->ioaddr + FLASHSS_MMC_CORE_CONFIG_1);

	if (plat->instance) {
		writel(STI_FLASHSS_MMC_CORE_CONFIG2,
		       host->ioaddr + FLASHSS_MMC_CORE_CONFIG_2);
		writel(STI_FLASHSS_MMC_CORE_CONFIG3,
		       host->ioaddr + FLASHSS_MMC_CORE_CONFIG_3);
	} else {
		writel(STI_FLASHSS_SDCARD_CORE_CONFIG2,
		       host->ioaddr + FLASHSS_MMC_CORE_CONFIG_2);
		writel(STI_FLASHSS_SDCARD_CORE_CONFIG3,
		       host->ioaddr + FLASHSS_MMC_CORE_CONFIG_3);
	}
	writel(STI_FLASHSS_MMC_CORE_CONFIG4,
	       host->ioaddr + FLASHSS_MMC_CORE_CONFIG_4);

	return 0;
}

static int sti_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sti_sdhci_plat *plat = dev_get_platdata(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	int ret;

	/*
	 * identify current mmc instance, mmc1 has a reset, not mmc0
	 * MMC0 is wired to the SD slot,
	 * MMC1 is wired on the high speed connector
	 */
	ret = reset_get_by_index(dev, 0, &plat->reset);
	if (!ret)
		plat->instance = 1;
	else
		if (ret == -ENOENT)
			plat->instance = 0;
		else
			return ret;

	ret = sti_mmc_core_config(dev);
	if (ret)
		return ret;

	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD |
		       SDHCI_QUIRK_32BIT_DMA_ADDR |
		       SDHCI_QUIRK_NO_HISPD_BIT;

	host->host_caps = MMC_MODE_DDR_52MHz;

	ret = sdhci_setup_cfg(&plat->cfg, host, 50000000, 400000);
	if (ret)
		return ret;

	host->mmc = &plat->mmc;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int sti_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct sdhci_host *host = dev_get_priv(dev);

	host->name = strdup(dev->name);
	host->ioaddr = (void *)devfdt_get_addr(dev);

	host->bus_width = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					 "bus-width", 4);

	return 0;
}

static int sti_sdhci_bind(struct udevice *dev)
{
	struct sti_sdhci_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id sti_sdhci_ids[] = {
	{ .compatible = "st,sdhci" },
	{ }
};

U_BOOT_DRIVER(sti_mmc) = {
	.name = "sti_sdhci",
	.id = UCLASS_MMC,
	.of_match = sti_sdhci_ids,
	.bind = sti_sdhci_bind,
	.ops = &sdhci_ops,
	.ofdata_to_platdata = sti_sdhci_ofdata_to_platdata,
	.probe = sti_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct sdhci_host),
	.platdata_auto_alloc_size = sizeof(struct sti_sdhci_plat),
};
