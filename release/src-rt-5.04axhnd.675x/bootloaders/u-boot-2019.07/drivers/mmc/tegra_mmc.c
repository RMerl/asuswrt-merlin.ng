// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Jaehoon Chung <jh80.chung@samsung.com>
 * Portions Copyright 2011-2016 NVIDIA Corporation
 */

#include <bouncebuf.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <mmc.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch-tegra/tegra_mmc.h>

struct tegra_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct tegra_mmc_priv {
	struct tegra_mmc *reg;
	struct reset_ctl reset_ctl;
	struct clk clk;
	struct gpio_desc cd_gpio;	/* Change Detect GPIO */
	struct gpio_desc pwr_gpio;	/* Power GPIO */
	struct gpio_desc wp_gpio;	/* Write Protect GPIO */
	unsigned int version;	/* SDHCI spec. version */
	unsigned int clock;	/* Current clock (MHz) */
};

static void tegra_mmc_set_power(struct tegra_mmc_priv *priv,
				unsigned short power)
{
	u8 pwr = 0;
	debug("%s: power = %x\n", __func__, power);

	if (power != (unsigned short)-1) {
		switch (1 << power) {
		case MMC_VDD_165_195:
			pwr = TEGRA_MMC_PWRCTL_SD_BUS_VOLTAGE_V1_8;
			break;
		case MMC_VDD_29_30:
		case MMC_VDD_30_31:
			pwr = TEGRA_MMC_PWRCTL_SD_BUS_VOLTAGE_V3_0;
			break;
		case MMC_VDD_32_33:
		case MMC_VDD_33_34:
			pwr = TEGRA_MMC_PWRCTL_SD_BUS_VOLTAGE_V3_3;
			break;
		}
	}
	debug("%s: pwr = %X\n", __func__, pwr);

	/* Set the bus voltage first (if any) */
	writeb(pwr, &priv->reg->pwrcon);
	if (pwr == 0)
		return;

	/* Now enable bus power */
	pwr |= TEGRA_MMC_PWRCTL_SD_BUS_POWER;
	writeb(pwr, &priv->reg->pwrcon);
}

static void tegra_mmc_prepare_data(struct tegra_mmc_priv *priv,
				   struct mmc_data *data,
				   struct bounce_buffer *bbstate)
{
	unsigned char ctrl;


	debug("buf: %p (%p), data->blocks: %u, data->blocksize: %u\n",
		bbstate->bounce_buffer, bbstate->user_buffer, data->blocks,
		data->blocksize);

	writel((u32)(unsigned long)bbstate->bounce_buffer, &priv->reg->sysad);
	/*
	 * DMASEL[4:3]
	 * 00 = Selects SDMA
	 * 01 = Reserved
	 * 10 = Selects 32-bit Address ADMA2
	 * 11 = Selects 64-bit Address ADMA2
	 */
	ctrl = readb(&priv->reg->hostctl);
	ctrl &= ~TEGRA_MMC_HOSTCTL_DMASEL_MASK;
	ctrl |= TEGRA_MMC_HOSTCTL_DMASEL_SDMA;
	writeb(ctrl, &priv->reg->hostctl);

	/* We do not handle DMA boundaries, so set it to max (512 KiB) */
	writew((7 << 12) | (data->blocksize & 0xFFF), &priv->reg->blksize);
	writew(data->blocks, &priv->reg->blkcnt);
}

static void tegra_mmc_set_transfer_mode(struct tegra_mmc_priv *priv,
					struct mmc_data *data)
{
	unsigned short mode;
	debug(" mmc_set_transfer_mode called\n");
	/*
	 * TRNMOD
	 * MUL1SIN0[5]	: Multi/Single Block Select
	 * RD1WT0[4]	: Data Transfer Direction Select
	 *	1 = read
	 *	0 = write
	 * ENACMD12[2]	: Auto CMD12 Enable
	 * ENBLKCNT[1]	: Block Count Enable
	 * ENDMA[0]	: DMA Enable
	 */
	mode = (TEGRA_MMC_TRNMOD_DMA_ENABLE |
		TEGRA_MMC_TRNMOD_BLOCK_COUNT_ENABLE);

