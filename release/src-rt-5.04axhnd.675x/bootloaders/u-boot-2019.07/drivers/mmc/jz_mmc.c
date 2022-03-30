// SPDX-License-Identifier: GPL-2.0+
/*
 * Ingenic JZ MMC driver
 *
 * Copyright (c) 2013 Imagination Technologies
 * Author: Paul Burton <paul.burton@imgtec.com>
 */

#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <errno.h>
#include <mach/jz4780.h>
#include <wait_bit.h>

/* Registers */
#define MSC_STRPCL			0x000
#define MSC_STAT			0x004
#define MSC_CLKRT			0x008
#define MSC_CMDAT			0x00c
#define MSC_RESTO			0x010
#define MSC_RDTO			0x014
#define MSC_BLKLEN			0x018
#define MSC_NOB				0x01c
#define MSC_SNOB			0x020
#define MSC_IMASK			0x024
#define MSC_IREG			0x028
#define MSC_CMD				0x02c
#define MSC_ARG				0x030
#define MSC_RES				0x034
#define MSC_RXFIFO			0x038
#define MSC_TXFIFO			0x03c
#define MSC_LPM				0x040
#define MSC_DMAC			0x044
#define MSC_DMANDA			0x048
#define MSC_DMADA			0x04c
#define MSC_DMALEN			0x050
#define MSC_DMACMD			0x054
#define MSC_CTRL2			0x058
#define MSC_RTCNT			0x05c
#define MSC_DBG				0x0fc

/* MSC Clock and Control Register (MSC_STRPCL) */
#define MSC_STRPCL_EXIT_MULTIPLE	BIT(7)
#define MSC_STRPCL_EXIT_TRANSFER	BIT(6)
#define MSC_STRPCL_START_READWAIT	BIT(5)
#define MSC_STRPCL_STOP_READWAIT	BIT(4)
#define MSC_STRPCL_RESET		BIT(3)
#define MSC_STRPCL_START_OP		BIT(2)
#define MSC_STRPCL_CLOCK_CONTROL_STOP	BIT(0)
#define MSC_STRPCL_CLOCK_CONTROL_START	BIT(1)

/* MSC Status Register (MSC_STAT) */
#define MSC_STAT_AUTO_CMD_DONE		BIT(31)
#define MSC_STAT_IS_RESETTING		BIT(15)
#define MSC_STAT_SDIO_INT_ACTIVE	BIT(14)
#define MSC_STAT_PRG_DONE		BIT(13)
#define MSC_STAT_DATA_TRAN_DONE		BIT(12)
#define MSC_STAT_END_CMD_RES		BIT(11)
#define MSC_STAT_DATA_FIFO_AFULL	BIT(10)
#define MSC_STAT_IS_READWAIT		BIT(9)
#define MSC_STAT_CLK_EN			BIT(8)
#define MSC_STAT_DATA_FIFO_FULL		BIT(7)
#define MSC_STAT_DATA_FIFO_EMPTY	BIT(6)
#define MSC_STAT_CRC_RES_ERR		BIT(5)
#define MSC_STAT_CRC_READ_ERROR		BIT(4)
#define MSC_STAT_CRC_WRITE_ERROR	BIT(2)
#define MSC_STAT_CRC_WRITE_ERROR_NOSTS	BIT(4)
#define MSC_STAT_TIME_OUT_RES		BIT(1)
#define MSC_STAT_TIME_OUT_READ		BIT(0)

/* MSC Bus Clock Control Register (MSC_CLKRT) */
#define MSC_CLKRT_CLK_RATE_MASK		0x7

/* MSC Command Sequence Control Register (MSC_CMDAT) */
#define MSC_CMDAT_IO_ABORT		BIT(11)
#define MSC_CMDAT_BUS_WIDTH_1BIT	(0x0 << 9)
#define MSC_CMDAT_BUS_WIDTH_4BIT	(0x2 << 9)
#define MSC_CMDAT_DMA_EN		BIT(8)
#define MSC_CMDAT_INIT			BIT(7)
#define MSC_CMDAT_BUSY			BIT(6)
#define MSC_CMDAT_STREAM_BLOCK		BIT(5)
#define MSC_CMDAT_WRITE			BIT(4)
#define MSC_CMDAT_DATA_EN		BIT(3)
#define MSC_CMDAT_RESPONSE_MASK		(0x7 << 0)
#define MSC_CMDAT_RESPONSE_NONE		(0x0 << 0) /* No response */
#define MSC_CMDAT_RESPONSE_R1		(0x1 << 0) /* Format R1 and R1b */
#define MSC_CMDAT_RESPONSE_R2		(0x2 << 0) /* Format R2 */
#define MSC_CMDAT_RESPONSE_R3		(0x3 << 0) /* Format R3 */
#define MSC_CMDAT_RESPONSE_R4		(0x4 << 0) /* Format R4 */
#define MSC_CMDAT_RESPONSE_R5		(0x5 << 0) /* Format R5 */
#define MSC_CMDAT_RESPONSE_R6		(0x6 << 0) /* Format R6 */

