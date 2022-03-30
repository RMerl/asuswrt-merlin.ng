// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Marek Vasut <marek.vasut@gmail.com>
 *
 * Loosely based on the old code and Linux's PXA MMC driver
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/regs-mmc.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <malloc.h>
#include <mmc.h>

/* PXAMMC Generic default config for various CPUs */
#if defined(CONFIG_CPU_PXA25X)
#define PXAMMC_FIFO_SIZE	1
#define PXAMMC_MIN_SPEED	312500
#define PXAMMC_MAX_SPEED	20000000
#define PXAMMC_HOST_CAPS	(0)
#elif defined(CONFIG_CPU_PXA27X)
#define PXAMMC_CRC_SKIP
#define PXAMMC_FIFO_SIZE	32
#define PXAMMC_MIN_SPEED	304000
#define PXAMMC_MAX_SPEED	19500000
#define PXAMMC_HOST_CAPS	(MMC_MODE_4BIT)
#elif defined(CONFIG_CPU_MONAHANS)
#define PXAMMC_FIFO_SIZE	32
#define PXAMMC_MIN_SPEED	304000
#define PXAMMC_MAX_SPEED	26000000
#define PXAMMC_HOST_CAPS	(MMC_MODE_4BIT | MMC_MODE_HS)
#else
#error "This CPU isn't supported by PXA MMC!"
#endif

#define MMC_STAT_ERRORS							\
	(MMC_STAT_RES_CRC_ERROR | MMC_STAT_SPI_READ_ERROR_TOKEN |	\
	MMC_STAT_CRC_READ_ERROR | MMC_STAT_TIME_OUT_RESPONSE |		\
	MMC_STAT_READ_TIME_OUT | MMC_STAT_CRC_WRITE_ERROR)

/* 1 millisecond (in wait cycles below it's 100 x 10uS waits) */
#define PXA_MMC_TIMEOUT	100

struct pxa_mmc_priv {
	struct pxa_mmc_regs *regs;
};

/* Wait for bit to be set */
static int pxa_mmc_wait(struct mmc *mmc, uint32_t mask)
{
	struct pxa_mmc_priv *priv = mmc->priv;
	struct pxa_mmc_regs *regs = priv->regs;
	unsigned int timeout = PXA_MMC_TIMEOUT;

	/* Wait for bit to be set */
	while (--timeout) {
		if (readl(&regs->stat) & mask)
			break;
		udelay(10);
	}

	if (!timeout)
		return -ETIMEDOUT;

	return 0;
}

static int pxa_mmc_stop_clock(struct mmc *mmc)
{
	struct pxa_mmc_priv *priv = mmc->priv;
	struct pxa_mmc_regs *regs = priv->regs;
	unsigned int timeout = PXA_MMC_TIMEOUT;

	/* If the clock aren't running, exit */
	if (!(readl(&regs->stat) & MMC_STAT_CLK_EN))
		return 0;

	/* Tell the controller to turn off the clock */
	writel(MMC_STRPCL_STOP_CLK, &regs->strpcl);

	/* Wait until the clock are off */
	while (--timeout) {
		if (!(readl(&regs->stat) & MMC_STAT_CLK_EN))
			break;
		udelay(10);
	}

	/* The clock refused to stop, scream and die a painful death */
	if (!timeout)
		return -ETIMEDOUT;

	/* The clock stopped correctly */
	return 0;
}

static int pxa_mmc_start_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
				uint32_t cmdat)
{
	struct pxa_mmc_priv *priv = mmc->priv;
	struct pxa_mmc_regs *regs = priv->regs;
	int ret;

	/* The card can send a "busy" response */
	if (cmd->resp_type & MMC_RSP_BUSY)
		cmdat |= MMC_CMDAT_BUSY;

	/* Inform the controller about response type */
	switch (cmd->resp_type) {
	case MMC_RSP_R1:
	case MMC_RSP_R1b:
		cmdat |= MMC_CMDAT_R1;
		break;
	case MMC_RSP_R2:
		cmdat |= MMC_CMDAT_R2;
		break;
	case MMC_RSP_R3:
		cmdat |= MMC_CMDAT_R3;
		break;
	default:
		break;
	}

	/* Load command and it's arguments into the controller */
	writel(cmd->cmdidx, &regs->cmd);
	writel(cmd->cmdarg >> 16, &regs->argh);
	writel(cmd->cmdarg & 0xffff, &regs->argl);
	writel(cmdat, &regs->cmdat);

	/* Start the controller clock and wait until they are started */
	writel(MMC_STRPCL_START_CLK, &regs->strpcl);

	ret = pxa_mmc_wait(mmc, MMC_STAT_CLK_EN);
	if (ret)
		return ret;

	/* Correct and happy end */
	return 0;
}