	if (data->blocks > 1)
		mode |= TEGRA_MMC_TRNMOD_MULTI_BLOCK_SELECT;

	if (data->flags & MMC_DATA_READ)
		mode |= TEGRA_MMC_TRNMOD_DATA_XFER_DIR_SEL_READ;

	writew(mode, &priv->reg->trnmod);
}

static int tegra_mmc_wait_inhibit(struct tegra_mmc_priv *priv,
				  struct mmc_cmd *cmd,
				  struct mmc_data *data,
				  unsigned int timeout)
{
	/*
	 * PRNSTS
	 * CMDINHDAT[1] : Command Inhibit (DAT)
	 * CMDINHCMD[0] : Command Inhibit (CMD)
	 */
	unsigned int mask = TEGRA_MMC_PRNSTS_CMD_INHIBIT_CMD;

	/*
	 * We shouldn't wait for data inhibit for stop commands, even
	 * though they might use busy signaling
	 */
	if ((data == NULL) && (cmd->resp_type & MMC_RSP_BUSY))
		mask |= TEGRA_MMC_PRNSTS_CMD_INHIBIT_DAT;

	while (readl(&priv->reg->prnsts) & mask) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return -1;
		}
		timeout--;
		udelay(1000);
	}

	return 0;
}

static int tegra_mmc_send_cmd_bounced(struct udevice *dev, struct mmc_cmd *cmd,
				      struct mmc_data *data,
				      struct bounce_buffer *bbstate)
{
	struct tegra_mmc_priv *priv = dev_get_priv(dev);
	int flags, i;
	int result;
	unsigned int mask = 0;
	unsigned int retry = 0x100000;
	debug(" mmc_send_cmd called\n");

	result = tegra_mmc_wait_inhibit(priv, cmd, data, 10 /* ms */);

	if (result < 0)
		return result;

	if (data)
		tegra_mmc_prepare_data(priv, data, bbstate);

	debug("cmd->arg: %08x\n", cmd->cmdarg);
	writel(cmd->cmdarg, &priv->reg->argument);

	if (data)
		tegra_mmc_set_transfer_mode(priv, data);

	if ((cmd->resp_type & MMC_RSP_136) && (cmd->resp_type & MMC_RSP_BUSY))
		return -1;

	/*
	 * CMDREG
	 * CMDIDX[13:8]	: Command index
	 * DATAPRNT[5]	: Data Present Select
	 * ENCMDIDX[4]	: Command Index Check Enable
	 * ENCMDCRC[3]	: Command CRC Check Enable
	 * RSPTYP[1:0]
	 *	00 = No Response
	 *	01 = Length 136
	 *	10 = Length 48
	 *	11 = Length 48 Check busy after response
	 */
	if (!(cmd->resp_type & MMC_RSP_PRESENT))
		flags = TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_NO_RESPONSE;
	else if (cmd->resp_type & MMC_RSP_136)
		flags = TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_LENGTH_136;
	else if (cmd->resp_type & MMC_RSP_BUSY)
		flags = TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_LENGTH_48_BUSY;
	else
		flags = TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_LENGTH_48;

	if (cmd->resp_type & MMC_RSP_CRC)
		flags |= TEGRA_MMC_TRNMOD_CMD_CRC_CHECK;
	if (cmd->resp_type & MMC_RSP_OPCODE)
		flags |= TEGRA_MMC_TRNMOD_CMD_INDEX_CHECK;
	if (data)
		flags |= TEGRA_MMC_TRNMOD_DATA_PRESENT_SELECT_DATA_TRANSFER;

	debug("cmd: %d\n", cmd->cmdidx);

	writew((cmd->cmdidx << 8) | flags, &priv->reg->cmdreg);

	for (i = 0; i < retry; i++) {
		mask = readl(&priv->reg->norintsts);
		/* Command Complete */
		if (mask & TEGRA_MMC_NORINTSTS_CMD_COMPLETE) {
			if (!data)
				writel(mask, &priv->reg->norintsts);
			break;
		}
	}

