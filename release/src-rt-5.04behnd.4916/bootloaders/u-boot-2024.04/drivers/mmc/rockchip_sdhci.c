// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Fuzhou Rockchip Electronics Co., Ltd
 *
 * Rockchip SD Host Controller Interface
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <dt-structs.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/libfdt.h>
#include <linux/iopoll.h>
#include <malloc.h>
#include <mapmem.h>
#include "mmc_private.h"
#include <sdhci.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>

/* DWCMSHC specific Mode Select value */
#define DWCMSHC_CTRL_HS400		0x7
/* 400KHz is max freq for card ID etc. Use that as min */
#define EMMC_MIN_FREQ	400000
#define KHz	(1000)
#define MHz	(1000 * KHz)
#define SDHCI_TUNING_LOOP_COUNT		40

#define PHYCTRL_CALDONE_MASK		0x1
#define PHYCTRL_CALDONE_SHIFT		0x6
#define PHYCTRL_CALDONE_DONE		0x1
#define PHYCTRL_DLLRDY_MASK		0x1
#define PHYCTRL_DLLRDY_SHIFT		0x5
#define PHYCTRL_DLLRDY_DONE		0x1
#define PHYCTRL_FREQSEL_200M		0x0
#define PHYCTRL_FREQSEL_50M		0x1
#define PHYCTRL_FREQSEL_100M		0x2
#define PHYCTRL_FREQSEL_150M		0x3
#define PHYCTRL_DLL_LOCK_WO_TMOUT(x)	\
	((((x) >> PHYCTRL_DLLRDY_SHIFT) & PHYCTRL_DLLRDY_MASK) ==\
	PHYCTRL_DLLRDY_DONE)

#define ARASAN_VENDOR_REGISTER		0x78
#define ARASAN_VENDOR_ENHANCED_STROBE	BIT(0)

/* Rockchip specific Registers */
#define DWCMSHC_EMMC_EMMC_CTRL		0x52c
#define DWCMSHC_CARD_IS_EMMC		BIT(0)
#define DWCMSHC_ENHANCED_STROBE		BIT(8)
#define DWCMSHC_EMMC_DLL_CTRL		0x800
#define DWCMSHC_EMMC_DLL_CTRL_RESET	BIT(1)
#define DWCMSHC_EMMC_DLL_RXCLK		0x804
#define DWCMSHC_EMMC_DLL_TXCLK		0x808
#define DWCMSHC_EMMC_DLL_STRBIN		0x80c
#define DWCMSHC_EMMC_DLL_CMDOUT		0x810
#define DWCMSHC_EMMC_DLL_STATUS0	0x840
#define DWCMSHC_EMMC_DLL_STATUS1	0x844
#define DWCMSHC_EMMC_DLL_START		BIT(0)
#define DWCMSHC_EMMC_DLL_LOCKED		BIT(8)
#define DWCMSHC_EMMC_DLL_TIMEOUT	BIT(9)
#define DWCMSHC_EMMC_DLL_START_POINT	16
#define DWCMSHC_EMMC_DLL_START_DEFAULT	5
#define DWCMSHC_EMMC_DLL_INC_VALUE	2
#define DWCMSHC_EMMC_DLL_INC		8
#define DWCMSHC_EMMC_DLL_BYPASS		BIT(24)
#define DWCMSHC_EMMC_DLL_DLYENA		BIT(27)
#define DLL_RXCLK_NO_INVERTER		BIT(29)
#define DLL_RXCLK_ORI_GATE		BIT(31)
#define DLL_TXCLK_TAPNUM_DEFAULT	0x10
#define DLL_TXCLK_TAPNUM_FROM_SW	BIT(24)
#define DLL_TXCLK_NO_INVERTER		BIT(29)
#define DLL_STRBIN_TAPNUM_DEFAULT	0x4
#define DLL_STRBIN_TAPNUM_FROM_SW	BIT(24)
#define DLL_STRBIN_DELAY_NUM_SEL	BIT(26)
#define DLL_STRBIN_DELAY_NUM_OFFSET	16
#define DLL_STRBIN_DELAY_NUM_DEFAULT	0x10
#define DLL_CMDOUT_TAPNUM_90_DEGREES	0x8
#define DLL_CMDOUT_TAPNUM_FROM_SW	BIT(24)
#define DLL_CMDOUT_SRC_CLK_NEG		BIT(28)
#define DLL_CMDOUT_EN_SRC_CLK_NEG	BIT(29)
#define DLL_CMDOUT_BOTH_CLK_EDGE	BIT(30)

