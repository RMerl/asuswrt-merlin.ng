// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Aaron <leafy.myeh@allwinnertech.com>
 *
 * MMC driver for allwinner sunxi platform.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <mmc.h>
#include <clk.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/mmc.h>
#include <asm-generic/gpio.h>

#ifdef CONFIG_DM_MMC
struct sunxi_mmc_variant {
	u16 mclk_offset;
};
#endif

struct sunxi_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct sunxi_mmc_priv {
	unsigned mmc_no;
	uint32_t *mclkreg;
	unsigned fatal_err;
	struct gpio_desc cd_gpio;	/* Change Detect GPIO */
	int cd_inverted;		/* Inverted Card Detect */
	struct sunxi_mmc *reg;
	struct mmc_config cfg;
#ifdef CONFIG_DM_MMC
	const struct sunxi_mmc_variant *variant;
#endif
};

#if !CONFIG_IS_ENABLED(DM_MMC)
/* support 4 mmc hosts */
struct sunxi_mmc_priv mmc_host[4];

static int sunxi_mmc_getcd_gpio(int sdc_no)
{
	switch (sdc_no) {
	case 0: return sunxi_name_to_gpio(CONFIG_MMC0_CD_PIN);
	case 1: return sunxi_name_to_gpio(CONFIG_MMC1_CD_PIN);
	case 2: return sunxi_name_to_gpio(CONFIG_MMC2_CD_PIN);
	case 3: return sunxi_name_to_gpio(CONFIG_MMC3_CD_PIN);
	}
	return -EINVAL;
}

static int mmc_resource_init(int sdc_no)
{
	struct sunxi_mmc_priv *priv = &mmc_host[sdc_no];
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int cd_pin, ret = 0;

	debug("init mmc %d resource\n", sdc_no);

	switch (sdc_no) {
	case 0:
		priv->reg = (struct sunxi_mmc *)SUNXI_MMC0_BASE;
		priv->mclkreg = &ccm->sd0_clk_cfg;
		break;
	case 1:
		priv->reg = (struct sunxi_mmc *)SUNXI_MMC1_BASE;
		priv->mclkreg = &ccm->sd1_clk_cfg;
		break;
	case 2:
		priv->reg = (struct sunxi_mmc *)SUNXI_MMC2_BASE;
		priv->mclkreg = &ccm->sd2_clk_cfg;
		break;
#ifdef SUNXI_MMC3_BASE
	case 3:
		priv->reg = (struct sunxi_mmc *)SUNXI_MMC3_BASE;
		priv->mclkreg = &ccm->sd3_clk_cfg;
		break;
#endif
	default:
		printf("Wrong mmc number %d\n", sdc_no);
		return -1;
	}
	priv->mmc_no = sdc_no;

	cd_pin = sunxi_mmc_getcd_gpio(sdc_no);
	if (cd_pin >= 0) {
		ret = gpio_request(cd_pin, "mmc_cd");
		if (!ret) {
			sunxi_gpio_set_pull(cd_pin, SUNXI_GPIO_PULL_UP);
			ret = gpio_direction_input(cd_pin);
		}
	}

	return ret;
}
#endif