/* MSC Interrupts Mask Register (MSC_IMASK) */
#define MSC_IMASK_TIME_OUT_RES		BIT(9)
#define MSC_IMASK_TIME_OUT_READ		BIT(8)
#define MSC_IMASK_SDIO			BIT(7)
#define MSC_IMASK_TXFIFO_WR_REQ		BIT(6)
#define MSC_IMASK_RXFIFO_RD_REQ		BIT(5)
#define MSC_IMASK_END_CMD_RES		BIT(2)
#define MSC_IMASK_PRG_DONE		BIT(1)
#define MSC_IMASK_DATA_TRAN_DONE	BIT(0)

/* MSC Interrupts Status Register (MSC_IREG) */
#define MSC_IREG_TIME_OUT_RES		BIT(9)
#define MSC_IREG_TIME_OUT_READ		BIT(8)
#define MSC_IREG_SDIO			BIT(7)
#define MSC_IREG_TXFIFO_WR_REQ		BIT(6)
#define MSC_IREG_RXFIFO_RD_REQ		BIT(5)
#define MSC_IREG_END_CMD_RES		BIT(2)
#define MSC_IREG_PRG_DONE		BIT(1)
#define MSC_IREG_DATA_TRAN_DONE		BIT(0)

struct jz_mmc_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct jz_mmc_priv {
	void __iomem		*regs;
	u32			flags;
/* priv flags */
#define JZ_MMC_BUS_WIDTH_MASK	0x3
#define JZ_MMC_BUS_WIDTH_1	0x0
#define JZ_MMC_BUS_WIDTH_4	0x2
#define JZ_MMC_BUS_WIDTH_8	0x3
#define JZ_MMC_SENT_INIT	BIT(2)
};

static int jz_mmc_clock_rate(void)
{
	return 24000000;
}

#if CONFIG_IS_ENABLED(MMC_WRITE)
static inline void jz_mmc_write_data(struct jz_mmc_priv *priv, struct mmc_data *data)
{
	int sz = DIV_ROUND_UP(data->blocks * data->blocksize, 4);
	const void *buf = data->src;

	while (sz--) {
		u32 val = get_unaligned_le32(buf);

		wait_for_bit_le32(priv->regs + MSC_IREG,
				  MSC_IREG_TXFIFO_WR_REQ,
				  true, 10000, false);
		writel(val, priv->regs + MSC_TXFIFO);
		buf += 4;
	}
}
#else
static void jz_mmc_write_data(struct jz_mmc_priv *priv, struct mmc_data *data)
{}
#endif

static inline int jz_mmc_read_data(struct jz_mmc_priv *priv, struct mmc_data *data)
{
	int sz = data->blocks * data->blocksize;
	void *buf = data->dest;
	u32 stat, val;

	do {
		stat = readl(priv->regs + MSC_STAT);

		if (stat & MSC_STAT_TIME_OUT_READ)
			return -ETIMEDOUT;
		if (stat & MSC_STAT_CRC_READ_ERROR)
			return -EINVAL;
		if (stat & MSC_STAT_DATA_FIFO_EMPTY) {
			udelay(10);
			continue;
		}
		do {
			val = readl(priv->regs + MSC_RXFIFO);
			if (sz == 1)
				*(u8 *)buf = (u8)val;
			else if (sz == 2)
				put_unaligned_le16(val, buf);
			else if (sz >= 4)
				put_unaligned_le32(val, buf);
			buf += 4;
			sz -= 4;
			stat = readl(priv->regs + MSC_STAT);
		} while (!(stat & MSC_STAT_DATA_FIFO_EMPTY));
	} while (!(stat & MSC_STAT_DATA_TRAN_DONE));
	return 0;
}

static int jz_mmc_send_cmd(struct mmc *mmc, struct jz_mmc_priv *priv,
			   struct mmc_cmd *cmd, struct mmc_data *data)
{
	u32 stat, mask, cmdat = 0;
	int i, ret;

	/* stop the clock */
	writel(MSC_STRPCL_CLOCK_CONTROL_STOP, priv->regs + MSC_STRPCL);
	ret = wait_for_bit_le32(priv->regs + MSC_STAT,
				MSC_STAT_CLK_EN, false, 10000, false);
	if (ret)
		return ret;

	writel(0, priv->regs + MSC_DMAC);