#define DLL_LOCK_WO_TMOUT(x) \
	((((x) & DWCMSHC_EMMC_DLL_LOCKED) == DWCMSHC_EMMC_DLL_LOCKED) && \
	(((x) & DWCMSHC_EMMC_DLL_TIMEOUT) == 0))
#define ROCKCHIP_MAX_CLKS		3

#define FLAG_INVERTER_FLAG_IN_RXCLK	BIT(0)

struct rockchip_sdhc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct rockchip_emmc_phy {
	u32 emmcphy_con[7];
	u32 reserved;
	u32 emmcphy_status;
};

struct rockchip_sdhc {
	struct sdhci_host host;
	struct udevice *dev;
	void *base;
	struct rockchip_emmc_phy *phy;
	struct clk emmc_clk;
};

struct sdhci_data {
	int (*get_phy)(struct udevice *dev);

	/**
	 * set_control_reg() - Set SDHCI control registers
	 *
	 * This is the set_control_reg() SDHCI operation that should be
	 * used for the hardware this driver data is associated with.
	 * Normally, this is used to set up control registers for
	 * voltage level and UHS speed mode.
	 *
	 * @host: SDHCI host structure
	 */
	void (*set_control_reg)(struct sdhci_host *host);

	/**
	 * set_ios_post() - Host specific hook after set_ios() calls
	 *
	 * This is the set_ios_post() SDHCI operation that should be
	 * used for the hardware this driver data is associated with.
	 * Normally, this is a hook that is called after sdhci_set_ios()
	 * that does any necessary host-specific configuration.
	 *
	 * @host: SDHCI host structure
	 * Return: 0 if successful, -ve on error
	 */
	int (*set_ios_post)(struct sdhci_host *host);

	void (*set_clock)(struct sdhci_host *host, u32 div);
	int (*config_dll)(struct sdhci_host *host, u32 clock, bool enable);

	/**
	 * set_enhanced_strobe() - Set HS400 Enhanced Strobe config
	 *
	 * This is the set_enhanced_strobe() SDHCI operation that should
	 * be used for the hardware this driver data is associated with.
	 * Normally, this is used to set any host-specific configuration
	 * necessary for HS400 ES.
	 *
	 * @host: SDHCI host structure
	 * Return: 0 if successful, -ve on error
	 */
	int (*set_enhanced_strobe)(struct sdhci_host *host);

	u32 flags;
	u8 hs200_txclk_tapnum;
	u8 hs400_txclk_tapnum;
};

static void rk3399_emmc_phy_power_on(struct rockchip_emmc_phy *phy, u32 clock)
{
	u32 caldone, dllrdy, freqsel;

	writel(RK_CLRSETBITS(7 << 4, 0), &phy->emmcphy_con[6]);
	writel(RK_CLRSETBITS(1 << 11, 1 << 11), &phy->emmcphy_con[0]);
	writel(RK_CLRSETBITS(0xf << 7, 6 << 7), &phy->emmcphy_con[0]);

	/*
	 * According to the user manual, calpad calibration
	 * cycle takes more than 2us without the minimal recommended
	 * value, so we may need a little margin here
	 */
	udelay(3);
	writel(RK_CLRSETBITS(1, 1), &phy->emmcphy_con[6]);

	/*
	 * According to the user manual, it asks driver to
	 * wait 5us for calpad busy trimming. But it seems that
	 * 5us of caldone isn't enough for all cases.
	 */
	udelay(500);
	caldone = readl(&phy->emmcphy_status);
	caldone = (caldone >> PHYCTRL_CALDONE_SHIFT) & PHYCTRL_CALDONE_MASK;
	if (caldone != PHYCTRL_CALDONE_DONE) {
		printf("%s: caldone timeout.\n", __func__);
		return;
	}

	/* Set the frequency of the DLL operation */
	if (clock < 75 * MHz)
		freqsel = PHYCTRL_FREQSEL_50M;
	else if (clock < 125 * MHz)
		freqsel = PHYCTRL_FREQSEL_100M;
	else if (clock < 175 * MHz)
		freqsel = PHYCTRL_FREQSEL_150M;
	else
		freqsel = PHYCTRL_FREQSEL_200M;

	/* Set the frequency of the DLL operation */
	writel(RK_CLRSETBITS(3 << 12, freqsel << 12), &phy->emmcphy_con[0]);
	writel(RK_CLRSETBITS(1 << 1, 1 << 1), &phy->emmcphy_con[6]);

	/* REN Enable on STRB Line for HS400 */
	writel(RK_CLRSETBITS(0, 1 << 9), &phy->emmcphy_con[2]);

	read_poll_timeout(readl, dllrdy, PHYCTRL_DLL_LOCK_WO_TMOUT(dllrdy), 1,
			  5000, &phy->emmcphy_status);
}

