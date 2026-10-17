// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Cortina Access
 * Arthur Li <arthur.li@cortina-access.com>
 */

#include <common.h>
#include <dwmmc.h>
#include <fdtdec.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <errno.h>
#include <dm.h>
#include <mapmem.h>

#define SD_CLK_SEL_MASK (0x3)
#define SD_DLL_DEFAULT  (0x143000)
#define SD_SCLK_MAX (200000000)

#define SD_CLK_SEL_200MHZ (0x2)
#define SD_CLK_SEL_100MHZ (0x1)
#define SD_CLK_SEL_50MHZ (0x0)

#define IO_DRV_SD_DS_OFFSET (16)
#define IO_DRV_SD_DS_MASK   (0xff << IO_DRV_SD_DS_OFFSET)

#define MIN_FREQ (400000)

DECLARE_GLOBAL_DATA_PTR;

struct ca_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct ca_dwmmc_priv_data {
	struct dwmci_host host;
	void __iomem *sd_dll_reg;
	void __iomem *io_drv_reg;
	u8 ds;
};

static int ca_dwmci_clksel(struct dwmci_host *host)
{
	struct ca_dwmmc_priv_data *priv = host->priv;
	u32 val = readl(priv->sd_dll_reg);

	val &= ~SD_CLK_SEL_MASK;
	if (host->bus_hz >= 200000000)
		val |= SD_CLK_SEL_200MHZ;
	else if (host->bus_hz >= 100000000)
		val |= SD_CLK_SEL_100MHZ;

	writel(val, priv->sd_dll_reg);

	return 0;
}

static void ca_dwmci_board_init(struct dwmci_host *host)
{
	struct ca_dwmmc_priv_data *priv = host->priv;
	u32 val = readl(priv->io_drv_reg);

	writel(SD_DLL_DEFAULT, priv->sd_dll_reg);

	val &= ~IO_DRV_SD_DS_MASK;
	if (priv && priv->ds)
		val |= priv->ds << IO_DRV_SD_DS_OFFSET;
	writel(val, priv->io_drv_reg);
}

unsigned int ca_dwmci_get_mmc_clock(struct dwmci_host *host, uint freq)
{
	struct ca_dwmmc_priv_data *priv = host->priv;
	u8 sd_clk_sel = readl(priv->sd_dll_reg) & SD_CLK_SEL_MASK;
	u8 clk_div;

	switch (sd_clk_sel) {
	case SD_CLK_SEL_50MHZ:
		clk_div = 4;
		break;
	case SD_CLK_SEL_100MHZ:
		clk_div = 2;
		break;
	default:
		clk_div = 1;
	}

	return SD_SCLK_MAX / clk_div / (host->div + 1);
}

static int ca_dwmmc_of_to_plat(struct udevice *dev)
{
	struct ca_dwmmc_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;
	u32 tmp;

	host->name = dev->name;
	host->dev_index = 0;

	host->buswidth = dev_read_u32_default(dev, "bus-width", 1);
	host->bus_hz = dev_read_u32_default(dev, "max-frequency", 50000000);
	priv->ds = dev_read_u32_default(dev, "io_ds", 0x33);
	host->fifo_mode = dev_read_bool(dev, "fifo-mode");

	dev_read_u32(dev, "sd_dll_ctrl", &tmp);
	priv->sd_dll_reg = map_sysmem((uintptr_t)tmp, sizeof(uintptr_t));
	if (!priv->sd_dll_reg)
		return -EINVAL;

	dev_read_u32(dev, "io_drv_ctrl", &tmp);
	priv->io_drv_reg = map_sysmem((uintptr_t)tmp, sizeof(uintptr_t));
	if (!priv->io_drv_reg)
		return -EINVAL;

	host->ioaddr = dev_read_addr_ptr(dev);
	if (!host->ioaddr)
		return -EINVAL;

	host->priv = priv;

	return 0;
}

struct dm_mmc_ops ca_dwmci_dm_ops;

static int ca_dwmmc_probe(struct udevice *dev)
{
	struct ca_mmc_plat *plat = dev_get_plat(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct ca_dwmmc_priv_data *priv = dev_get_priv(dev);
	struct dwmci_host *host = &priv->host;

	memcpy(&ca_dwmci_dm_ops, &dm_dwmci_ops, sizeof(struct dm_mmc_ops));

	dwmci_setup_cfg(&plat->cfg, host, host->bus_hz, MIN_FREQ);
	if (host->buswidth == 1)
		(&plat->cfg)->host_caps &= ~(MMC_MODE_8BIT | MMC_MODE_4BIT);

	host->mmc = &plat->mmc;
	host->mmc->priv = &priv->host;
	upriv->mmc = host->mmc;
	host->mmc->dev = dev;
	host->clksel = ca_dwmci_clksel;
	host->board_init = ca_dwmci_board_init;
	host->get_mmc_clk = ca_dwmci_get_mmc_clock;

	return dwmci_probe(dev);
}

static int ca_dwmmc_bind(struct udevice *dev)
{
	struct ca_mmc_plat *plat = dev_get_plat(dev);

	return dwmci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id ca_dwmmc_ids[] = {
	{ .compatible = "cortina,ca-mmc" },
	{ }
};

U_BOOT_DRIVER(ca_dwmmc_drv) = {
	.name		= "cortina_dwmmc",
	.id		= UCLASS_MMC,
	.of_match	= ca_dwmmc_ids,
	.of_to_plat = ca_dwmmc_of_to_plat,
	.bind		= ca_dwmmc_bind,
	.ops		= &ca_dwmci_dm_ops,
	.probe		= ca_dwmmc_probe,
	.priv_auto	= sizeof(struct ca_dwmmc_priv_data),
	.plat_auto	= sizeof(struct ca_mmc_plat),
};
