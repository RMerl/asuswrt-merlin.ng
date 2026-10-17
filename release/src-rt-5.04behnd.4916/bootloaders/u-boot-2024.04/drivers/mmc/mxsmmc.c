// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 SSP MMC driver
 *
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * (C) Copyright 2008-2010 Freescale Semiconductor, Inc.
 * Terry Lv
 *
 * Copyright 2007, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the pxa mmc code:
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <mmc.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/dma.h>
#include <bouncebuf.h>

#define	MXSMMC_MAX_TIMEOUT	10000
#define MXSMMC_SMALL_TRANSFER	512

#if !CONFIG_IS_ENABLED(DM_MMC)
struct mxsmmc_priv {
	int			id;
	int			(*mmc_is_wp)(int);
	int			(*mmc_cd)(int);
	struct mmc_config	cfg;	/* mmc configuration */
	struct mxs_dma_desc	*desc;
	uint32_t		buswidth;
	struct mxs_ssp_regs	*regs;
};
#else /* CONFIG_IS_ENABLED(DM_MMC) */
#include <dm/device.h>
#include <dm/read.h>
#include <dt-structs.h>

struct mxsmmc_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_fsl_imx23_mmc dtplat;
#endif
	struct mmc_config cfg;
	struct mmc mmc;
	fdt_addr_t base;
	int non_removable;
	int buswidth;
	int dma_id;
	int clk_id;
};

struct mxsmmc_priv {
	int clkid;
	struct mxs_dma_desc	*desc;
	u32			buswidth;
	struct mxs_ssp_regs	*regs;
	unsigned int            dma_channel;
};
#endif

#if !CONFIG_IS_ENABLED(DM_MMC)
static int mxsmmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			   struct mmc_data *data);

static int mxsmmc_cd(struct mxsmmc_priv *priv)
{
	struct mxs_ssp_regs *ssp_regs = priv->regs;

	if (priv->mmc_cd)
		return priv->mmc_cd(priv->id);

	return !(readl(&ssp_regs->hw_ssp_status) & SSP_STATUS_CARD_DETECT);
}

static int mxsmmc_set_ios(struct mmc *mmc)
{
	struct mxsmmc_priv *priv = mmc->priv;
	struct mxs_ssp_regs *ssp_regs = priv->regs;

	/* Set the clock speed */
	if (mmc->clock)
		mxs_set_ssp_busclock(priv->id, mmc->clock / 1000);

	switch (mmc->bus_width) {
	case 1:
		priv->buswidth = SSP_CTRL0_BUS_WIDTH_ONE_BIT;
		break;
	case 4:
		priv->buswidth = SSP_CTRL0_BUS_WIDTH_FOUR_BIT;
		break;
	case 8:
		priv->buswidth = SSP_CTRL0_BUS_WIDTH_EIGHT_BIT;
		break;
	}

	/* Set the bus width */
	clrsetbits_le32(&ssp_regs->hw_ssp_ctrl0,
			SSP_CTRL0_BUS_WIDTH_MASK, priv->buswidth);

	debug("MMC%d: Set %d bits bus width\n",
	      mmc->block_dev.devnum, mmc->bus_width);

	return 0;
}