static void rk3399_emmc_phy_power_off(struct rockchip_emmc_phy *phy)
{
	writel(RK_CLRSETBITS(1, 0), &phy->emmcphy_con[6]);
	writel(RK_CLRSETBITS(1 << 1, 0), &phy->emmcphy_con[6]);
}

static int rk3399_emmc_get_phy(struct udevice *dev)
{
	struct rockchip_sdhc *priv = dev_get_priv(dev);
	ofnode phy_node;
	void *grf_base;
	u32 grf_phy_offset, phandle;

	phandle = dev_read_u32_default(dev, "phys", 0);
	phy_node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(phy_node)) {
		debug("Not found emmc phy device\n");
		return -ENODEV;
	}

	grf_base = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR_OR_NULL(grf_base)) {
		printf("%s Get syscon grf failed", __func__);
		return -ENODEV;
	}
	grf_phy_offset = ofnode_read_u32_default(phy_node, "reg", 0);

	priv->phy = (struct rockchip_emmc_phy *)(grf_base + grf_phy_offset);

	return 0;
}

static int rk3399_sdhci_set_enhanced_strobe(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	u32 vendor;

	vendor = sdhci_readl(host, ARASAN_VENDOR_REGISTER);
	if (mmc->selected_mode == MMC_HS_400_ES)
		vendor |= ARASAN_VENDOR_ENHANCED_STROBE;
	else
		vendor &= ~ARASAN_VENDOR_ENHANCED_STROBE;
	sdhci_writel(host, vendor, ARASAN_VENDOR_REGISTER);

	return 0;
}

static void rk3399_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct mmc *mmc = host->mmc;
	uint clock = mmc->tran_speed;
	int cycle_phy = host->clock != clock && clock > EMMC_MIN_FREQ;

	if (cycle_phy)
		rk3399_emmc_phy_power_off(priv->phy);

	sdhci_set_control_reg(host);

	/*
	 * Reinitializing the device tries to set it to lower-speed modes
	 * first, which fails if the Enhanced Strobe bit is set, making
	 * the device impossible to use. Set the correct value here to
	 * let reinitialization attempts succeed.
	 */
	if (CONFIG_IS_ENABLED(MMC_HS400_ES_SUPPORT))
		rk3399_sdhci_set_enhanced_strobe(host);
};

static int rk3399_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct mmc *mmc = host->mmc;
	uint clock = mmc->tran_speed;
	int cycle_phy = host->clock != clock && clock > EMMC_MIN_FREQ;

	if (!clock)
		clock = mmc->clock;

	if (cycle_phy)
		rk3399_emmc_phy_power_on(priv->phy, clock);

	return 0;
}

static void rk3568_sdhci_set_clock(struct sdhci_host *host, u32 div)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct mmc *mmc = host->mmc;
	ulong rate;

	rate = clk_set_rate(&priv->emmc_clk, mmc->clock);
	if (IS_ERR_VALUE(rate))
		printf("%s: Set clock rate failed: %ld\n", __func__, (long)rate);
}