	if (i == retry) {
		printf("%s: waiting for status update\n", __func__);
		writel(mask, &priv->reg->norintsts);
		return -ETIMEDOUT;
	}

	if (mask & TEGRA_MMC_NORINTSTS_CMD_TIMEOUT) {
		/* Timeout Error */
		debug("timeout: %08x cmd %d\n", mask, cmd->cmdidx);
		writel(mask, &priv->reg->norintsts);
		return -ETIMEDOUT;
	} else if (mask & TEGRA_MMC_NORINTSTS_ERR_INTERRUPT) {
		/* Error Interrupt */
		debug("error: %08x cmd %d\n", mask, cmd->cmdidx);
		writel(mask, &priv->reg->norintsts);
		return -1;
	}

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd->resp_type & MMC_RSP_136) {
			/* CRC is stripped so we need to do some shifting. */
			for (i = 0; i < 4; i++) {
				unsigned long offset = (unsigned long)
					(&priv->reg->rspreg3 - i);
				cmd->response[i] = readl(offset) << 8;

				if (i != 3) {
					cmd->response[i] |=
						readb(offset - 1);
				}
				debug("cmd->resp[%d]: %08x\n",
						i, cmd->response[i]);
			}
		} else if (cmd->resp_type & MMC_RSP_BUSY) {
			for (i = 0; i < retry; i++) {
				/* PRNTDATA[23:20] : DAT[3:0] Line Signal */
				if (readl(&priv->reg->prnsts)
					& (1 << 20))	/* DAT[0] */
					break;
			}

			if (i == retry) {
				printf("%s: card is still busy\n", __func__);
				writel(mask, &priv->reg->norintsts);
				return -ETIMEDOUT;
			}

			cmd->response[0] = readl(&priv->reg->rspreg0);
			debug("cmd->resp[0]: %08x\n", cmd->response[0]);
		} else {
			cmd->response[0] = readl(&priv->reg->rspreg0);
			debug("cmd->resp[0]: %08x\n", cmd->response[0]);
		}
	}

	if (data) {
		unsigned long	start = get_timer(0);

		while (1) {
			mask = readl(&priv->reg->norintsts);

			if (mask & TEGRA_MMC_NORINTSTS_ERR_INTERRUPT) {
				/* Error Interrupt */
				writel(mask, &priv->reg->norintsts);
				printf("%s: error during transfer: 0x%08x\n",
						__func__, mask);
				return -1;
			} else if (mask & TEGRA_MMC_NORINTSTS_DMA_INTERRUPT) {
				/*
				 * DMA Interrupt, restart the transfer where
				 * it was interrupted.
				 */
				unsigned int address = readl(&priv->reg->sysad);

				debug("DMA end\n");
				writel(TEGRA_MMC_NORINTSTS_DMA_INTERRUPT,
				       &priv->reg->norintsts);
				writel(address, &priv->reg->sysad);
			} else if (mask & TEGRA_MMC_NORINTSTS_XFER_COMPLETE) {
				/* Transfer Complete */
				debug("r/w is done\n");
				break;
			} else if (get_timer(start) > 8000UL) {
				writel(mask, &priv->reg->norintsts);
				printf("%s: MMC Timeout\n"
				       "    Interrupt status        0x%08x\n"
				       "    Interrupt status enable 0x%08x\n"
				       "    Interrupt signal enable 0x%08x\n"
				       "    Present status          0x%08x\n",
				       __func__, mask,
				       readl(&priv->reg->norintstsen),
				       readl(&priv->reg->norintsigen),
				       readl(&priv->reg->prnsts));
				return -1;
			}
		}
		writel(mask, &priv->reg->norintsts);
	}

	udelay(1000);
	return 0;
}

