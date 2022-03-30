// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010
 * Rob Emanuele <rob@emanuele.us>
 * Reinhard Meyer, EMK Elektronik <reinhard.meyer@emk-elektronik.de>
 *
 * Original Driver:
 * Copyright (C) 2004-2006 Atmel Corporation
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/byteorder.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include "atmel_mci.h"

#ifndef CONFIG_SYS_MMC_CLK_OD
# define CONFIG_SYS_MMC_CLK_OD	150000
#endif

#define MMC_DEFAULT_BLKLEN	512

#if defined(CONFIG_ATMEL_MCI_PORTB)
# define MCI_BUS 1
#else
# define MCI_BUS 0
#endif

#ifdef CONFIG_DM_MMC
struct atmel_mci_plat {
	struct mmc		mmc;
	struct mmc_config	cfg;
	struct atmel_mci	*mci;
};
#endif

struct atmel_mci_priv {
#ifndef CONFIG_DM_MMC
	struct mmc_config	cfg;
	struct atmel_mci	*mci;
#endif
	unsigned int		initialized:1;
	unsigned int		curr_clk;
#ifdef CONFIG_DM_MMC
	ulong		bus_clk_rate;
#endif
};

/* Read Atmel MCI IP version */
static unsigned int atmel_mci_get_version(struct atmel_mci *mci)
{
	return readl(&mci->version) & 0x00000fff;
}

/*
 * Print command and status:
 *
 * - always when DEBUG is defined
 * - on command errors
 */
static void dump_cmd(u32 cmdr, u32 arg, u32 status, const char* msg)
{
	debug("gen_atmel_mci: CMDR %08x (%2u) ARGR %08x (SR: %08x) %s\n",
	      cmdr, cmdr & 0x3F, arg, status, msg);
}

static inline void mci_set_blklen(atmel_mci_t *mci, int blklen)
{
	unsigned int version = atmel_mci_get_version(mci);

	blklen &= 0xfffc;

	/* MCI IP version >= 0x200 has blkr */
	if (version >= 0x200)
		writel(MMCI_BFINS(BLKLEN, blklen, readl(&mci->blkr)),
		       &mci->blkr);
	else
		writel(MMCI_BFINS(BLKLEN, blklen, readl(&mci->mr)), &mci->mr);
}