static int pxa_mmc_cmd_done(struct mmc *mmc, struct mmc_cmd *cmd)
{
	struct pxa_mmc_priv *priv = mmc->priv;
	struct pxa_mmc_regs *regs = priv->regs;
	uint32_t a, b, c;
	int i;
	int stat;

	/* Read the controller status */
	stat = readl(&regs->stat);

	/*
	 * Linux says:
	 * Did I mention this is Sick.  We always need to
	 * discard the upper 8 bits of the first 16-bit word.
	 */
	a = readl(&regs->res) & 0xffff;
	for (i = 0; i < 4; i++) {
		b = readl(&regs->res) & 0xffff;
		c = readl(&regs->res) & 0xffff;
		cmd->response[i] = (a << 24) | (b << 8) | (c >> 8);
		a = c;
	}

	/* The command response didn't arrive */
	if (stat & MMC_STAT_TIME_OUT_RESPONSE)
		return -ETIMEDOUT;
	else if (stat & MMC_STAT_RES_CRC_ERROR
			&& cmd->resp_type & MMC_RSP_CRC) {
#ifdef	PXAMMC_CRC_SKIP
		if (cmd->resp_type & MMC_RSP_136
				&& cmd->response[0] & (1 << 31))
			printf("Ignoring CRC, this may be dangerous!\n");
		else
#endif
		return -EILSEQ;
	}

	/* The command response was successfully read */
	return 0;
}

static int pxa_mmc_do_read_xfer(struct mmc *mmc, struct mmc_data *data)
{
	struct pxa_mmc_priv *priv = mmc->priv;
	struct pxa_mmc_regs *regs = priv->regs;
	uint32_t len;
	uint32_t *buf = (uint32_t *)data->dest;
	int size;
	int ret;

	len = data->blocks * data->blocksize;

	while (len) {
		/* The controller has data ready */
		if (readl(&regs->i_reg) & MMC_I_REG_RXFIFO_RD_REQ) {
			size = min(len, (uint32_t)PXAMMC_FIFO_SIZE);
			len -= size;
			size /= 4;

			/* Read data into the buffer */
			while (size--)
				*buf++ = readl(&regs->rxfifo);

		}

		if (readl(&regs->stat) & MMC_STAT_ERRORS)
			return -EIO;
	}

	/* Wait for the transmission-done interrupt */
	ret = pxa_mmc_wait(mmc, MMC_STAT_DATA_TRAN_DONE);
	if (ret)
		return ret;

	return 0;
}

static int pxa_mmc_do_write_xfer(struct mmc *mmc, struct mmc_data *data)
{
	struct pxa_mmc_priv *priv = mmc->priv;
	struct pxa_mmc_regs *regs = priv->regs;
	uint32_t len;
	uint32_t *buf = (uint32_t *)data->src;
	int size;
	int ret;

	len = data->blocks * data->blocksize;

	while (len) {
		/* The controller is ready to receive data */
		if (readl(&regs->i_reg) & MMC_I_REG_TXFIFO_WR_REQ) {
			size = min(len, (uint32_t)PXAMMC_FIFO_SIZE);
			len -= size;
			size /= 4;

			while (size--)
				writel(*buf++, &regs->txfifo);

			if (min(len, (uint32_t)PXAMMC_FIFO_SIZE) < 32)
				writel(MMC_PRTBUF_BUF_PART_FULL, &regs->prtbuf);
		}

		if (readl(&regs->stat) & MMC_STAT_ERRORS)
			return -EIO;
	}

	/* Wait for the transmission-done interrupt */
	ret = pxa_mmc_wait(mmc, MMC_STAT_DATA_TRAN_DONE);
	if (ret)
		return ret;

	/* Wait until the data are really written to the card */
	ret = pxa_mmc_wait(mmc, MMC_STAT_PRG_DONE);
	if (ret)
		return ret;

	return 0;
}

static int pxa_mmc_request(struct mmc *mmc, struct mmc_cmd *cmd,
				struct mmc_data *data)
{
	struct pxa_mmc_priv *priv = mmc->priv;
	struct pxa_mmc_regs *regs = priv->regs;
	uint32_t cmdat = 0;
	int ret;

	/* Stop the controller */
	ret = pxa_mmc_stop_clock(mmc);
	if (ret)
		return ret;

	/* If we're doing data transfer, configure the controller accordingly */
	if (data) {
		writel(data->blocks, &regs->nob);
		writel(data->blocksize, &regs->blklen);
		/* This delay can be optimized, but stick with max value */
		writel(0xffff, &regs->rdto);
		cmdat |= MMC_CMDAT_DATA_EN;
		if (data->flags & MMC_DATA_WRITE)
			cmdat |= MMC_CMDAT_WRITE;
	}