static int tegra_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	void *buf;
	unsigned int bbflags;
	size_t len;
	struct bounce_buffer bbstate;
	int ret;

	if (data) {
		if (data->flags & MMC_DATA_READ) {
			buf = data->dest;
			bbflags = GEN_BB_WRITE;
		} else {
			buf = (void *)data->src;
			bbflags = GEN_BB_READ;
		}
		len = data->blocks * data->blocksize;

		bounce_buffer_start(&bbstate, buf, len, bbflags);
	}

	ret = tegra_mmc_send_cmd_bounced(dev, cmd, data, &bbstate);

	if (data)
		bounce_buffer_stop(&bbstate);

	return ret;
}

static void tegra_mmc_change_clock(struct tegra_mmc_priv *priv, uint clock)
{
	ulong rate;
	int div;
	unsigned short clk;
	unsigned long timeout;

	debug(" mmc_change_clock called\n");

	/*
	 * Change Tegra SDMMCx clock divisor here. Source is PLLP_OUT0
	 */
	if (clock == 0)
		goto out;

	rate = clk_set_rate(&priv->clk, clock);
	div = (rate + clock - 1) / clock;
	debug("div = %d\n", div);

	writew(0, &priv->reg->clkcon);

	/*
	 * CLKCON
	 * SELFREQ[15:8]	: base clock divided by value
	 * ENSDCLK[2]		: SD Clock Enable
	 * STBLINTCLK[1]	: Internal Clock Stable
	 * ENINTCLK[0]		: Internal Clock Enable
	 */
	div >>= 1;
	clk = ((div << TEGRA_MMC_CLKCON_SDCLK_FREQ_SEL_SHIFT) |
	       TEGRA_MMC_CLKCON_INTERNAL_CLOCK_ENABLE);
	writew(clk, &priv->reg->clkcon);

	/* Wait max 10 ms */
	timeout = 10;
	while (!(readw(&priv->reg->clkcon) &
		 TEGRA_MMC_CLKCON_INTERNAL_CLOCK_STABLE)) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return;
		}
		timeout--;
		udelay(1000);
	}

	clk |= TEGRA_MMC_CLKCON_SD_CLOCK_ENABLE;
	writew(clk, &priv->reg->clkcon);

	debug("mmc_change_clock: clkcon = %08X\n", clk);

out:
	priv->clock = clock;
}

static int tegra_mmc_set_ios(struct udevice *dev)
{
	struct tegra_mmc_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	unsigned char ctrl;
	debug(" mmc_set_ios called\n");

	debug("bus_width: %x, clock: %d\n", mmc->bus_width, mmc->clock);

	/* Change clock first */
	tegra_mmc_change_clock(priv, mmc->clock);

	ctrl = readb(&priv->reg->hostctl);

	/*
	 * WIDE8[5]
	 * 0 = Depend on WIDE4
	 * 1 = 8-bit mode
	 * WIDE4[1]
	 * 1 = 4-bit mode
	 * 0 = 1-bit mode
	 */
	if (mmc->bus_width == 8)
		ctrl |= (1 << 5);
	else if (mmc->bus_width == 4)
		ctrl |= (1 << 1);
	else
		ctrl &= ~(1 << 1 | 1 << 5);

	writeb(ctrl, &priv->reg->hostctl);
	debug("mmc_set_ios: hostctl = %08X\n", ctrl);

	return 0;
}

static void tegra_mmc_pad_init(struct tegra_mmc_priv *priv)
{
#if defined(CONFIG_TEGRA30)
	u32 val;

	debug("%s: sdmmc address = %08x\n", __func__, (unsigned int)priv->reg);

	/* Set the pad drive strength for SDMMC1 or 3 only */
	if (priv->reg != (void *)0x78000000 &&
	    priv->reg != (void *)0x78000400) {
		debug("%s: settings are only valid for SDMMC1/SDMMC3!\n",
		      __func__);
		return;
	}

	val = readl(&priv->reg->sdmemcmppadctl);
	val &= 0xFFFFFFF0;
	val |= MEMCOMP_PADCTRL_VREF;
	writel(val, &priv->reg->sdmemcmppadctl);

	val = readl(&priv->reg->autocalcfg);
	val &= 0xFFFF0000;
	val |= AUTO_CAL_PU_OFFSET | AUTO_CAL_PD_OFFSET | AUTO_CAL_ENABLED;
	writel(val, &priv->reg->autocalcfg);
#endif
}