static int mmc_set_mod_clk(struct sunxi_mmc_priv *priv, unsigned int hz)
{
	unsigned int pll, pll_hz, div, n, oclk_dly, sclk_dly;
	bool new_mode = true;
	bool calibrate = false;
	u32 val = 0;

	if (!IS_ENABLED(CONFIG_MMC_SUNXI_HAS_NEW_MODE))
		new_mode = false;

	/* A83T support new mode only on eMMC */
	if (IS_ENABLED(CONFIG_MACH_SUN8I_A83T) && priv->mmc_no != 2)
		new_mode = false;

#if defined(CONFIG_MACH_SUN50I) || defined(CONFIG_MACH_SUN50I_H6)
	calibrate = true;
#endif

	if (hz <= 24000000) {
		pll = CCM_MMC_CTRL_OSCM24;
		pll_hz = 24000000;
	} else {
#ifdef CONFIG_MACH_SUN9I
		pll = CCM_MMC_CTRL_PLL_PERIPH0;
		pll_hz = clock_get_pll4_periph0();
#elif defined(CONFIG_MACH_SUN50I_H6)
		pll = CCM_MMC_CTRL_PLL6X2;
		pll_hz = clock_get_pll6() * 2;
#else
		pll = CCM_MMC_CTRL_PLL6;
		pll_hz = clock_get_pll6();
#endif
	}

	div = pll_hz / hz;
	if (pll_hz % hz)
		div++;

	n = 0;
	while (div > 16) {
		n++;
		div = (div + 1) / 2;
	}

	if (n > 3) {
		printf("mmc %u error cannot set clock to %u\n", priv->mmc_no,
		       hz);
		return -1;
	}

	/* determine delays */
	if (hz <= 400000) {
		oclk_dly = 0;
		sclk_dly = 0;
	} else if (hz <= 25000000) {
		oclk_dly = 0;
		sclk_dly = 5;
#ifdef CONFIG_MACH_SUN9I
	} else if (hz <= 52000000) {
		oclk_dly = 5;
		sclk_dly = 4;
	} else {
		/* hz > 52000000 */
		oclk_dly = 2;
		sclk_dly = 4;
#else
	} else if (hz <= 52000000) {
		oclk_dly = 3;
		sclk_dly = 4;
	} else {
		/* hz > 52000000 */
		oclk_dly = 1;
		sclk_dly = 4;
#endif
	}

	if (new_mode) {
#ifdef CONFIG_MMC_SUNXI_HAS_NEW_MODE
#ifdef CONFIG_MMC_SUNXI_HAS_MODE_SWITCH
		val = CCM_MMC_CTRL_MODE_SEL_NEW;
#endif
		setbits_le32(&priv->reg->ntsr, SUNXI_MMC_NTSR_MODE_SEL_NEW);
#endif
	} else if (!calibrate) {
		/*
		 * Use hardcoded delay values if controller doesn't support
		 * calibration
		 */
		val = CCM_MMC_CTRL_OCLK_DLY(oclk_dly) |
			CCM_MMC_CTRL_SCLK_DLY(sclk_dly);
	}

	writel(CCM_MMC_CTRL_ENABLE| pll | CCM_MMC_CTRL_N(n) |
	       CCM_MMC_CTRL_M(div) | val, priv->mclkreg);

	debug("mmc %u set mod-clk req %u parent %u n %u m %u rate %u\n",
	      priv->mmc_no, hz, pll_hz, 1u << n, div, pll_hz / (1u << n) / div);

	return 0;
}

static int mmc_update_clk(struct sunxi_mmc_priv *priv)
{
	unsigned int cmd;
	unsigned timeout_msecs = 2000;
	unsigned long start = get_timer(0);

	cmd = SUNXI_MMC_CMD_START |
	      SUNXI_MMC_CMD_UPCLK_ONLY |
	      SUNXI_MMC_CMD_WAIT_PRE_OVER;

	writel(cmd, &priv->reg->cmd);
	while (readl(&priv->reg->cmd) & SUNXI_MMC_CMD_START) {
		if (get_timer(start) > timeout_msecs)
			return -1;
	}

	/* clock update sets various irq status bits, clear these */
	writel(readl(&priv->reg->rint), &priv->reg->rint);

	return 0;
}

static int mmc_config_clock(struct sunxi_mmc_priv *priv, struct mmc *mmc)
{
	unsigned rval = readl(&priv->reg->clkcr);

	/* Disable Clock */
	rval &= ~SUNXI_MMC_CLK_ENABLE;
	writel(rval, &priv->reg->clkcr);
	if (mmc_update_clk(priv))
		return -1;

	/* Set mod_clk to new rate */
	if (mmc_set_mod_clk(priv, mmc->clock))
		return -1;

	/* Clear internal divider */
	rval &= ~SUNXI_MMC_CLK_DIVIDER_MASK;
	writel(rval, &priv->reg->clkcr);

#if defined(CONFIG_MACH_SUN50I) || defined(CONFIG_MACH_SUN50I_H6)
	/* A64 supports calibration of delays on MMC controller and we
	 * have to set delay of zero before starting calibration.
	 * Allwinner BSP driver sets a delay only in the case of
	 * using HS400 which is not supported by mainline U-Boot or
	 * Linux at the moment
	 */
	writel(SUNXI_MMC_CAL_DL_SW_EN, &priv->reg->samp_dl);
#endif

	/* Re-enable Clock */
	rval |= SUNXI_MMC_CLK_ENABLE;
	writel(rval, &priv->reg->clkcr);
	if (mmc_update_clk(priv))
		return -1;

	return 0;
}

