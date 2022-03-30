// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 SSP MMC driver
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
#include <malloc.h>
#include <mmc.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/dma.h>
#include <bouncebuf.h>

struct mxsmmc_priv {
	int			id;
	struct mxs_ssp_regs	*regs;
	uint32_t		buswidth;
	int			(*mmc_is_wp)(int);
	int			(*mmc_cd)(int);
	struct mxs_dma_desc	*desc;
	struct mmc_config	cfg;	/* mmc configuration */
};

#define	MXSMMC_MAX_TIMEOUT	10000
#define MXSMMC_SMALL_TRANSFER	512

static int mxsmmc_cd(struct mxsmmc_priv *priv)
{
	struct mxs_ssp_regs *ssp_regs = priv->regs;

	if (priv->mmc_cd)
		return priv->mmc_cd(priv->id);

	return !(readl(&ssp_regs->hw_ssp_status) & SSP_STATUS_CARD_DETECT);
}

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

	dmach = MXS_DMA_CHANNEL_AHB_APBH_SSP0 + priv->id;
	mxs_dma_desc_append(dmach, priv->desc);
	if (mxs_dma_go(dmach)) {
		bounce_buffer_stop(&bbstate);
		return -ECOMM;
	}

	bounce_buffer_stop(&bbstate);

	return 0;
}

/*
 * Sends a command out on the bus.  Takes the mmc pointer,
 * a command pointer, and an optional data pointer.
 */
static int
mxsmmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	struct mxsmmc_priv *priv = mmc->priv;
	struct mxs_ssp_regs *ssp_regs = priv->regs;
	uint32_t reg;
	int timeout;
	uint32_t ctrl0;
	int ret;

	debug("MMC%d: CMD%d\n", mmc->block_dev.devnum, cmd->cmdidx);

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
		printf("MMC%d: Bus busy timeout!\n", mmc->block_dev.devnum);
		return -ETIMEDOUT;
	}

	/* See if card is present */
	if (!mxsmmc_cd(priv)) {
		printf("MMC%d: No card detected!\n", mmc->block_dev.devnum);
		return -ENOMEDIUM;
	}

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
		} else if (priv->mmc_is_wp &&
			priv->mmc_is_wp(mmc->block_dev.devnum)) {
			printf("MMC%d: Can not write a locked card!\n",
				mmc->block_dev.devnum);
			return -EOPNOTSUPP;
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
		printf("MMC%d: Command %d busy\n",
			mmc->block_dev.devnum, cmd->cmdidx);
		return -ETIMEDOUT;
	}

	/* Check command timeout */
	if (reg & SSP_STATUS_RESP_TIMEOUT) {
		printf("MMC%d: Command %d timeout (status 0x%08x)\n",
			mmc->block_dev.devnum, cmd->cmdidx, reg);
		return -ETIMEDOUT;
	}

	/* Check command errors */
	if (reg & (SSP_STATUS_RESP_CRC_ERR | SSP_STATUS_RESP_ERR)) {
		printf("MMC%d: Command %d error (status 0x%08x)!\n",
			mmc->block_dev.devnum, cmd->cmdidx, reg);
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
				"(status 0x%08x)!\n",
				mmc->block_dev.devnum, cmd->cmdidx, reg);
			return ret;
		}
	} else {
		ret = mxsmmc_send_cmd_dma(priv, data);
		if (ret) {
			printf("MMC%d: DMA transfer failed\n",
				mmc->block_dev.devnum);
			return ret;
		}
	}

	/* Check data errors */
	reg = readl(&ssp_regs->hw_ssp_status);
	if (reg &
		(SSP_STATUS_TIMEOUT | SSP_STATUS_DATA_CRC_ERR |
		SSP_STATUS_FIFO_OVRFLW | SSP_STATUS_FIFO_UNDRFLW)) {
		printf("MMC%d: Data error with command %d (status 0x%08x)!\n",
			mmc->block_dev.devnum, cmd->cmdidx, reg);
		return -ECOMM;
	}

	return 0;
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

int mxsmmc_initialize(bd_t *bis, int id, int (*wp)(int), int (*cd)(int))
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
	priv->cfg.f_max = mxc_get_clock(MXC_SSP0_CLK + mxsmmc_clk_id) * 1000 / 2;
	priv->cfg.b_max = 0x20;

	mmc = mmc_create(&priv->cfg, priv);
	if (mmc == NULL) {
		mxs_dma_desc_free(priv->desc);
		free(priv);
		return -ENOMEM;
	}
	return 0;
}
