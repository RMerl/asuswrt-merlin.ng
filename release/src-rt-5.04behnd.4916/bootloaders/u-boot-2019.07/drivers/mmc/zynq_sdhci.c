// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 - 2015 Xilinx, Inc.
 *
 * Xilinx Zynq SD Host Controller Interface
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include "mmc_private.h"
#include <linux/libfdt.h>
#include <malloc.h>
#include <sdhci.h>
#include <zynqmp_tap_delay.h>

DECLARE_GLOBAL_DATA_PTR;

struct arasan_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	unsigned int f_max;
};

struct arasan_sdhci_priv {
	struct sdhci_host *host;
	u8 deviceid;
	u8 bank;
	u8 no_1p8;
};

#if defined(CONFIG_ARCH_ZYNQMP)
#define MMC_HS200_BUS_SPEED	5

static const u8 mode2timing[] = {
	[MMC_LEGACY] = UHS_SDR12_BUS_SPEED,
	[SD_LEGACY] = UHS_SDR12_BUS_SPEED,
	[MMC_HS] = HIGH_SPEED_BUS_SPEED,
	[SD_HS] = HIGH_SPEED_BUS_SPEED,
	[MMC_HS_52] = HIGH_SPEED_BUS_SPEED,
	[MMC_DDR_52] = HIGH_SPEED_BUS_SPEED,
	[UHS_SDR12] = UHS_SDR12_BUS_SPEED,
	[UHS_SDR25] = UHS_SDR25_BUS_SPEED,
	[UHS_SDR50] = UHS_SDR50_BUS_SPEED,
	[UHS_DDR50] = UHS_DDR50_BUS_SPEED,
	[UHS_SDR104] = UHS_SDR104_BUS_SPEED,
	[MMC_HS_200] = MMC_HS200_BUS_SPEED,
};

#define SDHCI_HOST_CTRL2	0x3E
#define SDHCI_CTRL2_MODE_MASK	0x7
#define SDHCI_18V_SIGNAL	0x8
#define SDHCI_CTRL_EXEC_TUNING	0x0040
#define SDHCI_CTRL_TUNED_CLK	0x80
#define SDHCI_TUNING_LOOP_COUNT	40

static void arasan_zynqmp_dll_reset(struct sdhci_host *host, u8 deviceid)
{
	u16 clk;
	unsigned long timeout;

	clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	clk &= ~(SDHCI_CLOCK_CARD_EN);
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);

	/* Issue DLL Reset */
	zynqmp_dll_reset(deviceid);

	/* Wait max 20 ms */
	timeout = 100;
	while (!((clk = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
				& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			dev_err(mmc_dev(host->mmc),
				": Internal clock never stabilised.\n");
			return;
		}
		timeout--;
		udelay(1000);
	}

	clk |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
}

static int arasan_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	u32 ctrl;
	struct sdhci_host *host;
	struct arasan_sdhci_priv *priv = dev_get_priv(mmc->dev);
	char tuning_loop_counter = SDHCI_TUNING_LOOP_COUNT;
	u8 deviceid;

	debug("%s\n", __func__);

	host = priv->host;
	deviceid = priv->deviceid;

	ctrl = sdhci_readw(host, SDHCI_HOST_CTRL2);
	ctrl |= SDHCI_CTRL_EXEC_TUNING;
	sdhci_writew(host, ctrl, SDHCI_HOST_CTRL2);

	mdelay(1);

	arasan_zynqmp_dll_reset(host, deviceid);

	sdhci_writel(host, SDHCI_INT_DATA_AVAIL, SDHCI_INT_ENABLE);
	sdhci_writel(host, SDHCI_INT_DATA_AVAIL, SDHCI_SIGNAL_ENABLE);

	do {
		cmd.cmdidx = opcode;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 0;

		data.blocksize = 64;
		data.blocks = 1;
		data.flags = MMC_DATA_READ;

		if (tuning_loop_counter-- == 0)
			break;

		if (cmd.cmdidx == MMC_CMD_SEND_TUNING_BLOCK_HS200 &&
		    mmc->bus_width == 8)
			data.blocksize = 128;

		sdhci_writew(host, SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG,
						    data.blocksize),
			     SDHCI_BLOCK_SIZE);
		sdhci_writew(host, data.blocks, SDHCI_BLOCK_COUNT);
		sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

		mmc_send_cmd(mmc, &cmd, NULL);
		ctrl = sdhci_readw(host, SDHCI_HOST_CTRL2);

		if (cmd.cmdidx == MMC_CMD_SEND_TUNING_BLOCK)
			udelay(1);

	} while (ctrl & SDHCI_CTRL_EXEC_TUNING);

	if (tuning_loop_counter < 0) {
		ctrl &= ~SDHCI_CTRL_TUNED_CLK;
		sdhci_writel(host, ctrl, SDHCI_HOST_CTRL2);
	}

	if (!(ctrl & SDHCI_CTRL_TUNED_CLK)) {
		printf("%s:Tuning failed\n", __func__);
		return -1;
	}

	udelay(1);
	arasan_zynqmp_dll_reset(host, deviceid);

	/* Enable only interrupts served by the SD controller */
	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK,
		     SDHCI_INT_ENABLE);
	/* Mask all sdhci interrupt sources */
	sdhci_writel(host, 0x0, SDHCI_SIGNAL_ENABLE);

	return 0;
}