static int mxsmmc_init(struct mmc *mmc)
{
	struct mxsmmc_priv *priv = mmc->priv;
	struct mxs_ssp_regs *ssp_regs = priv->regs;

	/* Reset SSP */
	mxs_reset_block(&ssp_regs->hw_ssp_ctrl0_reg);

	/* Reconfigure the SSP block for MMC operation */
	writel(SSP_CTRL1_SSP_MODE_SD_MMC |
		SSP_CTRL1_WORD_LENGTH_EIGHT_BITS |
		SSP_CTRL1_DMA_ENABLE |
		SSP_CTRL1_POLARITY |
		SSP_CTRL1_RECV_TIMEOUT_IRQ_EN |
		SSP_CTRL1_DATA_CRC_IRQ_EN |
		SSP_CTRL1_DATA_TIMEOUT_IRQ_EN |
		SSP_CTRL1_RESP_TIMEOUT_IRQ_EN |
		SSP_CTRL1_RESP_ERR_IRQ_EN,
		&ssp_regs->hw_ssp_ctrl1_set);

	/* Set initial bit clock 400 KHz */
	mxs_set_ssp_busclock(priv->id, 400);

	/* Send initial 74 clock cycles (185 us @ 400 KHz)*/
	writel(SSP_CMD0_CONT_CLKING_EN, &ssp_regs->hw_ssp_cmd0_set);
	udelay(200);
	writel(SSP_CMD0_CONT_CLKING_EN, &ssp_regs->hw_ssp_cmd0_clr);

	return 0;
}

static const struct mmc_ops mxsmmc_ops = {
	.send_cmd	= mxsmmc_send_cmd,
	.set_ios	= mxsmmc_set_ios,
	.init		= mxsmmc_init,
};

int mxsmmc_initialize(struct bd_info *bis, int id, int (*wp)(int),
		      int (*cd)(int))
{
	struct mmc *mmc = NULL;
	struct mxsmmc_priv *priv = NULL;
	int ret;
	const unsigned int mxsmmc_clk_id = mxs_ssp_clock_by_bus(id);

	if (!mxs_ssp_bus_id_valid(id))
		return -ENODEV;

	priv = malloc(sizeof(struct mxsmmc_priv));
	if (!priv)
		return -ENOMEM;

	priv->desc = mxs_dma_desc_alloc();
	if (!priv->desc) {
		free(priv);
		return -ENOMEM;
	}

	ret = mxs_dma_init_channel(MXS_DMA_CHANNEL_AHB_APBH_SSP0 + id);
	if (ret)
		return ret;

	priv->mmc_is_wp = wp;
	priv->mmc_cd = cd;
	priv->id = id;
	priv->regs = mxs_ssp_regs_by_bus(id);

	priv->cfg.name = "MXS MMC";
	priv->cfg.ops = &mxsmmc_ops;

	priv->cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34;

	priv->cfg.host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT |
			 MMC_MODE_HS_52MHz | MMC_MODE_HS;

	/*
	 * SSPCLK = 480 * 18 / 29 / 1 = 297.731 MHz
	 * SSP bit rate = SSPCLK / (CLOCK_DIVIDE * (1 + CLOCK_RATE)),
	 * CLOCK_DIVIDE has to be an even value from 2 to 254, and
	 * CLOCK_RATE could be any integer from 0 to 255.
	 */
	priv->cfg.f_min = 400000;
	priv->cfg.f_max = mxc_get_clock(MXC_SSP0_CLK + mxsmmc_clk_id)
		* 1000 / 2;
	priv->cfg.b_max = 0x20;

	mmc = mmc_create(&priv->cfg, priv);
	if (!mmc) {
		mxs_dma_desc_free(priv->desc);
		free(priv);
		return -ENOMEM;
	}
	return 0;
}
#endif /* CONFIG_IS_ENABLED(DM_MMC) */

static int mxsmmc_send_cmd_pio(struct mxsmmc_priv *priv, struct mmc_data *data)
{
	struct mxs_ssp_regs *ssp_regs = priv->regs;
	uint32_t *data_ptr;
	int timeout = MXSMMC_MAX_TIMEOUT;
	uint32_t reg;
	uint32_t data_count = data->blocksize * data->blocks;

	if (data->flags & MMC_DATA_READ) {
		data_ptr = (uint32_t *)data->dest;
		while (data_count && --timeout) {
			reg = readl(&ssp_regs->hw_ssp_status);
			if (!(reg & SSP_STATUS_FIFO_EMPTY)) {
				*data_ptr++ = readl(&ssp_regs->hw_ssp_data);
				data_count -= 4;
				timeout = MXSMMC_MAX_TIMEOUT;
			} else
				udelay(1000);
		}
	} else {
		data_ptr = (uint32_t *)data->src;
		timeout *= 100;
		while (data_count && --timeout) {
			reg = readl(&ssp_regs->hw_ssp_status);
			if (!(reg & SSP_STATUS_FIFO_FULL)) {
				writel(*data_ptr++, &ssp_regs->hw_ssp_data);
				data_count -= 4;
				timeout = MXSMMC_MAX_TIMEOUT;
			} else
				udelay(1000);
		}
	}

	return timeout ? 0 : -ECOMM;
}