static void tegra_mmc_reset(struct tegra_mmc_priv *priv, struct mmc *mmc)
{
	unsigned int timeout;
	debug(" mmc_reset called\n");

	/*
	 * RSTALL[0] : Software reset for all
	 * 1 = reset
	 * 0 = work
	 */
	writeb(TEGRA_MMC_SWRST_SW_RESET_FOR_ALL, &priv->reg->swrst);

	priv->clock = 0;

	/* Wait max 100 ms */
	timeout = 100;

	/* hw clears the bit when it's done */
	while (readb(&priv->reg->swrst) & TEGRA_MMC_SWRST_SW_RESET_FOR_ALL) {
		if (timeout == 0) {
			printf("%s: timeout error\n", __func__);
			return;
		}
		timeout--;
		udelay(1000);
	}

	/* Set SD bus voltage & enable bus power */
	tegra_mmc_set_power(priv, fls(mmc->cfg->voltages) - 1);
	debug("%s: power control = %02X, host control = %02X\n", __func__,
		readb(&priv->reg->pwrcon), readb(&priv->reg->hostctl));

	/* Make sure SDIO pads are set up */
	tegra_mmc_pad_init(priv);
}

static int tegra_mmc_init(struct udevice *dev)
{
	struct tegra_mmc_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	unsigned int mask;
	debug(" tegra_mmc_init called\n");

	tegra_mmc_reset(priv, mmc);

#if defined(CONFIG_TEGRA124_MMC_DISABLE_EXT_LOOPBACK)
	/*
	 * Disable the external clock loopback and use the internal one on
	 * SDMMC3 as per the SDMMC_VENDOR_MISC_CNTRL_0 register's SDMMC_SPARE1
	 * bits being set to 0xfffd according to the TRM.
	 *
	 * TODO(marcel.ziswiler@toradex.com): Move to device tree controlled
	 * approach once proper kernel integration made it mainline.
	 */
	if (priv->reg == (void *)0x700b0400) {
		mask = readl(&priv->reg->venmiscctl);
		mask &= ~TEGRA_MMC_MISCON_ENABLE_EXT_LOOPBACK;
		writel(mask, &priv->reg->venmiscctl);
	}
#endif

	priv->version = readw(&priv->reg->hcver);
	debug("host version = %x\n", priv->version);

	/* mask all */
	writel(0xffffffff, &priv->reg->norintstsen);
	writel(0xffffffff, &priv->reg->norintsigen);

	writeb(0xe, &priv->reg->timeoutcon);	/* TMCLK * 2^27 */
	/*
	 * NORMAL Interrupt Status Enable Register init
	 * [5] ENSTABUFRDRDY : Buffer Read Ready Status Enable
	 * [4] ENSTABUFWTRDY : Buffer write Ready Status Enable
	 * [3] ENSTADMAINT   : DMA boundary interrupt
	 * [1] ENSTASTANSCMPLT : Transfre Complete Status Enable
	 * [0] ENSTACMDCMPLT : Command Complete Status Enable
	*/
	mask = readl(&priv->reg->norintstsen);
	mask &= ~(0xffff);
	mask |= (TEGRA_MMC_NORINTSTSEN_CMD_COMPLETE |
		 TEGRA_MMC_NORINTSTSEN_XFER_COMPLETE |
		 TEGRA_MMC_NORINTSTSEN_DMA_INTERRUPT |
		 TEGRA_MMC_NORINTSTSEN_BUFFER_WRITE_READY |
		 TEGRA_MMC_NORINTSTSEN_BUFFER_READ_READY);
	writel(mask, &priv->reg->norintstsen);

	/*
	 * NORMAL Interrupt Signal Enable Register init
	 * [1] ENSTACMDCMPLT : Transfer Complete Signal Enable
	 */
	mask = readl(&priv->reg->norintsigen);
	mask &= ~(0xffff);
	mask |= TEGRA_MMC_NORINTSIGEN_XFER_COMPLETE;
	writel(mask, &priv->reg->norintsigen);

	return 0;
}