	/* setup command */
	writel(cmd->cmdidx, priv->regs + MSC_CMD);
	writel(cmd->cmdarg, priv->regs + MSC_ARG);

	if (data) {
		/* setup data */
		cmdat |= MSC_CMDAT_DATA_EN;
		if (data->flags & MMC_DATA_WRITE)
			cmdat |= MSC_CMDAT_WRITE;

		writel(data->blocks, priv->regs + MSC_NOB);
		writel(data->blocksize, priv->regs + MSC_BLKLEN);
	} else {
		writel(0, priv->regs + MSC_NOB);
		writel(0, priv->regs + MSC_BLKLEN);
	}

	/* setup response */
	switch (cmd->resp_type) {
	case MMC_RSP_NONE:
		break;
	case MMC_RSP_R1:
	case MMC_RSP_R1b:
		cmdat |= MSC_CMDAT_RESPONSE_R1;
		break;
	case MMC_RSP_R2:
		cmdat |= MSC_CMDAT_RESPONSE_R2;
		break;
	case MMC_RSP_R3:
		cmdat |= MSC_CMDAT_RESPONSE_R3;
		break;
	default:
		break;
	}

	if (cmd->resp_type & MMC_RSP_BUSY)
		cmdat |= MSC_CMDAT_BUSY;

	/* set init for the first command only */
	if (!(priv->flags & JZ_MMC_SENT_INIT)) {
		cmdat |= MSC_CMDAT_INIT;
		priv->flags |= JZ_MMC_SENT_INIT;
	}

	cmdat |= (priv->flags & JZ_MMC_BUS_WIDTH_MASK) << 9;

	/* write the data setup */
	writel(cmdat, priv->regs + MSC_CMDAT);

	/* unmask interrupts */
	mask = 0xffffffff & ~(MSC_IMASK_END_CMD_RES | MSC_IMASK_TIME_OUT_RES);
	if (data) {
		mask &= ~MSC_IMASK_DATA_TRAN_DONE;
		if (data->flags & MMC_DATA_WRITE) {
			mask &= ~MSC_IMASK_TXFIFO_WR_REQ;
		} else {
			mask &= ~(MSC_IMASK_RXFIFO_RD_REQ |
				  MSC_IMASK_TIME_OUT_READ);
		}
	}
	writel(mask, priv->regs + MSC_IMASK);

	/* clear interrupts */
	writel(0xffffffff, priv->regs + MSC_IREG);

	/* start the command (& the clock) */
	writel(MSC_STRPCL_START_OP | MSC_STRPCL_CLOCK_CONTROL_START,
	       priv->regs + MSC_STRPCL);