static int mxsmmc_send_cmd_dma(struct mxsmmc_priv *priv, struct mmc_data *data)
{
	uint32_t data_count = data->blocksize * data->blocks;
	int dmach;
	struct mxs_dma_desc *desc = priv->desc;
	void *addr;
	unsigned int flags;
	struct bounce_buffer bbstate;

	memset(desc, 0, sizeof(struct mxs_dma_desc));
	desc->address = (dma_addr_t)desc;

	if (data->flags & MMC_DATA_READ) {
		priv->desc->cmd.data = MXS_DMA_DESC_COMMAND_DMA_WRITE;
		addr = data->dest;
		flags = GEN_BB_WRITE;
	} else {
		priv->desc->cmd.data = MXS_DMA_DESC_COMMAND_DMA_READ;
		addr = (void *)data->src;
		flags = GEN_BB_READ;
	}

	bounce_buffer_start(&bbstate, addr, data_count, flags);

	priv->desc->cmd.address = (dma_addr_t)bbstate.bounce_buffer;

	priv->desc->cmd.data |= MXS_DMA_DESC_IRQ | MXS_DMA_DESC_DEC_SEM |
				(data_count << MXS_DMA_DESC_BYTES_OFFSET);

#if !CONFIG_IS_ENABLED(DM_MMC)
	dmach = MXS_DMA_CHANNEL_AHB_APBH_SSP0 + priv->id;
#else
	dmach = priv->dma_channel;
#endif
	mxs_dma_desc_append(dmach, priv->desc);
	if (mxs_dma_go(dmach)) {
		bounce_buffer_stop(&bbstate);
		return -ECOMM;
	}

	bounce_buffer_stop(&bbstate);

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
/*
 * Sends a command out on the bus.  Takes the mmc pointer,
 * a command pointer, and an optional data pointer.
 */
static int
mxsmmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	struct mxsmmc_priv *priv = mmc->priv;
	struct mxs_ssp_regs *ssp_regs = priv->regs;
#else
static int
mxsmmc_send_cmd(struct udevice *dev, struct mmc_cmd *cmd, struct mmc_data *data)
{
	struct mxsmmc_plat *plat = dev_get_plat(dev);
	struct mxsmmc_priv *priv = dev_get_priv(dev);
	struct mxs_ssp_regs *ssp_regs = priv->regs;
	struct mmc *mmc = &plat->mmc;
#endif
	uint32_t reg;
	int timeout;
	uint32_t ctrl0;
	int ret;
#if !CONFIG_IS_ENABLED(DM_MMC)
	int devnum = mmc->block_dev.devnum;
#else
	int devnum = mmc_get_blk_desc(mmc)->devnum;
#endif
	debug("MMC%d: CMD%d\n", devnum, cmd->cmdidx);

	/* Check bus busy */
	timeout = MXSMMC_MAX_TIMEOUT;
	while (--timeout) {
		udelay(1000);
		reg = readl(&ssp_regs->hw_ssp_status);
		if (!(reg &
			(SSP_STATUS_BUSY | SSP_STATUS_DATA_BUSY |
			SSP_STATUS_CMD_BUSY))) {
			break;
		}
	}

	if (!timeout) {
		printf("MMC%d: Bus busy timeout!\n", devnum);
		return -ETIMEDOUT;
	}
#if !CONFIG_IS_ENABLED(DM_MMC)
	/* See if card is present */
	if (!mxsmmc_cd(priv)) {
		printf("MMC%d: No card detected!\n", devnum);
		return -ENOMEDIUM;
	}
#endif
	/* Start building CTRL0 contents */
	ctrl0 = priv->buswidth;

	/* Set up command */
	if (!(cmd->resp_type & MMC_RSP_CRC))
		ctrl0 |= SSP_CTRL0_IGNORE_CRC;
	if (cmd->resp_type & MMC_RSP_PRESENT)	/* Need to get response */
		ctrl0 |= SSP_CTRL0_GET_RESP;
	if (cmd->resp_type & MMC_RSP_136)	/* It's a 136 bits response */
		ctrl0 |= SSP_CTRL0_LONG_RESP;

	if (data && (data->blocksize * data->blocks < MXSMMC_SMALL_TRANSFER))
		writel(SSP_CTRL1_DMA_ENABLE, &ssp_regs->hw_ssp_ctrl1_clr);
	else
		writel(SSP_CTRL1_DMA_ENABLE, &ssp_regs->hw_ssp_ctrl1_set);

	/* Command index */
	reg = readl(&ssp_regs->hw_ssp_cmd0);
	reg &= ~(SSP_CMD0_CMD_MASK | SSP_CMD0_APPEND_8CYC);
	reg |= cmd->cmdidx << SSP_CMD0_CMD_OFFSET;
	if (cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION)
		reg |= SSP_CMD0_APPEND_8CYC;
	writel(reg, &ssp_regs->hw_ssp_cmd0);

	/* Command argument */
	writel(cmd->cmdarg, &ssp_regs->hw_ssp_cmd1);

	/* Set up data */
	if (data) {
		/* READ or WRITE */
		if (data->flags & MMC_DATA_READ) {
			ctrl0 |= SSP_CTRL0_READ;
#if !CONFIG_IS_ENABLED(DM_MMC)
		} else if (priv->mmc_is_wp &&
			priv->mmc_is_wp(devnum)) {
			printf("MMC%d: Can not write a locked card!\n", devnum);
			return -EOPNOTSUPP;
#endif
		}
		ctrl0 |= SSP_CTRL0_DATA_XFER;

		reg = data->blocksize * data->blocks;
#if defined(CONFIG_MX23)
		ctrl0 |= reg & SSP_CTRL0_XFER_COUNT_MASK;

		clrsetbits_le32(&ssp_regs->hw_ssp_cmd0,
			SSP_CMD0_BLOCK_SIZE_MASK | SSP_CMD0_BLOCK_COUNT_MASK,
			((data->blocks - 1) << SSP_CMD0_BLOCK_COUNT_OFFSET) |
			((ffs(data->blocksize) - 1) <<
				SSP_CMD0_BLOCK_SIZE_OFFSET));
#elif defined(CONFIG_MX28)
		writel(reg, &ssp_regs->hw_ssp_xfer_size);

		reg = ((data->blocks - 1) <<
			SSP_BLOCK_SIZE_BLOCK_COUNT_OFFSET) |
			((ffs(data->blocksize) - 1) <<
			SSP_BLOCK_SIZE_BLOCK_SIZE_OFFSET);
		writel(reg, &ssp_regs->hw_ssp_block_size);
#endif
	}

	/* Kick off the command */
	ctrl0 |= SSP_CTRL0_WAIT_FOR_IRQ | SSP_CTRL0_ENABLE | SSP_CTRL0_RUN;
	writel(ctrl0, &ssp_regs->hw_ssp_ctrl0);

	/* Wait for the command to complete */
	timeout = MXSMMC_MAX_TIMEOUT;
	while (--timeout) {
		udelay(1000);
		reg = readl(&ssp_regs->hw_ssp_status);
		if (!(reg & SSP_STATUS_CMD_BUSY))
			break;
	}

	if (!timeout) {
		printf("MMC%d: Command %d busy\n", devnum, cmd->cmdidx);
		return -ETIMEDOUT;
	}

	/* Check command timeout */
	if (reg & SSP_STATUS_RESP_TIMEOUT) {
		debug("MMC%d: Command %d timeout (status 0x%08x)\n",
		      devnum, cmd->cmdidx, reg);
		return -ETIMEDOUT;
	}

	/* Check command errors */
	if (reg & (SSP_STATUS_RESP_CRC_ERR | SSP_STATUS_RESP_ERR)) {
		printf("MMC%d: Command %d error (status 0x%08x)!\n",
		       devnum, cmd->cmdidx, reg);
		return -ECOMM;
	}

	/* Copy response to response buffer */
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[3] = readl(&ssp_regs->hw_ssp_sdresp0);
		cmd->response[2] = readl(&ssp_regs->hw_ssp_sdresp1);
		cmd->response[1] = readl(&ssp_regs->hw_ssp_sdresp2);
		cmd->response[0] = readl(&ssp_regs->hw_ssp_sdresp3);
	} else
		cmd->response[0] = readl(&ssp_regs->hw_ssp_sdresp0);

	/* Return if no data to process */
	if (!data)
		return 0;

	if (data->blocksize * data->blocks < MXSMMC_SMALL_TRANSFER) {
		ret = mxsmmc_send_cmd_pio(priv, data);
		if (ret) {
			printf("MMC%d: Data timeout with command %d "
				"(status 0x%08x)!\n", devnum, cmd->cmdidx, reg);
			return ret;
		}
	} else {
		ret = mxsmmc_send_cmd_dma(priv, data);
		if (ret) {
			printf("MMC%d: DMA transfer failed\n", devnum);
			return ret;
		}
	}

	/* Check data errors */
	reg = readl(&ssp_regs->hw_ssp_status);
	if (reg &
		(SSP_STATUS_TIMEOUT | SSP_STATUS_DATA_CRC_ERR |
		SSP_STATUS_FIFO_OVRFLW | SSP_STATUS_FIFO_UNDRFLW)) {
		printf("MMC%d: Data error with command %d (status 0x%08x)!\n",
		       devnum, cmd->cmdidx, reg);
		return -ECOMM;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(DM_MMC)
/* Base numbers of i.MX2[38] clk for ssp0 IP block */
#define MXS_SSP_IMX23_CLKID_SSP0 33
#define MXS_SSP_IMX28_CLKID_SSP0 46

static int mxsmmc_get_cd(struct udevice *dev)
{
	struct mxsmmc_plat *plat = dev_get_plat(dev);
	struct mxsmmc_priv *priv = dev_get_priv(dev);
	struct mxs_ssp_regs *ssp_regs = priv->regs;

	if (plat->non_removable)
		return 1;

	return !(readl(&ssp_regs->hw_ssp_status) & SSP_STATUS_CARD_DETECT);
}

static int mxsmmc_set_ios(struct udevice *dev)
{
	struct mxsmmc_plat *plat = dev_get_plat(dev);
	struct mxsmmc_priv *priv = dev_get_priv(dev);
	struct mxs_ssp_regs *ssp_regs = priv->regs;
	struct mmc *mmc = &plat->mmc;

	/* Set the clock speed */
	if (mmc->clock)
		mxs_set_ssp_busclock(priv->clkid, mmc->clock / 1000);

	switch (mmc->bus_width) {
	case 1:
		priv->buswidth = SSP_CTRL0_BUS_WIDTH_ONE_BIT;
		break;
	case 4:
		priv->buswidth = SSP_CTRL0_BUS_WIDTH_FOUR_BIT;
		break;
	case 8:
		priv->buswidth = SSP_CTRL0_BUS_WIDTH_EIGHT_BIT;
		break;
	}

	/* Set the bus width */
	clrsetbits_le32(&ssp_regs->hw_ssp_ctrl0,
			SSP_CTRL0_BUS_WIDTH_MASK, priv->buswidth);

	debug("MMC%d: Set %d bits bus width\n", mmc_get_blk_desc(mmc)->devnum,
	      mmc->bus_width);

	return 0;
}

static int mxsmmc_init(struct udevice *dev)
{
	struct mxsmmc_priv *priv = dev_get_priv(dev);
	struct mxs_ssp_regs *ssp_regs = priv->regs;

	/* Reset SSP */
	mxs_reset_block(&ssp_regs->hw_ssp_ctrl0_reg);

	/* Reconfigure the SSP block for MMC operation */
	writel(SSP_CTRL1_SSP_MODE_SD_MMC |
		SSP_CTRL1_WORD_LENGTH_EIGHT_BITS |
		SSP_CTRL1_DMA_ENABLE |
		SSP_CTRL1_POLARITY |
		SSP_CTRL1_RECV_TIMEOUT_IRQ_EN |
		SSP_CTRL1_DATA_CRC_IRQ_EN |
		SSP_CTRL1_DATA_TIMEOUT_IRQ_EN |
		SSP_CTRL1_RESP_TIMEOUT_IRQ_EN |
		SSP_CTRL1_RESP_ERR_IRQ_EN,
		&ssp_regs->hw_ssp_ctrl1_set);

	/* Set initial bit clock 400 KHz */
	mxs_set_ssp_busclock(priv->clkid, 400);

	/* Send initial 74 clock cycles (185 us @ 400 KHz)*/
	writel(SSP_CMD0_CONT_CLKING_EN, &ssp_regs->hw_ssp_cmd0_set);
	udelay(200);
	writel(SSP_CMD0_CONT_CLKING_EN, &ssp_regs->hw_ssp_cmd0_clr);

	return 0;
}

static int mxsmmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct mxsmmc_plat *plat = dev_get_plat(dev);
	struct mxsmmc_priv *priv = dev_get_priv(dev);
	struct blk_desc *bdesc;
	struct mmc *mmc;
	int ret, clkid;

	debug("%s: probe\n", __func__);

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_fsl_imx23_mmc *dtplat = &plat->dtplat;
	struct phandle_1_arg *p1a = &dtplat->clocks[0];

	priv->buswidth = dtplat->bus_width;
	priv->regs = (struct mxs_ssp_regs *)dtplat->reg[0];
	priv->dma_channel = dtplat->dmas[1];
	clkid = p1a->arg[0];
	plat->non_removable = dtplat->non_removable;

	debug("OF_PLATDATA: regs: 0x%p bw: %d clkid: %d non_removable: %d\n",
	      priv->regs, priv->buswidth, clkid, plat->non_removable);
#else
	priv->regs = (struct mxs_ssp_regs *)plat->base;
	priv->dma_channel = plat->dma_id;
	clkid = plat->clk_id;
#endif

#ifdef CONFIG_MX28
	priv->clkid = clkid - MXS_SSP_IMX28_CLKID_SSP0;
#else /* CONFIG_MX23 */
	priv->clkid = clkid - MXS_SSP_IMX23_CLKID_SSP0;
#endif
	mmc = &plat->mmc;
	mmc->cfg = &plat->cfg;
	mmc->dev = dev;

	priv->desc = mxs_dma_desc_alloc();
	if (!priv->desc) {
		printf("%s: Cannot allocate DMA descriptor\n", __func__);
		return -ENOMEM;
	}

	ret = mxs_dma_init_channel(priv->dma_channel);
	if (ret)
		return ret;

	plat->cfg.name = "MXS MMC";
	plat->cfg.voltages = MMC_VDD_32_33 | MMC_VDD_33_34;

	plat->cfg.host_caps = MMC_MODE_4BIT | MMC_MODE_8BIT |
		MMC_MODE_HS_52MHz | MMC_MODE_HS;

	/*
	 * SSPCLK = 480 * 18 / 29 / 1 = 297.731 MHz
	 * SSP bit rate = SSPCLK / (CLOCK_DIVIDE * (1 + CLOCK_RATE)),
	 * CLOCK_DIVIDE has to be an even value from 2 to 254, and
	 * CLOCK_RATE could be any integer from 0 to 255.
	 */
	plat->cfg.f_min = 400000;
	plat->cfg.f_max = mxc_get_clock(MXC_SSP0_CLK + priv->clkid) * 1000 / 2;
	plat->cfg.b_max = 0x20;

	bdesc = mmc_get_blk_desc(mmc);
	if (!bdesc) {
		printf("%s: No block device descriptor!\n", __func__);
		return -ENODEV;
	}

	if (plat->non_removable)
		bdesc->removable = 0;

	ret = mxsmmc_init(dev);
	if (ret)
		printf("%s: MMC%d init error %d\n", __func__,
		       bdesc->devnum, ret);

	/* Set the initial clock speed */
	mmc_set_clock(mmc, 400000, MMC_CLK_ENABLE);

	upriv->mmc = mmc;

	return 0;
};

#if CONFIG_IS_ENABLED(BLK)
static int mxsmmc_bind(struct udevice *dev)
{
	struct mxsmmc_plat *plat = dev_get_plat(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}
#endif

static const struct dm_mmc_ops mxsmmc_ops = {
	.get_cd		= mxsmmc_get_cd,
	.send_cmd	= mxsmmc_send_cmd,
	.set_ios	= mxsmmc_set_ios,
};

#if CONFIG_IS_ENABLED(OF_REAL)
static int mxsmmc_of_to_plat(struct udevice *bus)
{
	struct mxsmmc_plat *plat = dev_get_plat(bus);
	u32 prop[2];
	int ret;

	plat->base = dev_read_addr(bus);
	plat->buswidth =
		dev_read_u32_default(bus, "bus-width", 1);
	plat->non_removable = dev_read_bool(bus, "non-removable");

	ret = dev_read_u32_array(bus, "dmas", prop, ARRAY_SIZE(prop));
	if (ret) {
		printf("%s: Reading 'dmas' property failed!\n", __func__);
		return ret;
	}
	plat->dma_id = prop[1];

	ret = dev_read_u32_array(bus, "clocks", prop, ARRAY_SIZE(prop));
	if (ret) {
		printf("%s: Reading 'clocks' property failed!\n", __func__);
		return ret;
	}
	plat->clk_id = prop[1];

	debug("%s: base=0x%x, bus_width=%d %s dma_id=%d clk_id=%d\n",
	      __func__, (uint)plat->base, plat->buswidth,
	      plat->non_removable ? "non-removable" : NULL,
	      plat->dma_id, plat->clk_id);

	return 0;
}

static const struct udevice_id mxsmmc_ids[] = {
	{ .compatible = "fsl,imx23-mmc", },
	{ .compatible = "fsl,imx28-mmc", },
	{ /* sentinel */ }
};
#endif

U_BOOT_DRIVER(fsl_imx23_mmc) = {
	.name = "fsl_imx23_mmc",
	.id	= UCLASS_MMC,
#if CONFIG_IS_ENABLED(OF_REAL)
	.of_match = mxsmmc_ids,
	.of_to_plat = mxsmmc_of_to_plat,
#endif
	.ops	= &mxsmmc_ops,
#if CONFIG_IS_ENABLED(BLK)
	.bind	= mxsmmc_bind,
#endif
	.probe	= mxsmmc_probe,
	.priv_auto	= sizeof(struct mxsmmc_priv),
	.plat_auto	= sizeof(struct mxsmmc_plat),
};

DM_DRIVER_ALIAS(fsl_imx23_mmc, fsl_imx28_mmc)
#endif /* CONFIG_DM_MMC */