/* Setup for MCI Clock and Block Size */
#ifdef CONFIG_DM_MMC
static void mci_set_mode(struct udevice *dev, u32 hz, u32 blklen)
{
	struct atmel_mci_plat *plat = dev_get_platdata(dev);
	struct atmel_mci_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = &plat->mmc;
	u32 bus_hz = priv->bus_clk_rate;
	atmel_mci_t *mci = plat->mci;
#else
static void mci_set_mode(struct mmc *mmc, u32 hz, u32 blklen)
{
	struct atmel_mci_priv *priv = mmc->priv;
	u32 bus_hz = get_mci_clk_rate();
	atmel_mci_t *mci = priv->mci;
#endif

	u32 clkdiv = 255;
	unsigned int version = atmel_mci_get_version(mci);
	u32 clkodd = 0;
	u32 mr;

	debug("mci: bus_hz is %u, setting clock %u Hz, block size %u\n",
		bus_hz, hz, blklen);
	if (hz > 0) {
		if (version >= 0x500) {
			clkdiv = DIV_ROUND_UP(bus_hz, hz) - 2;
			if (clkdiv > 511)
				clkdiv = 511;

			clkodd = clkdiv & 1;
			clkdiv >>= 1;

			debug("mci: setting clock %u Hz, block size %u\n",
			      bus_hz / (clkdiv * 2 + clkodd + 2), blklen);
		} else {
			/* find clkdiv yielding a rate <= than requested */
			for (clkdiv = 0; clkdiv < 255; clkdiv++) {
				if ((bus_hz / (clkdiv + 1) / 2) <= hz)
					break;
			}
			debug("mci: setting clock %u Hz, block size %u\n",
			      (bus_hz / (clkdiv + 1)) / 2, blklen);

		}
	}
	if (version >= 0x500)
		priv->curr_clk = bus_hz / (clkdiv * 2 + clkodd + 2);
	else
		priv->curr_clk = (bus_hz / (clkdiv + 1)) / 2;

	mr = MMCI_BF(CLKDIV, clkdiv);

	/* MCI IP version >= 0x200 has R/WPROOF */
	if (version >= 0x200)
		mr |= MMCI_BIT(RDPROOF) | MMCI_BIT(WRPROOF);

	/*
	 * MCI IP version >= 0x500 use bit 16 as clkodd.
	 * MCI IP version < 0x500 use upper 16 bits for blklen.
	 */
	if (version >= 0x500)
		mr |= MMCI_BF(CLKODD, clkodd);

	writel(mr, &mci->mr);

	mci_set_blklen(mci, blklen);

	if (mmc->card_caps & mmc->cfg->host_caps & MMC_MODE_HS)
		writel(MMCI_BIT(HSMODE), &mci->cfg);

	priv->initialized = 1;
}

/* Return the CMDR with flags for a given command and data packet */
static u32 mci_encode_cmd(
	struct mmc_cmd *cmd, struct mmc_data *data, u32* error_flags)
{
	u32 cmdr = 0;

	/* Default Flags for Errors */
	*error_flags |= (MMCI_BIT(DTOE) | MMCI_BIT(RDIRE) | MMCI_BIT(RENDE) |
		MMCI_BIT(RINDE) | MMCI_BIT(RTOE));

	/* Default Flags for the Command */
	cmdr |= MMCI_BIT(MAXLAT);

	if (data) {
		cmdr |= MMCI_BF(TRCMD, 1);
		if (data->blocks > 1)
			cmdr |= MMCI_BF(TRTYP, 1);
		if (data->flags & MMC_DATA_READ)
			cmdr |= MMCI_BIT(TRDIR);
	}

	if (cmd->resp_type & MMC_RSP_CRC)
		*error_flags |= MMCI_BIT(RCRCE);
	if (cmd->resp_type & MMC_RSP_136)
		cmdr |= MMCI_BF(RSPTYP, 2);
	else if (cmd->resp_type & MMC_RSP_BUSY)
		cmdr |= MMCI_BF(RSPTYP, 3);
	else if (cmd->resp_type & MMC_RSP_PRESENT)
		cmdr |= MMCI_BF(RSPTYP, 1);

	return cmdr | MMCI_BF(CMDNB, cmd->cmdidx);
}

/* Entered into function pointer in mci_send_cmd */
static u32 mci_data_read(atmel_mci_t *mci, u32* data, u32 error_flags)
{
	u32 status;

	do {
		status = readl(&mci->sr);
		if (status & (error_flags | MMCI_BIT(OVRE)))
			goto io_fail;
	} while (!(status & MMCI_BIT(RXRDY)));

	if (status & MMCI_BIT(RXRDY)) {
		*data = readl(&mci->rdr);
		status = 0;
	}
io_fail:
	return status;
}

/* Entered into function pointer in mci_send_cmd */
static u32 mci_data_write(atmel_mci_t *mci, u32* data, u32 error_flags)
{
	u32 status;

	do {
		status = readl(&mci->sr);
		if (status & (error_flags | MMCI_BIT(UNRE)))
			goto io_fail;
	} while (!(status & MMCI_BIT(TXRDY)));

	if (status & MMCI_BIT(TXRDY)) {
		writel(*data, &mci->tdr);
		status = 0;
	}
io_fail:
	return status;
}

/*
 * Entered into mmc structure during driver init
 *
 * Sends a command out on the bus and deals with the block data.
 * Takes the mmc pointer, a command pointer, and an optional data pointer.
 */
#ifdef CONFIG_DM_MMC
static int atmel_mci_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct atmel_mci_plat *plat = dev_get_platdata(dev);
	struct atmel_mci_priv *priv = dev_get_priv(dev);
	atmel_mci_t *mci = plat->mci;
#else
static int
mci_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	struct atmel_mci_priv *priv = mmc->priv;
	atmel_mci_t *mci = priv->mci;
#endif
	u32 cmdr;
	u32 error_flags = 0;
	u32 status;

	if (!priv->initialized) {
		puts ("MCI not initialized!\n");
		return -ECOMM;
	}

	/* Figure out the transfer arguments */
	cmdr = mci_encode_cmd(cmd, data, &error_flags);

	mci_set_blklen(mci, data->blocksize);

	/* For multi blocks read/write, set the block register */
	if ((cmd->cmdidx == MMC_CMD_READ_MULTIPLE_BLOCK)
			|| (cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK))
		writel(data->blocks | MMCI_BF(BLKLEN, data->blocksize),
		       &mci->blkr);

	/* Send the command */
	writel(cmd->cmdarg, &mci->argr);
	writel(cmdr, &mci->cmdr);

#ifdef DEBUG
	dump_cmd(cmdr, cmd->cmdarg, 0, "DEBUG");
#endif

	/* Wait for the command to complete */
	while (!((status = readl(&mci->sr)) & MMCI_BIT(CMDRDY)));

	if ((status & error_flags) & MMCI_BIT(RTOE)) {
		dump_cmd(cmdr, cmd->cmdarg, status, "Command Time Out");
		return -ETIMEDOUT;
	} else if (status & error_flags) {
		dump_cmd(cmdr, cmd->cmdarg, status, "Command Failed");
		return -ECOMM;
	}

	/* Copy the response to the response buffer */
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = readl(&mci->rspr);
		cmd->response[1] = readl(&mci->rspr1);
		cmd->response[2] = readl(&mci->rspr2);
		cmd->response[3] = readl(&mci->rspr3);
	} else
		cmd->response[0] = readl(&mci->rspr);

	/* transfer all of the blocks */
	if (data) {
		u32 word_count, block_count;
		u32* ioptr;
		u32 i;
		u32 (*mci_data_op)
			(atmel_mci_t *mci, u32* data, u32 error_flags);

		if (data->flags & MMC_DATA_READ) {
			mci_data_op = mci_data_read;
			ioptr = (u32*)data->dest;
		} else {
			mci_data_op = mci_data_write;
			ioptr = (u32*)data->src;
		}

		status = 0;
		for (block_count = 0;
				block_count < data->blocks && !status;
				block_count++) {
			word_count = 0;
			do {
				status = mci_data_op(mci, ioptr, error_flags);
				word_count++;
				ioptr++;
			} while (!status && word_count < (data->blocksize/4));
#ifdef DEBUG
			if (data->flags & MMC_DATA_READ)
			{
				u32 cnt = word_count * 4;
				printf("Read Data:\n");
				print_buffer(0, data->dest + cnt * block_count,
					     1, cnt, 0);
			}
#endif
			if (status) {
				dump_cmd(cmdr, cmd->cmdarg, status,
					"Data Transfer Failed");
				return -ECOMM;
			}
		}

		/* Wait for Transfer End */
		i = 0;
		do {
			status = readl(&mci->sr);

			if (status & error_flags) {
				dump_cmd(cmdr, cmd->cmdarg, status,
					"DTIP Wait Failed");
				return -ECOMM;
			}
			i++;
		} while ((status & MMCI_BIT(DTIP)) && i < 10000);
		if (status & MMCI_BIT(DTIP)) {
			dump_cmd(cmdr, cmd->cmdarg, status,
				"XFER DTIP never unset, ignoring");
		}
	}

	/*
	 * After the switch command, wait for 8 clocks before the next
	 * command
	 */
	if (cmd->cmdidx == MMC_CMD_SWITCH)
		udelay(8*1000000 / priv->curr_clk); /* 8 clk in us */

	return 0;
}