static int sunxi_mmc_set_ios_common(struct sunxi_mmc_priv *priv,
				    struct mmc *mmc)
{
	debug("set ios: bus_width: %x, clock: %d\n",
	      mmc->bus_width, mmc->clock);

	/* Change clock first */
	if (mmc->clock && mmc_config_clock(priv, mmc) != 0) {
		priv->fatal_err = 1;
		return -EINVAL;
	}

	/* Change bus width */
	if (mmc->bus_width == 8)
		writel(0x2, &priv->reg->width);
	else if (mmc->bus_width == 4)
		writel(0x1, &priv->reg->width);
	else
		writel(0x0, &priv->reg->width);

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static int sunxi_mmc_core_init(struct mmc *mmc)
{
	struct sunxi_mmc_priv *priv = mmc->priv;

	/* Reset controller */
	writel(SUNXI_MMC_GCTRL_RESET, &priv->reg->gctrl);
	udelay(1000);

	return 0;
}
#endif

static int mmc_trans_data_by_cpu(struct sunxi_mmc_priv *priv, struct mmc *mmc,
				 struct mmc_data *data)
{
	const int reading = !!(data->flags & MMC_DATA_READ);
	const uint32_t status_bit = reading ? SUNXI_MMC_STATUS_FIFO_EMPTY :
					      SUNXI_MMC_STATUS_FIFO_FULL;
	unsigned i;
	unsigned *buff = (unsigned int *)(reading ? data->dest : data->src);
	unsigned byte_cnt = data->blocksize * data->blocks;
	unsigned timeout_msecs = byte_cnt >> 8;
	unsigned long  start;

	if (timeout_msecs < 2000)
		timeout_msecs = 2000;

	/* Always read / write data through the CPU */
	setbits_le32(&priv->reg->gctrl, SUNXI_MMC_GCTRL_ACCESS_BY_AHB);

	start = get_timer(0);

	for (i = 0; i < (byte_cnt >> 2); i++) {
		while (readl(&priv->reg->status) & status_bit) {
			if (get_timer(start) > timeout_msecs)
				return -1;
		}

		if (reading)
			buff[i] = readl(&priv->reg->fifo);
		else
			writel(buff[i], &priv->reg->fifo);
	}

	return 0;
}

static int mmc_rint_wait(struct sunxi_mmc_priv *priv, struct mmc *mmc,
			 uint timeout_msecs, uint done_bit, const char *what)
{
	unsigned int status;
	unsigned long start = get_timer(0);

	do {
		status = readl(&priv->reg->rint);
		if ((get_timer(start) > timeout_msecs) ||
		    (status & SUNXI_MMC_RINT_INTERRUPT_ERROR_BIT)) {
			debug("%s timeout %x\n", what,
			      status & SUNXI_MMC_RINT_INTERRUPT_ERROR_BIT);
			return -ETIMEDOUT;
		}
	} while (!(status & done_bit));

	return 0;
}

static int sunxi_mmc_send_cmd_common(struct sunxi_mmc_priv *priv,
				     struct mmc *mmc, struct mmc_cmd *cmd,
				     struct mmc_data *data)
{
	unsigned int cmdval = SUNXI_MMC_CMD_START;
	unsigned int timeout_msecs;
	int error = 0;
	unsigned int status = 0;
	unsigned int bytecnt = 0;

	if (priv->fatal_err)
		return -1;
	if (cmd->resp_type & MMC_RSP_BUSY)
		debug("mmc cmd %d check rsp busy\n", cmd->cmdidx);
	if (cmd->cmdidx == 12)
		return 0;

	if (!cmd->cmdidx)
		cmdval |= SUNXI_MMC_CMD_SEND_INIT_SEQ;
	if (cmd->resp_type & MMC_RSP_PRESENT)
		cmdval |= SUNXI_MMC_CMD_RESP_EXPIRE;
	if (cmd->resp_type & MMC_RSP_136)
		cmdval |= SUNXI_MMC_CMD_LONG_RESPONSE;
	if (cmd->resp_type & MMC_RSP_CRC)
		cmdval |= SUNXI_MMC_CMD_CHK_RESPONSE_CRC;

