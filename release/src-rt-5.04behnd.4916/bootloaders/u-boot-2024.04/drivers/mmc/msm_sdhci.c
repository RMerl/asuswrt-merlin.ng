// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm SDHCI driver - SD/eMMC controller
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * Based on Linux driver
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <wait_bit.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>

/* Non-standard registers needed for SDHCI startup */
#define SDCC_MCI_POWER   0x0
#define SDCC_MCI_POWER_SW_RST BIT(7)

/* This is undocumented register */
#define SDCC_MCI_VERSION		0x50
#define SDCC_V5_VERSION			0x318

#define SDCC_VERSION_MAJOR_SHIFT	28
#define SDCC_VERSION_MAJOR_MASK		(0xf << SDCC_VERSION_MAJOR_SHIFT)
#define SDCC_VERSION_MINOR_MASK		0xff

#define SDCC_MCI_STATUS2 0x6C
#define SDCC_MCI_STATUS2_MCI_ACT 0x1
#define SDCC_MCI_HC_MODE 0x78

/* Non standard (?) SDHCI register */
#define SDHCI_VENDOR_SPEC_CAPABILITIES0  0x11c

struct msm_sdhc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct msm_sdhc {
	struct sdhci_host host;
	void *base;
};

struct msm_sdhc_variant_info {
	bool mci_removed;
};

DECLARE_GLOBAL_DATA_PTR;

static int msm_sdc_clk_init(struct udevice *dev)
{
	int node = dev_of_offset(dev);
	uint clk_rate = fdtdec_get_uint(gd->fdt_blob, node, "clock-frequency",
					400000);
	uint clkd[2]; /* clk_id and clk_no */
	int clk_offset;
	struct udevice *clk_dev;
	struct clk clk;
	int ret;

	ret = fdtdec_get_int_array(gd->fdt_blob, node, "clock", clkd, 2);
	if (ret)
		return ret;

	clk_offset = fdt_node_offset_by_phandle(gd->fdt_blob, clkd[0]);
	if (clk_offset < 0)
		return clk_offset;

	ret = uclass_get_device_by_of_offset(UCLASS_CLK, clk_offset, &clk_dev);
	if (ret)
		return ret;

	clk.id = clkd[1];
	ret = clk_request(clk_dev, &clk);
	if (ret < 0)
		return ret;

	ret = clk_set_rate(&clk, clk_rate);
	if (ret < 0)
		return ret;

	return 0;
}

static int msm_sdc_mci_init(struct msm_sdhc *prv)
{
	/* Reset the core and Enable SDHC mode */
	writel(readl(prv->base + SDCC_MCI_POWER) | SDCC_MCI_POWER_SW_RST,
	       prv->base + SDCC_MCI_POWER);


	/* Wait for reset to be written to register */
	if (wait_for_bit_le32(prv->base + SDCC_MCI_STATUS2,
			      SDCC_MCI_STATUS2_MCI_ACT, false, 10, false)) {
		printf("msm_sdhci: reset request failed\n");
		return -EIO;
	}

	/* SW reset can take upto 10HCLK + 15MCLK cycles. (min 40us) */
	if (wait_for_bit_le32(prv->base + SDCC_MCI_POWER,
			      SDCC_MCI_POWER_SW_RST, false, 2, false)) {
		printf("msm_sdhci: stuck in reset\n");
		return -ETIMEDOUT;
	}

	/* Enable host-controller mode */
	writel(1, prv->base + SDCC_MCI_HC_MODE);

	return 0;
}

static int msm_sdc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct msm_sdhc_plat *plat = dev_get_plat(dev);
	struct msm_sdhc *prv = dev_get_priv(dev);
	const struct msm_sdhc_variant_info *var_info;
	struct sdhci_host *host = &prv->host;
	u32 core_version, core_minor, core_major;
	u32 caps;
	int ret;

	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD | SDHCI_QUIRK_BROKEN_R1B;

	host->max_clk = 0;

	/* Init clocks */
	ret = msm_sdc_clk_init(dev);
	if (ret)
		return ret;

	var_info = (void *)dev_get_driver_data(dev);
	if (!var_info->mci_removed) {
		ret = msm_sdc_mci_init(prv);
		if (ret)
			return ret;
	}

	if (!var_info->mci_removed)
		core_version = readl(prv->base + SDCC_MCI_VERSION);
	else
		core_version = readl(host->ioaddr + SDCC_V5_VERSION);

	core_major = (core_version & SDCC_VERSION_MAJOR_MASK);
	core_major >>= SDCC_VERSION_MAJOR_SHIFT;

	core_minor = core_version & SDCC_VERSION_MINOR_MASK;

	/*
	 * Support for some capabilities is not advertised by newer
	 * controller versions and must be explicitly enabled.
	 */
	if (core_major >= 1 && core_minor != 0x11 && core_minor != 0x12) {
		caps = readl(host->ioaddr + SDHCI_CAPABILITIES);
		caps |= SDHCI_CAN_VDD_300 | SDHCI_CAN_DO_8BIT;
		writel(caps, host->ioaddr + SDHCI_VENDOR_SPEC_CAPABILITIES0);
	}

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret)
		return ret;

	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		return ret;
	host->mmc->priv = &prv->host;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int msm_sdc_remove(struct udevice *dev)
{
	struct msm_sdhc *priv = dev_get_priv(dev);
	const struct msm_sdhc_variant_info *var_info;

	var_info = (void *)dev_get_driver_data(dev);

	/* Disable host-controller mode */
	if (!var_info->mci_removed)
		writel(0, priv->base + SDCC_MCI_HC_MODE);

	return 0;
}

static int msm_of_to_plat(struct udevice *dev)
{
	struct udevice *parent = dev->parent;
	struct msm_sdhc *priv = dev_get_priv(dev);
	struct sdhci_host *host = &priv->host;
	int node = dev_of_offset(dev);

	host->name = strdup(dev->name);
	host->ioaddr = dev_read_addr_ptr(dev);
	host->bus_width = fdtdec_get_int(gd->fdt_blob, node, "bus-width", 4);
	host->index = fdtdec_get_uint(gd->fdt_blob, node, "index", 0);
	priv->base = (void *)fdtdec_get_addr_size_auto_parent(gd->fdt_blob,
			dev_of_offset(parent), node, "reg", 1, NULL, false);
	if (priv->base == (void *)FDT_ADDR_T_NONE ||
	    host->ioaddr == (void *)FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static int msm_sdc_bind(struct udevice *dev)
{
	struct msm_sdhc_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct msm_sdhc_variant_info msm_sdhc_mci_var = {
	.mci_removed = false,
};

static const struct msm_sdhc_variant_info msm_sdhc_v5_var = {
	.mci_removed = true,
};

static const struct udevice_id msm_mmc_ids[] = {
	{ .compatible = "qcom,sdhci-msm-v4", .data = (ulong)&msm_sdhc_mci_var },
	{ .compatible = "qcom,sdhci-msm-v5", .data = (ulong)&msm_sdhc_v5_var },
	{ }
};

U_BOOT_DRIVER(msm_sdc_drv) = {
	.name		= "msm_sdc",
	.id		= UCLASS_MMC,
	.of_match	= msm_mmc_ids,
	.of_to_plat = msm_of_to_plat,
	.ops		= &sdhci_ops,
	.bind		= msm_sdc_bind,
	.probe		= msm_sdc_probe,
	.remove		= msm_sdc_remove,
	.priv_auto	= sizeof(struct msm_sdhc),
	.plat_auto	= sizeof(struct msm_sdhc_plat),
};