	/* Run in 4bit mode if the card can do it */
	if (mmc->bus_width == 4)
		cmdat |= MMC_CMDAT_SD_4DAT;

	/* Execute the command */
	ret = pxa_mmc_start_cmd(mmc, cmd, cmdat);
	if (ret)
		return ret;

	/* Wait until the command completes */
	ret = pxa_mmc_wait(mmc, MMC_STAT_END_CMD_RES);
	if (ret)
		return ret;

	/* Read back the result */
	ret = pxa_mmc_cmd_done(mmc, cmd);
	if (ret)
		return ret;

	/* In case there was a data transfer scheduled, do it */
	if (data) {
		if (data->flags & MMC_DATA_WRITE)
			pxa_mmc_do_write_xfer(mmc, data);
		else
			pxa_mmc_do_read_xfer(mmc, data);
	}

	return 0;
}

static int pxa_mmc_set_ios(struct mmc *mmc)
{
	struct pxa_mmc_priv *priv = mmc->priv;
	struct pxa_mmc_regs *regs = priv->regs;
	uint32_t tmp;
	uint32_t pxa_mmc_clock;

	if (!mmc->clock) {
		pxa_mmc_stop_clock(mmc);
		return 0;
	}

	/* PXA3xx can do 26MHz with special settings. */
	if (mmc->clock == 26000000) {
		writel(0x7, &regs->clkrt);
		return 0;
	}

	/* Set clock to the card the usual way. */
	pxa_mmc_clock = 0;
	tmp = mmc->cfg->f_max / mmc->clock;
	tmp += tmp % 2;

	while (tmp > 1) {
		pxa_mmc_clock++;
		tmp >>= 1;
	}

	writel(pxa_mmc_clock, &regs->clkrt);

	return 0;
}

static int pxa_mmc_init(struct mmc *mmc)
{
	struct pxa_mmc_priv *priv = mmc->priv;
	struct pxa_mmc_regs *regs = priv->regs;

	/* Make sure the clock are stopped */
	pxa_mmc_stop_clock(mmc);

	/* Turn off SPI mode */
	writel(0, &regs->spi);

	/* Set up maximum timeout to wait for command response */
	writel(MMC_RES_TO_MAX_MASK, &regs->resto);

	/* Mask all interrupts */
	writel(~(MMC_I_MASK_TXFIFO_WR_REQ | MMC_I_MASK_RXFIFO_RD_REQ),
		&regs->i_mask);
	return 0;
}

static const struct mmc_ops pxa_mmc_ops = {
	.send_cmd	= pxa_mmc_request,
	.set_ios	= pxa_mmc_set_ios,
	.init		= pxa_mmc_init,
};

static struct mmc_config pxa_mmc_cfg = {
	.name		= "PXA MMC",
	.ops		= &pxa_mmc_ops,
	.voltages	= MMC_VDD_32_33 | MMC_VDD_33_34,
	.f_max		= PXAMMC_MAX_SPEED,
	.f_min		= PXAMMC_MIN_SPEED,
	.host_caps	= PXAMMC_HOST_CAPS,
	.b_max		= CONFIG_SYS_MMC_MAX_BLK_COUNT,
};

int pxa_mmc_register(int card_index)
{
	struct mmc *mmc;
	struct pxa_mmc_priv *priv;
	uint32_t reg;
	int ret = -ENOMEM;

	priv = malloc(sizeof(struct pxa_mmc_priv));
	if (!priv)
		goto err0;

	memset(priv, 0, sizeof(*priv));

	switch (card_index) {
	case 0:
		priv->regs = (struct pxa_mmc_regs *)MMC0_BASE;
		break;
	case 1:
		priv->regs = (struct pxa_mmc_regs *)MMC1_BASE;
		break;
	default:
		ret = -EINVAL;
		printf("PXA MMC: Invalid MMC controller ID (card_index = %d)\n",
			card_index);
		goto err1;
	}

#ifndef	CONFIG_CPU_MONAHANS	/* PXA2xx */
	reg = readl(CKEN);
	reg |= CKEN12_MMC;
	writel(reg, CKEN);
#else				/* PXA3xx */
	reg = readl(CKENA);
	reg |= CKENA_12_MMC0 | CKENA_13_MMC1;
	writel(reg, CKENA);
#endif

	mmc = mmc_create(&pxa_mmc_cfg, priv);
	if (mmc == NULL)
		goto err1;

	return 0;

err1:
	free(priv);
err0:
	return ret;
}