	if (data) {
		if ((u32)(long)data->dest & 0x3) {
			error = -1;
			goto out;
		}

		cmdval |= SUNXI_MMC_CMD_DATA_EXPIRE|SUNXI_MMC_CMD_WAIT_PRE_OVER;
		if (data->flags & MMC_DATA_WRITE)
			cmdval |= SUNXI_MMC_CMD_WRITE;
		if (data->blocks > 1)
			cmdval |= SUNXI_MMC_CMD_AUTO_STOP;
		writel(data->blocksize, &priv->reg->blksz);
		writel(data->blocks * data->blocksize, &priv->reg->bytecnt);
	}

	debug("mmc %d, cmd %d(0x%08x), arg 0x%08x\n", priv->mmc_no,
	      cmd->cmdidx, cmdval | cmd->cmdidx, cmd->cmdarg);
	writel(cmd->cmdarg, &priv->reg->arg);

	if (!data)
		writel(cmdval | cmd->cmdidx, &priv->reg->cmd);

	/*
	 * transfer data and check status
	 * STATREG[2] : FIFO empty
	 * STATREG[3] : FIFO full
	 */
	if (data) {
		int ret = 0;

		bytecnt = data->blocksize * data->blocks;
		debug("trans data %d bytes\n", bytecnt);
		writel(cmdval | cmd->cmdidx, &priv->reg->cmd);
		ret = mmc_trans_data_by_cpu(priv, mmc, data);
		if (ret) {
			error = readl(&priv->reg->rint) &
				SUNXI_MMC_RINT_INTERRUPT_ERROR_BIT;
			error = -ETIMEDOUT;
			goto out;
		}
	}

	error = mmc_rint_wait(priv, mmc, 1000, SUNXI_MMC_RINT_COMMAND_DONE,
			      "cmd");
	if (error)
		goto out;

	if (data) {
		timeout_msecs = 120;
		debug("cacl timeout %x msec\n", timeout_msecs);
		error = mmc_rint_wait(priv, mmc, timeout_msecs,
				      data->blocks > 1 ?
				      SUNXI_MMC_RINT_AUTO_COMMAND_DONE :
				      SUNXI_MMC_RINT_DATA_OVER,
				      "data");
		if (error)
			goto out;
	}

	if (cmd->resp_type & MMC_RSP_BUSY) {
		unsigned long start = get_timer(0);
		timeout_msecs = 2000;

		do {
			status = readl(&priv->reg->status);
			if (get_timer(start) > timeout_msecs) {
				debug("busy timeout\n");
				error = -ETIMEDOUT;
				goto out;
			}
		} while (status & SUNXI_MMC_STATUS_CARD_DATA_BUSY);
	}

	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = readl(&priv->reg->resp3);
		cmd->response[1] = readl(&priv->reg->resp2);
		cmd->response[2] = readl(&priv->reg->resp1);
		cmd->response[3] = readl(&priv->reg->resp0);
		debug("mmc resp 0x%08x 0x%08x 0x%08x 0x%08x\n",
		      cmd->response[3], cmd->response[2],
		      cmd->response[1], cmd->response[0]);
	} else {
		cmd->response[0] = readl(&priv->reg->resp0);
		debug("mmc resp 0x%08x\n", cmd->response[0]);
	}
out:
	if (error < 0) {
		writel(SUNXI_MMC_GCTRL_RESET, &priv->reg->gctrl);
		mmc_update_clk(priv);
	}
	writel(0xffffffff, &priv->reg->rint);
	writel(readl(&priv->reg->gctrl) | SUNXI_MMC_GCTRL_FIFO_RESET,
	       &priv->reg->gctrl);

	return error;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static int sunxi_mmc_set_ios_legacy(struct mmc *mmc)
{
	struct sunxi_mmc_priv *priv = mmc->priv;

	return sunxi_mmc_set_ios_common(priv, mmc);
}

static int sunxi_mmc_send_cmd_legacy(struct mmc *mmc, struct mmc_cmd *cmd,
				     struct mmc_data *data)
{
	struct sunxi_mmc_priv *priv = mmc->priv;