static int rk3568_sdhci_config_dll(struct sdhci_host *host, u32 clock, bool enable)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(priv->dev);
	struct mmc *mmc = host->mmc;
	int val, ret;
	u32 extra, txclk_tapnum;

	if (!enable) {
		sdhci_writel(host, 0, DWCMSHC_EMMC_DLL_CTRL);
		return 0;
	}

	if (clock >= 100 * MHz) {
		/* reset DLL */
		sdhci_writel(host, DWCMSHC_EMMC_DLL_CTRL_RESET, DWCMSHC_EMMC_DLL_CTRL);
		udelay(1);
		sdhci_writel(host, 0, DWCMSHC_EMMC_DLL_CTRL);

		/* Init DLL settings */
		extra = DWCMSHC_EMMC_DLL_START_DEFAULT << DWCMSHC_EMMC_DLL_START_POINT |
			DWCMSHC_EMMC_DLL_INC_VALUE << DWCMSHC_EMMC_DLL_INC |
			DWCMSHC_EMMC_DLL_START;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_CTRL);

		ret = read_poll_timeout(readl, val, DLL_LOCK_WO_TMOUT(val), 1,
					500,
					host->ioaddr + DWCMSHC_EMMC_DLL_STATUS0);
		if (ret)
			return ret;

		extra = DWCMSHC_EMMC_DLL_DLYENA | DLL_RXCLK_ORI_GATE;
		if (data->flags & FLAG_INVERTER_FLAG_IN_RXCLK)
			extra |= DLL_RXCLK_NO_INVERTER;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_RXCLK);

		txclk_tapnum = data->hs200_txclk_tapnum;
		if (mmc->selected_mode == MMC_HS_400 ||
		    mmc->selected_mode == MMC_HS_400_ES) {
			txclk_tapnum = data->hs400_txclk_tapnum;

			extra = DLL_CMDOUT_SRC_CLK_NEG |
				DLL_CMDOUT_BOTH_CLK_EDGE |
				DWCMSHC_EMMC_DLL_DLYENA |
				DLL_CMDOUT_TAPNUM_90_DEGREES |
				DLL_CMDOUT_TAPNUM_FROM_SW;
			sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_CMDOUT);
		}

		extra = DWCMSHC_EMMC_DLL_DLYENA |
			DLL_TXCLK_TAPNUM_FROM_SW |
			DLL_TXCLK_NO_INVERTER |
			txclk_tapnum;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_TXCLK);

		extra = DWCMSHC_EMMC_DLL_DLYENA |
			DLL_STRBIN_TAPNUM_DEFAULT |
			DLL_STRBIN_TAPNUM_FROM_SW;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_STRBIN);
	} else {
		/*
		 * Disable DLL and reset both of sample and drive clock.
		 * The bypass bit and start bit need to be set if DLL is not locked.
		 */
		extra = DWCMSHC_EMMC_DLL_BYPASS | DWCMSHC_EMMC_DLL_START;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_CTRL);
		sdhci_writel(host, DLL_RXCLK_ORI_GATE, DWCMSHC_EMMC_DLL_RXCLK);
		sdhci_writel(host, 0, DWCMSHC_EMMC_DLL_TXCLK);
		sdhci_writel(host, 0, DWCMSHC_EMMC_DLL_CMDOUT);
		/*
		 * Before switching to hs400es mode, the driver will enable
		 * enhanced strobe first. PHY needs to configure the parameters
		 * of enhanced strobe first.
		 */
		extra = DWCMSHC_EMMC_DLL_DLYENA |
			DLL_STRBIN_DELAY_NUM_SEL |
			DLL_STRBIN_DELAY_NUM_DEFAULT << DLL_STRBIN_DELAY_NUM_OFFSET;
		sdhci_writel(host, extra, DWCMSHC_EMMC_DLL_STRBIN);
	}

	return 0;
}

static int rk3568_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct mmc *mmc = host->mmc;
	u32 reg;

	reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	reg &= ~SDHCI_CTRL_UHS_MASK;

	switch (mmc->selected_mode) {
	case UHS_SDR25:
	case MMC_HS:
	case MMC_HS_52:
		reg |= SDHCI_CTRL_UHS_SDR25;
		break;
	case UHS_SDR50:
		reg |= SDHCI_CTRL_UHS_SDR50;
		break;
	case UHS_DDR50:
	case MMC_DDR_52:
		reg |= SDHCI_CTRL_UHS_DDR50;
		break;
	case UHS_SDR104:
	case MMC_HS_200:
		reg |= SDHCI_CTRL_UHS_SDR104;
		break;
	case MMC_HS_400:
	case MMC_HS_400_ES:
		reg |= DWCMSHC_CTRL_HS400;
		break;
	default:
		reg |= SDHCI_CTRL_UHS_SDR12;
	}

	sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);

	reg = sdhci_readw(host, DWCMSHC_EMMC_EMMC_CTRL);

	if (IS_MMC(mmc))
		reg |= DWCMSHC_CARD_IS_EMMC;
	else
		reg &= ~DWCMSHC_CARD_IS_EMMC;

	if (mmc->selected_mode == MMC_HS_400_ES)
		reg |= DWCMSHC_ENHANCED_STROBE;
	else
		reg &= ~DWCMSHC_ENHANCED_STROBE;

	sdhci_writew(host, reg, DWCMSHC_EMMC_EMMC_CTRL);

	return 0;
}