static int tegra_mmc_getcd(struct udevice *dev)
{
	struct tegra_mmc_priv *priv = dev_get_priv(dev);

	debug("tegra_mmc_getcd called\n");

	if (dm_gpio_is_valid(&priv->cd_gpio))
		return dm_gpio_get_value(&priv->cd_gpio);

	return 1;
}

static const struct dm_mmc_ops tegra_mmc_ops = {
	.send_cmd	= tegra_mmc_send_cmd,
	.set_ios	= tegra_mmc_set_ios,
	.get_cd		= tegra_mmc_getcd,
};

static int tegra_mmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct tegra_mmc_plat *plat = dev_get_platdata(dev);
	struct tegra_mmc_priv *priv = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	int bus_width, ret;

	cfg->name = dev->name;

	bus_width = dev_read_u32_default(dev, "bus-width", 1);

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	cfg->host_caps = 0;
	if (bus_width == 8)
		cfg->host_caps |= MMC_MODE_8BIT;
	if (bus_width >= 4)
		cfg->host_caps |= MMC_MODE_4BIT;
	cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

	/*
	 * min freq is for card identification, and is the highest
	 *  low-speed SDIO card frequency (actually 400KHz)
	 * max freq is highest HS eMMC clock as per the SD/MMC spec
	 *  (actually 52MHz)
	 */
	cfg->f_min = 375000;
	cfg->f_max = 48000000;

	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	priv->reg = (void *)dev_read_addr(dev);

	ret = reset_get_by_name(dev, "sdhci", &priv->reset_ctl);
	if (ret) {
		debug("reset_get_by_name() failed: %d\n", ret);
		return ret;
	}
	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret) {
		debug("clk_get_by_index() failed: %d\n", ret);
		return ret;
	}

	ret = reset_assert(&priv->reset_ctl);
	if (ret)
		return ret;
	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;
	ret = clk_set_rate(&priv->clk, 20000000);
	if (IS_ERR_VALUE(ret))
		return ret;
	ret = reset_deassert(&priv->reset_ctl);
	if (ret)
		return ret;

	/* These GPIOs are optional */
	gpio_request_by_name(dev, "cd-gpios", 0, &priv->cd_gpio, GPIOD_IS_IN);
	gpio_request_by_name(dev, "wp-gpios", 0, &priv->wp_gpio, GPIOD_IS_IN);
	gpio_request_by_name(dev, "power-gpios", 0, &priv->pwr_gpio,
			     GPIOD_IS_OUT);
	if (dm_gpio_is_valid(&priv->pwr_gpio))
		dm_gpio_set_value(&priv->pwr_gpio, 1);

	upriv->mmc = &plat->mmc;

	return tegra_mmc_init(dev);
}

static int tegra_mmc_bind(struct udevice *dev)
{
	struct tegra_mmc_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id tegra_mmc_ids[] = {
	{ .compatible = "nvidia,tegra20-sdhci" },
	{ .compatible = "nvidia,tegra30-sdhci" },
	{ .compatible = "nvidia,tegra114-sdhci" },
	{ .compatible = "nvidia,tegra124-sdhci" },
	{ .compatible = "nvidia,tegra210-sdhci" },
	{ .compatible = "nvidia,tegra186-sdhci" },
	{ }
};

U_BOOT_DRIVER(tegra_mmc_drv) = {
	.name		= "tegra_mmc",
	.id		= UCLASS_MMC,
	.of_match	= tegra_mmc_ids,
	.bind		= tegra_mmc_bind,
	.probe		= tegra_mmc_probe,
	.ops		= &tegra_mmc_ops,
	.platdata_auto_alloc_size = sizeof(struct tegra_mmc_plat),
	.priv_auto_alloc_size = sizeof(struct tegra_mmc_priv),
};