static void arasan_sdhci_set_tapdelay(struct sdhci_host *host)
{
	struct arasan_sdhci_priv *priv = dev_get_priv(host->mmc->dev);
	struct mmc *mmc = (struct mmc *)host->mmc;
	u8 uhsmode;

	uhsmode = mode2timing[mmc->selected_mode];

	if (uhsmode >= UHS_SDR25_BUS_SPEED)
		arasan_zynqmp_set_tapdelay(priv->deviceid, uhsmode,
					   priv->bank);
}

static void arasan_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct mmc *mmc = (struct mmc *)host->mmc;
	u32 reg;

	if (!IS_SD(mmc))
		return;

	if (mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180) {
		reg = sdhci_readw(host, SDHCI_HOST_CTRL2);
		reg |= SDHCI_18V_SIGNAL;
		sdhci_writew(host, reg, SDHCI_HOST_CTRL2);
	}

	if (mmc->selected_mode > SD_HS &&
	    mmc->selected_mode <= UHS_DDR50) {
		reg = sdhci_readw(host, SDHCI_HOST_CTRL2);
		reg &= ~SDHCI_CTRL2_MODE_MASK;
		switch (mmc->selected_mode) {
		case UHS_SDR12:
			reg |= UHS_SDR12_BUS_SPEED;
			break;
		case UHS_SDR25:
			reg |= UHS_SDR25_BUS_SPEED;
			break;
		case UHS_SDR50:
			reg |= UHS_SDR50_BUS_SPEED;
			break;
		case UHS_SDR104:
			reg |= UHS_SDR104_BUS_SPEED;
			break;
		case UHS_DDR50:
			reg |= UHS_DDR50_BUS_SPEED;
			break;
		default:
			break;
		}
		sdhci_writew(host, reg, SDHCI_HOST_CTRL2);
	}
}
#endif

#if defined(CONFIG_DM_MMC) && defined(CONFIG_ARCH_ZYNQMP)
const struct sdhci_ops arasan_ops = {
	.platform_execute_tuning	= &arasan_sdhci_execute_tuning,
	.set_delay = &arasan_sdhci_set_tapdelay,
	.set_control_reg = &arasan_sdhci_set_control_reg,
};
#endif

static int arasan_sdhci_probe(struct udevice *dev)
{
	struct arasan_sdhci_plat *plat = dev_get_platdata(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct arasan_sdhci_priv *priv = dev_get_priv(dev);
	struct sdhci_host *host;
	struct clk clk;
	unsigned long clock;
	int ret;

	host = priv->host;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(dev, "failed to get rate\n");
		return clock;
	}

	debug("%s: CLK %ld\n", __func__, clock);

	ret = clk_enable(&clk);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD |
		       SDHCI_QUIRK_BROKEN_R1B;

#ifdef CONFIG_ZYNQ_HISPD_BROKEN
	host->quirks |= SDHCI_QUIRK_BROKEN_HISPD_MODE;
#endif

	if (priv->no_1p8)
		host->quirks |= SDHCI_QUIRK_NO_1_8_V;

	host->max_clk = clock;

	ret = sdhci_setup_cfg(&plat->cfg, host, plat->f_max,
			      CONFIG_ZYNQ_SDHCI_MIN_FREQ);
	host->mmc = &plat->mmc;
	if (ret)
		return ret;
	host->mmc->priv = host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	return sdhci_probe(dev);
}

static int arasan_sdhci_ofdata_to_platdata(struct udevice *dev)
{
	struct arasan_sdhci_plat *plat = dev_get_platdata(dev);
	struct arasan_sdhci_priv *priv = dev_get_priv(dev);

	priv->host = calloc(1, sizeof(struct sdhci_host));
	if (!priv->host)
		return -1;

	priv->host->name = dev->name;

#if defined(CONFIG_DM_MMC) && defined(CONFIG_ARCH_ZYNQMP)
	priv->host->ops = &arasan_ops;
#endif

	priv->host->ioaddr = (void *)dev_read_addr(dev);
	if (IS_ERR(priv->host->ioaddr))
		return PTR_ERR(priv->host->ioaddr);

	priv->deviceid = dev_read_u32_default(dev, "xlnx,device_id", -1);
	priv->bank = dev_read_u32_default(dev, "xlnx,mio_bank", -1);
	priv->no_1p8 = dev_read_bool(dev, "no-1-8-v");

	plat->f_max = dev_read_u32_default(dev, "max-frequency",
					   CONFIG_ZYNQ_SDHCI_MAX_FREQ);
	return 0;
}

static int arasan_sdhci_bind(struct udevice *dev)
{
	struct arasan_sdhci_plat *plat = dev_get_platdata(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id arasan_sdhci_ids[] = {
	{ .compatible = "arasan,sdhci-8.9a" },
	{ }
};

U_BOOT_DRIVER(arasan_sdhci_drv) = {
	.name		= "arasan_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= arasan_sdhci_ids,
	.ofdata_to_platdata = arasan_sdhci_ofdata_to_platdata,
	.ops		= &sdhci_ops,
	.bind		= arasan_sdhci_bind,
	.probe		= arasan_sdhci_probe,
	.priv_auto_alloc_size = sizeof(struct arasan_sdhci_priv),
	.platdata_auto_alloc_size = sizeof(struct arasan_sdhci_plat),
};