static void rockchip_sdhci_set_control_reg(struct sdhci_host *host)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(priv->dev);

	if (data->set_control_reg)
		data->set_control_reg(host);
}

static int rockchip_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(priv->dev);

	if (data->set_ios_post)
		return data->set_ios_post(host);

	return 0;
}

static void rockchip_sdhci_set_clock(struct sdhci_host *host, u32 div)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(priv->dev);

	if (data->set_clock)
		data->set_clock(host, div);
}

static int rockchip_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct rockchip_sdhc *priv = dev_get_priv(mmc->dev);
	struct sdhci_host *host = &priv->host;
	char tuning_loop_counter = SDHCI_TUNING_LOOP_COUNT;
	struct mmc_cmd cmd;
	u32 ctrl, blk_size;
	int ret;

	ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
	ctrl |= SDHCI_CTRL_EXEC_TUNING;
	sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);

	sdhci_writel(host, SDHCI_INT_DATA_AVAIL, SDHCI_INT_ENABLE);

	blk_size = SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG, 64);
	if (opcode == MMC_CMD_SEND_TUNING_BLOCK_HS200 && mmc->bus_width == 8)
		blk_size = SDHCI_MAKE_BLKSZ(SDHCI_DEFAULT_BOUNDARY_ARG, 128);
	sdhci_writew(host, blk_size, SDHCI_BLOCK_SIZE);
	sdhci_writew(host, SDHCI_TRNS_READ, SDHCI_TRANSFER_MODE);

	cmd.cmdidx = opcode;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

	do {
		ret = mmc_send_cmd(mmc, &cmd, NULL);
		ctrl = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		if (ret || tuning_loop_counter-- == 0)
			break;
	} while (ctrl & SDHCI_CTRL_EXEC_TUNING);

	if (ret || tuning_loop_counter < 0 || !(ctrl & SDHCI_CTRL_TUNED_CLK)) {
		if (!ret)
			ret = -EIO;
		printf("%s: Tuning failed: %d\n", __func__, ret);

		ctrl &= ~SDHCI_CTRL_TUNED_CLK;
		ctrl &= ~SDHCI_CTRL_EXEC_TUNING;
		sdhci_writew(host, ctrl, SDHCI_HOST_CONTROL2);
	}

	/* Enable only interrupts served by the SD controller */
	sdhci_writel(host, SDHCI_INT_DATA_MASK | SDHCI_INT_CMD_MASK, SDHCI_INT_ENABLE);

	return ret;
}

static int rockchip_sdhci_config_dll(struct sdhci_host *host, u32 clock, bool enable)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(priv->dev);

	if (data->config_dll)
		return data->config_dll(host, clock, enable);

	return 0;
}

static int rockchip_sdhci_set_enhanced_strobe(struct sdhci_host *host)
{
	struct rockchip_sdhc *priv = container_of(host, struct rockchip_sdhc, host);
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(priv->dev);

	if (data->set_enhanced_strobe)
		return data->set_enhanced_strobe(host);

	return 0;
}

static struct sdhci_ops rockchip_sdhci_ops = {
	.set_control_reg = rockchip_sdhci_set_control_reg,
	.set_ios_post = rockchip_sdhci_set_ios_post,
	.set_clock = rockchip_sdhci_set_clock,
	.platform_execute_tuning = rockchip_sdhci_execute_tuning,
	.config_dll = rockchip_sdhci_config_dll,
	.set_enhanced_strobe = rockchip_sdhci_set_enhanced_strobe,
};