	return sunxi_mmc_send_cmd_common(priv, mmc, cmd, data);
}

static int sunxi_mmc_getcd_legacy(struct mmc *mmc)
{
	struct sunxi_mmc_priv *priv = mmc->priv;
	int cd_pin;

	cd_pin = sunxi_mmc_getcd_gpio(priv->mmc_no);
	if (cd_pin < 0)
		return 1;

	return !gpio_get_value(cd_pin);
}

static const struct mmc_ops sunxi_mmc_ops = {
	.send_cmd	= sunxi_mmc_send_cmd_legacy,
	.set_ios	= sunxi_mmc_set_ios_legacy,
	.init		= sunxi_mmc_core_init,
	.getcd		= sunxi_mmc_getcd_legacy,
};

struct mmc *sunxi_mmc_init(int sdc_no)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_mmc_priv *priv = &mmc_host[sdc_no];
	struct mmc_config *cfg = &priv->cfg;
	int ret;

	memset(priv, '\0', sizeof(struct sunxi_mmc_priv));

	cfg->name = "SUNXI SD/MMC";
	cfg->ops  = &sunxi_mmc_ops;

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	cfg->host_caps = MMC_MODE_4BIT;
#if defined(CONFIG_MACH_SUN50I) || defined(CONFIG_MACH_SUN8I) || defined(CONFIG_MACH_SUN50I_H6)
	if (sdc_no == 2)
		cfg->host_caps = MMC_MODE_8BIT;
#endif
	cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	cfg->f_min = 400000;
	cfg->f_max = 52000000;

	if (mmc_resource_init(sdc_no) != 0)
		return NULL;

	/* config ahb clock */
	debug("init mmc %d clock and io\n", sdc_no);
#if !defined(CONFIG_MACH_SUN50I_H6)
	setbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MMC(sdc_no));

#ifdef CONFIG_SUNXI_GEN_SUN6I
	/* unassert reset */
	setbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_MMC(sdc_no));
#endif
#if defined(CONFIG_MACH_SUN9I)
	/* sun9i has a mmc-common module, also set the gate and reset there */
	writel(SUNXI_MMC_COMMON_CLK_GATE | SUNXI_MMC_COMMON_RESET,
	       SUNXI_MMC_COMMON_BASE + 4 * sdc_no);
#endif
#else /* CONFIG_MACH_SUN50I_H6 */
	setbits_le32(&ccm->sd_gate_reset, 1 << sdc_no);
	/* unassert reset */
	setbits_le32(&ccm->sd_gate_reset, 1 << (RESET_SHIFT + sdc_no));
#endif
	ret = mmc_set_mod_clk(priv, 24000000);
	if (ret)
		return NULL;

	return mmc_create(cfg, priv);
}
#else

static int sunxi_mmc_set_ios(struct udevice *dev)
{
	struct sunxi_mmc_plat *plat = dev_get_platdata(dev);
	struct sunxi_mmc_priv *priv = dev_get_priv(dev);

	return sunxi_mmc_set_ios_common(priv, &plat->mmc);
}

static int sunxi_mmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct sunxi_mmc_plat *plat = dev_get_platdata(dev);
	struct sunxi_mmc_priv *priv = dev_get_priv(dev);

	return sunxi_mmc_send_cmd_common(priv, &plat->mmc, cmd, data);
}

static int sunxi_mmc_getcd(struct udevice *dev)
{
	struct sunxi_mmc_priv *priv = dev_get_priv(dev);

	if (dm_gpio_is_valid(&priv->cd_gpio)) {
		int cd_state = dm_gpio_get_value(&priv->cd_gpio);

		return cd_state ^ priv->cd_inverted;
	}
	return 1;
}

static const struct dm_mmc_ops sunxi_mmc_ops = {
	.send_cmd	= sunxi_mmc_send_cmd,
	.set_ios	= sunxi_mmc_set_ios,
	.get_cd		= sunxi_mmc_getcd,
};