#ifdef CONFIG_DM_MMC
static int atmel_mci_set_ios(struct udevice *dev)
{
	struct atmel_mci_plat *plat = dev_get_platdata(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	atmel_mci_t *mci = plat->mci;
#else
/* Entered into mmc structure during driver init */
static int mci_set_ios(struct mmc *mmc)
{
	struct atmel_mci_priv *priv = mmc->priv;
	atmel_mci_t *mci = priv->mci;
#endif
	int bus_width = mmc->bus_width;
	unsigned int version = atmel_mci_get_version(mci);
	int busw;

	/* Set the clock speed */
#ifdef CONFIG_DM_MMC
	mci_set_mode(dev, mmc->clock, MMC_DEFAULT_BLKLEN);
#else
	mci_set_mode(mmc, mmc->clock, MMC_DEFAULT_BLKLEN);
#endif

	/*
	 * set the bus width and select slot for this interface
	 * there is no capability for multiple slots on the same interface yet
	 */
	if ((version & 0xf00) >= 0x300) {
		switch (bus_width) {
		case 8:
			busw = 3;
			break;
		case 4:
			busw = 2;
			break;
		default:
			busw = 0;
			break;
		}

		writel(busw << 6 | MMCI_BF(SCDSEL, MCI_BUS), &mci->sdcr);
	} else {
		busw = (bus_width == 4) ? 1 : 0;

		writel(busw << 7 | MMCI_BF(SCDSEL, MCI_BUS), &mci->sdcr);
	}

	return 0;
}

#ifdef CONFIG_DM_MMC
static int atmel_mci_hw_init(struct udevice *dev)
{
	struct atmel_mci_plat *plat = dev_get_platdata(dev);
	atmel_mci_t *mci = plat->mci;
#else
/* Entered into mmc structure during driver init */
static int mci_init(struct mmc *mmc)
{
	struct atmel_mci_priv *priv = mmc->priv;
	atmel_mci_t *mci = priv->mci;
#endif

	/* Initialize controller */
	writel(MMCI_BIT(SWRST), &mci->cr);	/* soft reset */
	writel(MMCI_BIT(PWSDIS), &mci->cr);	/* disable power save */
	writel(MMCI_BIT(MCIEN), &mci->cr);	/* enable mci */
	writel(MMCI_BF(SCDSEL, MCI_BUS), &mci->sdcr);	/* select port */

	/* This delay can be optimized, but stick with max value */
	writel(0x7f, &mci->dtor);
	/* Disable Interrupts */
	writel(~0UL, &mci->idr);

	/* Set default clocks and blocklen */
#ifdef CONFIG_DM_MMC
	mci_set_mode(dev, CONFIG_SYS_MMC_CLK_OD, MMC_DEFAULT_BLKLEN);
#else
	mci_set_mode(mmc, CONFIG_SYS_MMC_CLK_OD, MMC_DEFAULT_BLKLEN);
#endif

	return 0;
}

#ifndef CONFIG_DM_MMC
static const struct mmc_ops atmel_mci_ops = {
	.send_cmd	= mci_send_cmd,
	.set_ios	= mci_set_ios,
	.init		= mci_init,
};

/*
 * This is the only exported function
 *
 * Call it with the MCI register base address
 */
int atmel_mci_init(void *regs)
{
	struct mmc *mmc;
	struct mmc_config *cfg;
	struct atmel_mci_priv *priv;
	unsigned int version;

	priv = calloc(1, sizeof(*priv));
	if (!priv)
		return -ENOMEM;

	cfg = &priv->cfg;

	cfg->name = "mci";
	cfg->ops = &atmel_mci_ops;

	priv->mci = (struct atmel_mci *)regs;
	priv->initialized = 0;

	/* need to be able to pass these in on a board by board basis */
	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;
	version = atmel_mci_get_version(priv->mci);
	if ((version & 0xf00) >= 0x300) {
		cfg->host_caps = MMC_MODE_8BIT;
		cfg->host_caps |= MMC_MODE_HS | MMC_MODE_HS_52MHz;
	}

	cfg->host_caps |= MMC_MODE_4BIT;

	/*
	 * min and max frequencies determined by
	 * max and min of clock divider
	 */
	cfg->f_min = get_mci_clk_rate() / (2*256);
	cfg->f_max = get_mci_clk_rate() / (2*1);

	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	mmc = mmc_create(cfg, priv);

	if (mmc == NULL) {
		free(priv);
		return -ENODEV;
	}
	/* NOTE: possibly leaking the priv structure */

	return 0;
}
#endif

#ifdef CONFIG_DM_MMC
static const struct dm_mmc_ops atmel_mci_mmc_ops = {
	.send_cmd = atmel_mci_send_cmd,
	.set_ios = atmel_mci_set_ios,
};

static void atmel_mci_setup_cfg(struct udevice *dev)
{
	struct atmel_mci_plat *plat = dev_get_platdata(dev);
	struct atmel_mci_priv *priv = dev_get_priv(dev);
	struct mmc_config *cfg;
	u32 version;

	cfg = &plat->cfg;
	cfg->name = "Atmel mci";
	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34;

	/*
	 * If the version is above 3.0, the capabilities of the 8-bit
	 * bus width and high speed are supported.
	 */
	version = atmel_mci_get_version(plat->mci);
	if ((version & 0xf00) >= 0x300) {
		cfg->host_caps = MMC_MODE_8BIT |
				 MMC_MODE_HS | MMC_MODE_HS_52MHz;
	}

	cfg->host_caps |= MMC_MODE_4BIT;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;
	cfg->f_min = priv->bus_clk_rate / (2 * 256);
	cfg->f_max = priv->bus_clk_rate / 2;
}

static int atmel_mci_enable_clk(struct udevice *dev)
{
	struct atmel_mci_priv *priv = dev_get_priv(dev);
	struct clk clk;
	ulong clk_rate;
	int ret = 0;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret) {
		ret = -EINVAL;
		goto failed;
	}

	ret = clk_enable(&clk);
	if (ret)
		goto failed;

	clk_rate = clk_get_rate(&clk);
	if (!clk_rate) {
		ret = -EINVAL;
		goto failed;
	}

	priv->bus_clk_rate = clk_rate;

failed:
	clk_free(&clk);

	return ret;
}

static int atmel_mci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct atmel_mci_plat *plat = dev_get_platdata(dev);
	struct mmc *mmc;
	int ret;

	ret = atmel_mci_enable_clk(dev);
	if (ret)
		return ret;

	plat->mci = (struct atmel_mci *)devfdt_get_addr_ptr(dev);

	atmel_mci_setup_cfg(dev);

	mmc = &plat->mmc;
	mmc->cfg = &plat->cfg;
	mmc->dev = dev;
	upriv->mmc = mmc;

	atmel_mci_hw_init(dev);

	return 0;
}

static int atmel_mci_bind(struct udevice *dev)
{
	struct atmel_mci_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id atmel_mci_ids[] = {
	{ .compatible = "atmel,hsmci" },
	{ }
};

U_BOOT_DRIVER(atmel_mci) = {
	.name = "atmel-mci",
	.id = UCLASS_MMC,
	.of_match = atmel_mci_ids,
	.bind = atmel_mci_bind,
	.probe = atmel_mci_probe,
	.platdata_auto_alloc_size = sizeof(struct atmel_mci_plat),
	.priv_auto_alloc_size = sizeof(struct atmel_mci_priv),
	.ops = &atmel_mci_mmc_ops,
};
#endif