static int rockchip_sdhci_probe(struct udevice *dev)
{
	struct sdhci_data *data = (struct sdhci_data *)dev_get_driver_data(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct rockchip_sdhc_plat *plat = dev_get_plat(dev);
	struct rockchip_sdhc *priv = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct sdhci_host *host = &priv->host;
	struct clk clk;
	int ret;

	host->max_clk = cfg->f_max;
	ret = clk_get_by_index(dev, 0, &clk);
	if (!ret) {
		ret = clk_set_rate(&clk, host->max_clk);
		if (IS_ERR_VALUE(ret))
			printf("%s clk set rate fail!\n", __func__);
	} else {
		printf("%s fail to get clk\n", __func__);
	}

	priv->emmc_clk = clk;
	priv->dev = dev;

	if (data->get_phy) {
		ret = data->get_phy(dev);
		if (ret)
			return ret;
	}

	host->ops = &rockchip_sdhci_ops;
	host->quirks = SDHCI_QUIRK_WAIT_SEND_CMD;

	host->mmc = &plat->mmc;
	host->mmc->priv = &priv->host;
	host->mmc->dev = dev;
	upriv->mmc = host->mmc;

	ret = sdhci_setup_cfg(cfg, host, cfg->f_max, EMMC_MIN_FREQ);
	if (ret)
		return ret;

	/*
	 * Disable use of DMA and force use of PIO mode in SPL to fix an issue
	 * where loading part of TF-A into SRAM using DMA silently fails.
	 */
	if (IS_ENABLED(CONFIG_SPL_BUILD) &&
	    dev_read_bool(dev, "u-boot,spl-fifo-mode"))
		host->flags &= ~USE_DMA;

	/*
	 * Reading more than 4 blocks with a single CMD18 command in PIO mode
	 * triggers Data End Bit Error on RK3568 and RK3588. Limit to reading
	 * max 4 blocks in one command when using PIO mode.
	 */
	if (!(host->flags & USE_DMA) &&
	    (device_is_compatible(dev, "rockchip,rk3568-dwcmshc") ||
	     device_is_compatible(dev, "rockchip,rk3588-dwcmshc")))
		cfg->b_max = 4;

	return sdhci_probe(dev);
}

static int rockchip_sdhci_of_to_plat(struct udevice *dev)
{
	struct rockchip_sdhc_plat *plat = dev_get_plat(dev);
	struct rockchip_sdhc *priv = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	struct sdhci_host *host = &priv->host;
	int ret;

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);

	ret = mmc_of_parse(dev, cfg);
	if (ret)
		return ret;

	return 0;
}

static int rockchip_sdhci_bind(struct udevice *dev)
{
	struct rockchip_sdhc_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct sdhci_data rk3399_data = {
	.get_phy = rk3399_emmc_get_phy,
	.set_control_reg = rk3399_sdhci_set_control_reg,
	.set_ios_post = rk3399_sdhci_set_ios_post,
	.set_enhanced_strobe = rk3399_sdhci_set_enhanced_strobe,
};

static const struct sdhci_data rk3568_data = {
	.set_ios_post = rk3568_sdhci_set_ios_post,
	.set_clock = rk3568_sdhci_set_clock,
	.config_dll = rk3568_sdhci_config_dll,
	.flags = FLAG_INVERTER_FLAG_IN_RXCLK,
	.hs200_txclk_tapnum = DLL_TXCLK_TAPNUM_DEFAULT,
	.hs400_txclk_tapnum = 0x8,
};

static const struct sdhci_data rk3588_data = {
	.set_ios_post = rk3568_sdhci_set_ios_post,
	.set_clock = rk3568_sdhci_set_clock,
	.config_dll = rk3568_sdhci_config_dll,
	.hs200_txclk_tapnum = DLL_TXCLK_TAPNUM_DEFAULT,
	.hs400_txclk_tapnum = 0x9,
};

static const struct udevice_id sdhci_ids[] = {
	{
		.compatible = "arasan,sdhci-5.1",
		.data = (ulong)&rk3399_data,
	},
	{
		.compatible = "rockchip,rk3568-dwcmshc",
		.data = (ulong)&rk3568_data,
	},
	{
		.compatible = "rockchip,rk3588-dwcmshc",
		.data = (ulong)&rk3588_data,
	},
	{ }
};

U_BOOT_DRIVER(arasan_sdhci_drv) = {
	.name		= "rockchip_sdhci_5_1",
	.id		= UCLASS_MMC,
	.of_match	= sdhci_ids,
	.of_to_plat	= rockchip_sdhci_of_to_plat,
	.ops		= &sdhci_ops,
	.bind		= rockchip_sdhci_bind,
	.probe		= rockchip_sdhci_probe,
	.priv_auto	= sizeof(struct rockchip_sdhc),
	.plat_auto	= sizeof(struct rockchip_sdhc_plat),
};
