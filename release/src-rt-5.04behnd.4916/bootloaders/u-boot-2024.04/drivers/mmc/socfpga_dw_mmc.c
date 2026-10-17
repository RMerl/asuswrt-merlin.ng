// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <log.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/secure_reg_helper.h>
#include <asm/arch/system_manager.h>
#include <clk.h>
#include <dm.h>
#include <dwmmc.h>
#include <errno.h>
#include <fdtdec.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <linux/intel-smc.h>
#include <linux/libfdt.h>
#include <linux/err.h>
#include <malloc.h>
#include <reset.h>

DECLARE_GLOBAL_DATA_PTR;

struct socfpga_dwmci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

/* socfpga implmentation specific driver private data */
struct dwmci_socfpga_priv_data {
	struct dwmci_host	host;
	unsigned int		drvsel;
	unsigned int		smplsel;
};

static void socfpga_dwmci_reset(struct udevice *dev)
{
	struct reset_ctl_bulk reset_bulk;
	int ret;

	ret = reset_get_bulk(dev, &reset_bulk);
	if (ret) {
		dev_warn(dev, "Can't get reset: %d\n", ret);
		return;
	}

	reset_deassert_bulk(&reset_bulk);
}

static int socfpga_dwmci_clksel(struct dwmci_host *host)
{
	struct dwmci_socfpga_priv_data *priv = host->priv;
	u32 sdmmc_mask = ((priv->smplsel & 0x7) << SYSMGR_SDMMC_SMPLSEL_SHIFT) |
			 ((priv->drvsel & 0x7) << SYSMGR_SDMMC_DRVSEL_SHIFT);

	/* Disable SDMMC clock. */
	clrbits_le32(socfpga_get_clkmgr_addr() + CLKMGR_PERPLL_EN,
		     CLKMGR_PERPLLGRP_EN_SDMMCCLK_MASK);

	debug("%s: drvsel %d smplsel %d\n", __func__,
	      priv->drvsel, priv->smplsel);

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_ATF)
	int ret;

	ret = socfpga_secure_reg_write32(SOCFPGA_SECURE_REG_SYSMGR_SOC64_SDMMC,
					 sdmmc_mask);
	if (ret) {
		printf("DWMMC: Failed to set clksel via SMC call");
		return ret;
	}
#else
	writel(sdmmc_mask, socfpga_get_sysmgr_addr() + SYSMGR_SDMMC);

	debug("%s: SYSMGR_SDMMCGRP_CTRL_REG = 0x%x\n", __func__,
		readl(socfpga_get_sysmgr_addr() + SYSMGR_SDMMC));
#endif

	/* Enable SDMMC clock */
	setbits_le32(socfpga_get_clkmgr_addr() + CLKMGR_PERPLL_EN,
		     CLKMGR_PERPLLGRP_EN_SDMMCCLK_MASK);

	return 0;
}

static int socfpga_dwmmc_get_clk_rate(struct udevice *dev)
{
	struct dwmci_socfpga_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
#if CONFIG_IS_ENABLED(CLK)
	struct clk clk;
	int ret;

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret)
		return ret;

	host->bus_hz = clk_get_rate(&clk);

#else
	/* Fixed clock divide by 4 which due to the SDMMC wrapper */
	host->bus_hz = cm_get_mmc_controller_clk_hz();
#endif
	if (host->bus_hz == 0) {
		printf("DWMMC: MMC clock is zero!");
		return -EINVAL;
	}

	return 0;
}

static int socfpga_dwmmc_of_to_plat(struct udevice *dev)
{
	struct dwmci_socfpga_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	int fifo_depth;

	fifo_depth = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
				    "fifo-depth", 0);
	if (fifo_depth < 0) {
		printf("DWMMC: Can't get FIFO depth\n");
		return -EINVAL;
	}

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);
	host->buswidth = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					"bus-width", 4);
	host->clksel = socfpga_dwmci_clksel;

	/*
	 * TODO(sjg@chromium.org): Remove the need for this hack.
	 * We only have one dwmmc block on gen5 SoCFPGA.
	 */
	host->dev_index = 0;
	host->fifoth_val = MSIZE(0x2) |
		RX_WMARK(fifo_depth / 2 - 1) | TX_WMARK(fifo_depth / 2);
	priv->drvsel = fdtdec_get_uint(gd->fdt_blob, dev_of_offset(dev),
				       "drvsel", 3);
	priv->smplsel = fdtdec_get_uint(gd->fdt_blob, dev_of_offset(dev),
					"smplsel", 0);
	host->priv = priv;

	host->fifo_mode = dev_read_bool(dev, "fifo-mode");

	return 0;
}

static int socfpga_dwmmc_probe(struct udevice *dev)
{
#ifdef CONFIG_BLK
	struct socfpga_dwmci_plat *plat = dev_get_plat(dev);
#endif
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct dwmci_socfpga_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	int ret;

	ret = socfpga_dwmmc_get_clk_rate(dev);
	if (ret)
		return ret;

	socfpga_dwmci_reset(dev);

#ifdef CONFIG_BLK
	dwmci_setup_cfg(&plat->cfg, host, host->bus_hz, 400000);
	host->mmc = &plat->mmc;
#else

	ret = add_dwmci(host, host->bus_hz, 400000);
	if (ret)
		return ret;
#endif
	host->mmc->priv = &priv->host;
	upriv->mmc = host->mmc;
	host->mmc->dev = dev;

	return dwmci_probe(dev);
}

static int socfpga_dwmmc_bind(struct udevice *dev)
{
#ifdef CONFIG_BLK
	struct socfpga_dwmci_plat *plat = dev_get_plat(dev);
	int ret;

	ret = dwmci_bind(dev, &plat->mmc, &plat->cfg);
	if (ret)
		return ret;
#endif

	return 0;
}

static const struct udevice_id socfpga_dwmmc_ids[] = {
	{ .compatible = "altr,socfpga-dw-mshc" },
	{ }
};

U_BOOT_DRIVER(socfpga_dwmmc_drv) = {
	.name		= "socfpga_dwmmc",
	.id		= UCLASS_MMC,
	.of_match	= socfpga_dwmmc_ids,
	.of_to_plat = socfpga_dwmmc_of_to_plat,
	.ops		= &dm_dwmci_ops,
	.bind		= socfpga_dwmmc_bind,
	.probe		= socfpga_dwmmc_probe,
	.priv_auto	= sizeof(struct dwmci_socfpga_priv_data),
	.plat_auto	= sizeof(struct socfpga_dwmci_plat),
};