static int sunxi_mmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sunxi_mmc_plat *plat = dev_get_platdata(dev);
	struct sunxi_mmc_priv *priv = dev_get_priv(dev);
	struct reset_ctl_bulk reset_bulk;
	struct clk gate_clk;
	struct mmc_config *cfg = &plat->cfg;
	struct ofnode_phandle_args args;
	u32 *ccu_reg;
	int bus_width, ret;

	cfg->name = dev->name;
	bus_width = dev_read_u32_default(dev, "bus-width", 1);

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	cfg->host_caps = 0;
	if (bus_width == 8)
		cfg->host_caps |= MMC_MODE_8BIT;
	if (bus_width >= 4)
		cfg->host_caps |= MMC_MODE_4BIT;
	cfg->host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	cfg->f_min = 400000;
	cfg->f_max = 52000000;

	priv->reg = (void *)dev_read_addr(dev);
	priv->variant =
		(const struct sunxi_mmc_variant *)dev_get_driver_data(dev);

	/* We don't have a sunxi clock driver so find the clock address here */
	ret = dev_read_phandle_with_args(dev, "clocks", "#clock-cells", 0,
					  1, &args);
	if (ret)
		return ret;
	ccu_reg = (u32 *)ofnode_get_addr(args.node);

	priv->mmc_no = ((uintptr_t)priv->reg - SUNXI_MMC0_BASE) / 0x1000;
	priv->mclkreg = (void *)ccu_reg +
			(priv->variant->mclk_offset + (priv->mmc_no * 4));

	ret = clk_get_by_name(dev, "ahb", &gate_clk);
	if (!ret)
		clk_enable(&gate_clk);

	ret = reset_get_bulk(dev, &reset_bulk);
	if (!ret)
		reset_deassert_bulk(&reset_bulk);

	ret = mmc_set_mod_clk(priv, 24000000);
	if (ret)
		return ret;

	/* This GPIO is optional */
	if (!dev_read_bool(dev, "non-removable") &&
	    !gpio_request_by_name(dev, "cd-gpios", 0, &priv->cd_gpio,
				  GPIOD_IS_IN)) {
		int cd_pin = gpio_get_number(&priv->cd_gpio);

		sunxi_gpio_set_pull(cd_pin, SUNXI_GPIO_PULL_UP);
	}

	/* Check if card detect is inverted */
	priv->cd_inverted = dev_read_bool(dev, "cd-inverted");

	upriv->mmc = &plat->mmc;

	/* Reset controller */
	writel(SUNXI_MMC_GCTRL_RESET, &priv->reg->gctrl);
	udelay(1000);

	return 0;
}

static int sunxi_mmc_bind(struct udevice *dev)
{
	struct sunxi_mmc_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct sunxi_mmc_variant sun4i_a10_variant = {
	.mclk_offset = 0x88,
};

static const struct sunxi_mmc_variant sun9i_a80_variant = {
	.mclk_offset = 0x410,
};

static const struct sunxi_mmc_variant sun50i_h6_variant = {
	.mclk_offset = 0x830,
};

static const struct udevice_id sunxi_mmc_ids[] = {
	{
	  .compatible = "allwinner,sun4i-a10-mmc",
	  .data = (ulong)&sun4i_a10_variant,
	},
	{
	  .compatible = "allwinner,sun5i-a13-mmc",
	  .data = (ulong)&sun4i_a10_variant,
	},
	{
	  .compatible = "allwinner,sun7i-a20-mmc",
	  .data = (ulong)&sun4i_a10_variant,
	},
	{
	  .compatible = "allwinner,sun8i-a83t-emmc",
	  .data = (ulong)&sun4i_a10_variant,
	},
	{
	  .compatible = "allwinner,sun9i-a80-mmc",
	  .data = (ulong)&sun9i_a80_variant,
	},
	{
	  .compatible = "allwinner,sun50i-a64-mmc",
	  .data = (ulong)&sun4i_a10_variant,
	},
	{
	  .compatible = "allwinner,sun50i-a64-emmc",
	  .data = (ulong)&sun4i_a10_variant,
	},
	{
	  .compatible = "allwinner,sun50i-h6-mmc",
	  .data = (ulong)&sun50i_h6_variant,
	},
	{
	  .compatible = "allwinner,sun50i-h6-emmc",
	  .data = (ulong)&sun50i_h6_variant,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sunxi_mmc_drv) = {
	.name		= "sunxi_mmc",
	.id		= UCLASS_MMC,
	.of_match	= sunxi_mmc_ids,
	.bind		= sunxi_mmc_bind,
	.probe		= sunxi_mmc_probe,
	.ops		= &sunxi_mmc_ops,
	.platdata_auto_alloc_size = sizeof(struct sunxi_mmc_plat),
	.priv_auto_alloc_size = sizeof(struct sunxi_mmc_priv),
};
#endif