	/* wait for completion */
	for (i = 0; i < 100; i++) {
		stat = readl(priv->regs + MSC_IREG);
		stat &= MSC_IREG_END_CMD_RES | MSC_IREG_TIME_OUT_RES;
		if (stat)
			break;
		mdelay(1);
	}
	writel(stat, priv->regs + MSC_IREG);
	if (stat & MSC_IREG_TIME_OUT_RES)
		return -ETIMEDOUT;

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		/* read the response */
		if (cmd->resp_type & MMC_RSP_136) {
			u16 a, b, c, i;

			a = readw(priv->regs + MSC_RES);
			for (i = 0; i < 4; i++) {
				b = readw(priv->regs + MSC_RES);
				c = readw(priv->regs + MSC_RES);
				cmd->response[i] =
					(a << 24) | (b << 8) | (c >> 8);
				a = c;
			}
		} else {
			cmd->response[0] = readw(priv->regs + MSC_RES) << 24;
			cmd->response[0] |= readw(priv->regs + MSC_RES) << 8;
			cmd->response[0] |= readw(priv->regs + MSC_RES) & 0xff;
		}
	}
	if (data) {
		if (data->flags & MMC_DATA_WRITE)
			jz_mmc_write_data(priv, data);
		else if (data->flags & MMC_DATA_READ) {
			ret = jz_mmc_read_data(priv, data);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int jz_mmc_set_ios(struct mmc *mmc, struct jz_mmc_priv *priv)
{
	u32 real_rate = jz_mmc_clock_rate();
	u8 clk_div = 0;

	/* calculate clock divide */
	while ((real_rate > mmc->clock) && (clk_div < 7)) {
		real_rate >>= 1;
		clk_div++;
	}
	writel(clk_div & MSC_CLKRT_CLK_RATE_MASK, priv->regs + MSC_CLKRT);

	/* set the bus width for the next command */
	priv->flags &= ~JZ_MMC_BUS_WIDTH_MASK;
	if (mmc->bus_width == 8)
		priv->flags |= JZ_MMC_BUS_WIDTH_8;
	else if (mmc->bus_width == 4)
		priv->flags |= JZ_MMC_BUS_WIDTH_4;
	else
		priv->flags |= JZ_MMC_BUS_WIDTH_1;

	return 0;
}

static int jz_mmc_core_init(struct mmc *mmc)
{
	struct jz_mmc_priv *priv = mmc->priv;
	int ret;

	/* Reset */
	writel(MSC_STRPCL_RESET, priv->regs + MSC_STRPCL);
	ret = wait_for_bit_le32(priv->regs + MSC_STAT,
				MSC_STAT_IS_RESETTING, false, 10000, false);
	if (ret)
		return ret;

	/* Maximum timeouts */
	writel(0xffff, priv->regs + MSC_RESTO);
	writel(0xffffffff, priv->regs + MSC_RDTO);

	/* Enable low power mode */
	writel(0x1, priv->regs + MSC_LPM);

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)

static int jz_mmc_legacy_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
				  struct mmc_data *data)
{
	struct jz_mmc_priv *priv = mmc->priv;

	return jz_mmc_send_cmd(mmc, priv, cmd, data);
}

static int jz_mmc_legacy_set_ios(struct mmc *mmc)
{
	struct jz_mmc_priv *priv = mmc->priv;

	return jz_mmc_set_ios(mmc, priv);
};

static const struct mmc_ops jz_msc_ops = {
	.send_cmd	= jz_mmc_legacy_send_cmd,
	.set_ios	= jz_mmc_legacy_set_ios,
	.init		= jz_mmc_core_init,
};

static struct jz_mmc_priv jz_mmc_priv_static;
static struct jz_mmc_plat jz_mmc_plat_static = {
	.cfg = {
		.name = "MSC",
		.ops = &jz_msc_ops,

		.voltages = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 |
			    MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 |
			    MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36,
		.host_caps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS,

		.f_min = 375000,
		.f_max = 48000000,
		.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT,
	},
};

int jz_mmc_init(void __iomem *base)
{
	struct mmc *mmc;

	jz_mmc_priv_static.regs = base;

	mmc = mmc_create(&jz_mmc_plat_static.cfg, &jz_mmc_priv_static);

	return mmc ? 0 : -ENODEV;
}

#else /* CONFIG_DM_MMC */

#include <dm.h>
DECLARE_GLOBAL_DATA_PTR;

static int jz_mmc_dm_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	struct jz_mmc_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	return jz_mmc_send_cmd(mmc, priv, cmd, data);
}

static int jz_mmc_dm_set_ios(struct udevice *dev)
{
	struct jz_mmc_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	return jz_mmc_set_ios(mmc, priv);
};

static const struct dm_mmc_ops jz_msc_ops = {
	.send_cmd	= jz_mmc_dm_send_cmd,
	.set_ios	= jz_mmc_dm_set_ios,
};

static int jz_mmc_ofdata_to_platdata(struct udevice *dev)
{
	struct jz_mmc_priv *priv = dev_get_priv(dev);
	struct jz_mmc_plat *plat = dev_get_platdata(dev);
	struct mmc_config *cfg;
	int ret;

	priv->regs = map_physmem(devfdt_get_addr(dev), 0x100, MAP_NOCACHE);
	cfg = &plat->cfg;

	cfg->name = "MSC";
	cfg->host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS;

	ret = mmc_of_parse(dev, cfg);
	if (ret < 0) {
		dev_err(dev, "failed to parse host caps\n");
		return ret;
	}

	cfg->f_min = 400000;
	cfg->f_max = 52000000;

	cfg->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	cfg->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	return 0;
}

static int jz_mmc_bind(struct udevice *dev)
{
	struct jz_mmc_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static int jz_mmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct jz_mmc_priv *priv = dev_get_priv(dev);
	struct jz_mmc_plat *plat = dev_get_platdata(dev);

	plat->mmc.priv = priv;
	upriv->mmc = &plat->mmc;
	return jz_mmc_core_init(&plat->mmc);
}

static const struct udevice_id jz_mmc_ids[] = {
	{ .compatible = "ingenic,jz4780-mmc" },
	{ }
};

U_BOOT_DRIVER(jz_mmc_drv) = {
	.name			= "jz_mmc",
	.id			= UCLASS_MMC,
	.of_match		= jz_mmc_ids,
	.ofdata_to_platdata	= jz_mmc_ofdata_to_platdata,
	.bind			= jz_mmc_bind,
	.probe			= jz_mmc_probe,
	.priv_auto_alloc_size	= sizeof(struct jz_mmc_priv),
	.platdata_auto_alloc_size = sizeof(struct jz_mmc_plat),
	.ops			= &jz_msc_ops,
};
#endif /* CONFIG_DM_MMC */
