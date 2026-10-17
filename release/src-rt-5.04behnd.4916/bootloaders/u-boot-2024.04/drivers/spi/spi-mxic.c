// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2021 Macronix International Co., Ltd.
 *
 * Authors:
 *	zhengxunli <zhengxunli@mxic.com.tw>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <malloc.h>
#include <spi.h>
#include <spi-mem.h>
#include <linux/bug.h>
#include <linux/iopoll.h>

#define HC_CFG			0x0
#define HC_CFG_IF_CFG(x)	((x) << 27)
#define HC_CFG_DUAL_SLAVE	BIT(31)
#define HC_CFG_INDIVIDUAL	BIT(30)
#define HC_CFG_NIO(x)		(((x) / 4) << 27)
#define HC_CFG_TYPE(s, t)	((t) << (23 + ((s) * 2)))
#define HC_CFG_TYPE_SPI_NOR	0
#define HC_CFG_TYPE_SPI_NAND	1
#define HC_CFG_TYPE_SPI_RAM	2
#define HC_CFG_TYPE_RAW_NAND	3
#define HC_CFG_SLV_ACT(x)	((x) << 21)
#define HC_CFG_CLK_PH_EN	BIT(20)
#define HC_CFG_CLK_POL_INV	BIT(19)
#define HC_CFG_BIG_ENDIAN	BIT(18)
#define HC_CFG_DATA_PASS	BIT(17)
#define HC_CFG_IDLE_SIO_LVL(x)	((x) << 16)
#define HC_CFG_MAN_START_EN	BIT(3)
#define HC_CFG_MAN_START	BIT(2)
#define HC_CFG_MAN_CS_EN	BIT(1)
#define HC_CFG_MAN_CS_ASSERT	BIT(0)

#define INT_STS			0x4
#define INT_STS_EN		0x8
#define INT_SIG_EN		0xc
#define INT_STS_ALL		GENMASK(31, 0)
#define INT_RDY_PIN		BIT(26)
#define INT_RDY_SR		BIT(25)
#define INT_LNR_SUSP		BIT(24)
#define INT_ECC_ERR		BIT(17)
#define INT_CRC_ERR		BIT(16)
#define INT_LWR_DIS		BIT(12)
#define INT_LRD_DIS		BIT(11)
#define INT_SDMA_INT		BIT(10)
#define INT_DMA_FINISH		BIT(9)
#define INT_RX_NOT_FULL		BIT(3)
#define INT_RX_NOT_EMPTY	BIT(2)
#define INT_TX_NOT_FULL		BIT(1)
#define INT_TX_EMPTY		BIT(0)

#define HC_EN			0x10
#define HC_EN_BIT		BIT(0)

#define TXD(x)			(0x14 + ((x) * 4))
#define RXD			0x24

#define SS_CTRL(s)		(0x30 + ((s) * 4))
#define LRD_CFG			0x44
#define LWR_CFG			0x80
#define RWW_CFG			0x70
#define OP_READ			BIT(23)
#define OP_DUMMY_CYC(x)		((x) << 17)
#define OP_ADDR_BYTES(x)	((x) << 14)
#define OP_CMD_BYTES(x)		(((x) - 1) << 13)
#define OP_OCTA_CRC_EN		BIT(12)
#define OP_DQS_EN		BIT(11)
#define OP_ENHC_EN		BIT(10)
#define OP_PREAMBLE_EN		BIT(9)
#define OP_DATA_DDR		BIT(8)
#define OP_DATA_BUSW(x)		((x) << 6)
#define OP_ADDR_DDR		BIT(5)
#define OP_ADDR_BUSW(x)		((x) << 3)
#define OP_CMD_DDR		BIT(2)
#define OP_CMD_BUSW(x)		(x)
#define OP_BUSW_1		0
#define OP_BUSW_2		1
#define OP_BUSW_4		2
#define OP_BUSW_8		3

#define OCTA_CRC		0x38
#define OCTA_CRC_IN_EN(s)	BIT(3 + ((s) * 16))
#define OCTA_CRC_CHUNK(s, x)	((fls((x) / 32)) << (1 + ((s) * 16)))
#define OCTA_CRC_OUT_EN(s)	BIT(0 + ((s) * 16))

#define ONFI_DIN_CNT(s)		(0x3c + (s))

#define LRD_CTRL		0x48
#define RWW_CTRL		0x74
#define LWR_CTRL		0x84
#define LMODE_EN		BIT(31)
#define LMODE_SLV_ACT(x)	((x) << 21)
#define LMODE_CMD1(x)		((x) << 8)
#define LMODE_CMD0(x)		(x)

#define LRD_ADDR		0x4c
#define LWR_ADDR		0x88
#define LRD_RANGE		0x50
#define LWR_RANGE		0x8c

#define AXI_SLV_ADDR		0x54

#define DMAC_RD_CFG		0x58
#define DMAC_WR_CFG		0x94
#define DMAC_CFG_PERIPH_EN	BIT(31)
#define DMAC_CFG_ALLFLUSH_EN	BIT(30)
#define DMAC_CFG_LASTFLUSH_EN	BIT(29)
#define DMAC_CFG_QE(x)		(((x) + 1) << 16)
#define DMAC_CFG_BURST_LEN(x)	(((x) + 1) << 12)
#define DMAC_CFG_BURST_SZ(x)	((x) << 8)
#define DMAC_CFG_DIR_READ	BIT(1)
#define DMAC_CFG_START		BIT(0)

#define DMAC_RD_CNT		0x5c
#define DMAC_WR_CNT		0x98

#define SDMA_ADDR		0x60

#define DMAM_CFG		0x64
#define DMAM_CFG_START		BIT(31)
#define DMAM_CFG_CONT		BIT(30)
#define DMAM_CFG_SDMA_GAP(x)	(fls((x) / 8192) << 2)
#define DMAM_CFG_DIR_READ	BIT(1)
#define DMAM_CFG_EN		BIT(0)

#define DMAM_CNT		0x68

#define LNR_TIMER_TH		0x6c

#define RDM_CFG0		0x78
#define RDM_CFG0_POLY(x)	(x)

#define RDM_CFG1		0x7c
#define RDM_CFG1_RDM_EN		BIT(31)
#define RDM_CFG1_SEED(x)	(x)

#define LWR_SUSP_CTRL		0x90
#define LWR_SUSP_CTRL_EN	BIT(31)

#define DMAS_CTRL		0x9c
#define DMAS_CTRL_EN		BIT(31)
#define DMAS_CTRL_DIR_READ	BIT(30)

#define DATA_STROB		0xa0
#define DATA_STROB_EDO_EN	BIT(2)
#define DATA_STROB_INV_POL	BIT(1)
#define DATA_STROB_DELAY_2CYC	BIT(0)

#define IDLY_CODE(x)		(0xa4 + ((x) * 4))
#define IDLY_CODE_VAL(x, v)	((v) << (((x) % 4) * 8))

#define GPIO			0xc4
#define GPIO_PT(x)		BIT(3 + ((x) * 16))
#define GPIO_RESET(x)		BIT(2 + ((x) * 16))
#define GPIO_HOLDB(x)		BIT(1 + ((x) * 16))
#define GPIO_WPB(x)		BIT((x) * 16)

#define HC_VER			0xd0

#define HW_TEST(x)		(0xe0 + ((x) * 4))

struct mxic_spi_priv {
	struct clk *send_clk;
	struct clk *send_dly_clk;
	void __iomem *regs;
	u32 cur_speed_hz;
};

static int mxic_spi_clk_enable(struct mxic_spi_priv *priv)
{
	int ret;

	ret = clk_prepare_enable(priv->send_clk);
	if (ret)
		return ret;

	ret = clk_prepare_enable(priv->send_dly_clk);
	if (ret)
		goto err_send_dly_clk;

	return ret;

err_send_dly_clk:
	clk_disable_unprepare(priv->send_clk);

	return ret;
}

static void mxic_spi_clk_disable(struct mxic_spi_priv *priv)
{
	clk_disable_unprepare(priv->send_clk);
	clk_disable_unprepare(priv->send_dly_clk);
}

static void mxic_spi_set_input_delay_dqs(struct mxic_spi_priv *priv,
					 u8 idly_code)
{
	writel(IDLY_CODE_VAL(0, idly_code) |
	       IDLY_CODE_VAL(1, idly_code) |
	       IDLY_CODE_VAL(2, idly_code) |
	       IDLY_CODE_VAL(3, idly_code),
	       priv->regs + IDLY_CODE(0));
	writel(IDLY_CODE_VAL(4, idly_code) |
	       IDLY_CODE_VAL(5, idly_code) |
	       IDLY_CODE_VAL(6, idly_code) |
	       IDLY_CODE_VAL(7, idly_code),
	       priv->regs + IDLY_CODE(1));
}

static int mxic_spi_clk_setup(struct mxic_spi_priv *priv, uint freq)
{
	int ret;

	ret = clk_set_rate(priv->send_clk, freq);
	if (ret)
		return ret;

	ret = clk_set_rate(priv->send_dly_clk, freq);
	if (ret)
		return ret;

	/*
	 * A constant delay range from 0x0 ~ 0x1F for input delay,
	 * the unit is 78 ps, the max input delay is 2.418 ns.
	 */
	mxic_spi_set_input_delay_dqs(priv, 0xf);

	return 0;
}

static int mxic_spi_set_speed(struct udevice *bus, uint freq)
{
	struct mxic_spi_priv *priv = dev_get_priv(bus);
	int ret;

	if (priv->cur_speed_hz == freq)
		return 0;

	mxic_spi_clk_disable(priv);
	ret = mxic_spi_clk_setup(priv, freq);
	if (ret)
		return ret;

	ret = mxic_spi_clk_enable(priv);
	if (ret)
		return ret;

	priv->cur_speed_hz = freq;

	return 0;
}

static int mxic_spi_set_mode(struct udevice *bus, uint mode)
{
	struct mxic_spi_priv *priv = dev_get_priv(bus);
	u32 hc_config = 0;

	if (mode & SPI_CPHA)
		hc_config |= HC_CFG_CLK_PH_EN;
	if (mode & SPI_CPOL)
		hc_config |= HC_CFG_CLK_POL_INV;

	writel(hc_config, priv->regs + HC_CFG);

	return 0;
}

static void mxic_spi_hw_init(struct mxic_spi_priv *priv)
{
	writel(0, priv->regs + DATA_STROB);
	writel(INT_STS_ALL, priv->regs + INT_STS_EN);
	writel(0, priv->regs + HC_EN);
	writel(0, priv->regs + LRD_CFG);
	writel(0, priv->regs + LRD_CTRL);
	writel(HC_CFG_NIO(1) | HC_CFG_TYPE(0, HC_CFG_TYPE_SPI_NOR) |
	       HC_CFG_SLV_ACT(0) | HC_CFG_MAN_CS_EN | HC_CFG_IDLE_SIO_LVL(1),
	       priv->regs + HC_CFG);
}

static int mxic_spi_data_xfer(struct mxic_spi_priv *priv, const void *txbuf,
			      void *rxbuf, unsigned int len)
{
	unsigned int pos = 0;

	while (pos < len) {
		unsigned int nbytes = len - pos;
		u32 data = 0xffffffff;
		u32 sts;
		int ret;

		if (nbytes > 4)
			nbytes = 4;

		if (txbuf)
			memcpy(&data, txbuf + pos, nbytes);

		ret = readl_poll_timeout(priv->regs + INT_STS, sts,
					 sts & INT_TX_EMPTY, 1000000);
		if (ret)
			return ret;

		writel(data, priv->regs + TXD(nbytes % 4));

		if (rxbuf) {
			ret = readl_poll_timeout(priv->regs + INT_STS, sts,
						 sts & INT_TX_EMPTY,
						 1000000);
			if (ret)
				return ret;

			ret = readl_poll_timeout(priv->regs + INT_STS, sts,
						 sts & INT_RX_NOT_EMPTY,
						 1000000);
			if (ret)
				return ret;

			data = readl(priv->regs + RXD);
			data >>= (8 * (4 - nbytes));
			memcpy(rxbuf + pos, &data, nbytes);
			WARN_ON(readl(priv->regs + INT_STS) & INT_RX_NOT_EMPTY);
		} else {
			readl(priv->regs + RXD);
		}
		WARN_ON(readl(priv->regs + INT_STS) & INT_RX_NOT_EMPTY);

		pos += nbytes;
	}

	return 0;
}

static bool mxic_spi_mem_supports_op(struct spi_slave *slave,
				     const struct spi_mem_op *op)
{
	if (op->data.buswidth > 8 || op->addr.buswidth > 8 ||
	    op->dummy.buswidth > 8 || op->cmd.buswidth > 8)
		return false;

	if (op->addr.nbytes > 7)
		return false;

	return spi_mem_default_supports_op(slave, op);
}

static int mxic_spi_mem_exec_op(struct spi_slave *slave,
				const struct spi_mem_op *op)
{
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(slave->dev);
	struct udevice *bus = slave->dev->parent;
	struct mxic_spi_priv *priv = dev_get_priv(bus);
	int nio = 1, i, ret;
	u32 ss_ctrl;
	u8 addr[8], dummy_bytes = 0;

	if (slave->mode & (SPI_TX_OCTAL | SPI_RX_OCTAL))
		nio = 8;
	else if (slave->mode & (SPI_TX_QUAD | SPI_RX_QUAD))
		nio = 4;
	else if (slave->mode & (SPI_TX_DUAL | SPI_RX_DUAL))
		nio = 2;

	writel(HC_CFG_NIO(nio) |
	       HC_CFG_TYPE(slave_plat->cs, HC_CFG_TYPE_SPI_NOR) |
	       HC_CFG_SLV_ACT(slave_plat->cs) | HC_CFG_IDLE_SIO_LVL(1) |
	       HC_CFG_MAN_CS_EN,
	       priv->regs + HC_CFG);
	writel(HC_EN_BIT, priv->regs + HC_EN);

	ss_ctrl = OP_CMD_BYTES(1) | OP_CMD_BUSW(fls(op->cmd.buswidth) - 1);

	if (op->addr.nbytes)
		ss_ctrl |= OP_ADDR_BYTES(op->addr.nbytes) |
			   OP_ADDR_BUSW(fls(op->addr.buswidth) - 1);

	/*
	 * Since the SPI MXIC dummy buswidth is aligned with the data buswidth,
	 * the dummy byte needs to be recalculated to send out the correct
	 * dummy cycle.
	 */
	if (op->dummy.nbytes) {
		dummy_bytes = op->dummy.nbytes /
			      op->addr.buswidth *
			      op->data.buswidth;
		ss_ctrl |= OP_DUMMY_CYC(dummy_bytes);
	}

	if (op->data.nbytes) {
		ss_ctrl |= OP_DATA_BUSW(fls(op->data.buswidth) - 1);
		if (op->data.dir == SPI_MEM_DATA_IN)
			ss_ctrl |= OP_READ;
	}

	writel(ss_ctrl, priv->regs + SS_CTRL(slave_plat->cs));

	writel(readl(priv->regs + HC_CFG) | HC_CFG_MAN_CS_ASSERT,
	       priv->regs + HC_CFG);

	ret = mxic_spi_data_xfer(priv, &op->cmd.opcode, NULL, 1);
	if (ret)
		goto out;

	for (i = 0; i < op->addr.nbytes; i++)
		addr[i] = op->addr.val >> (8 * (op->addr.nbytes - i - 1));

	ret = mxic_spi_data_xfer(priv, addr, NULL, op->addr.nbytes);
	if (ret)
		goto out;

	ret = mxic_spi_data_xfer(priv, NULL, NULL, dummy_bytes);
	if (ret)
		goto out;

	ret = mxic_spi_data_xfer(priv,
				 op->data.dir == SPI_MEM_DATA_OUT ?
				 op->data.buf.out : NULL,
				 op->data.dir == SPI_MEM_DATA_IN ?
				 op->data.buf.in : NULL,
				 op->data.nbytes);

out:
	writel(readl(priv->regs + HC_CFG) & ~HC_CFG_MAN_CS_ASSERT,
	       priv->regs + HC_CFG);
	writel(0, priv->regs + HC_EN);

	return ret;
}

static const struct spi_controller_mem_ops mxic_spi_mem_ops = {
	.supports_op = mxic_spi_mem_supports_op,
	.exec_op = mxic_spi_mem_exec_op,
};

static int mxic_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct mxic_spi_priv *priv = dev_get_priv(bus);

	writel(readl(priv->regs + HC_CFG) | HC_CFG_MAN_CS_EN,
	       priv->regs + HC_CFG);
	writel(HC_EN_BIT, priv->regs + HC_EN);
	writel(readl(priv->regs + HC_CFG) | HC_CFG_MAN_CS_ASSERT,
	       priv->regs + HC_CFG);

	return 0;
}

static int mxic_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct mxic_spi_priv *priv = dev_get_priv(bus);

	writel(readl(priv->regs + HC_CFG) & ~HC_CFG_MAN_CS_ASSERT,
	       priv->regs + HC_CFG);
	writel(0, priv->regs + HC_EN);

	return 0;
}

static int mxic_spi_xfer(struct udevice *dev, unsigned int bitlen,
			 const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct mxic_spi_priv *priv = dev_get_priv(bus);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	unsigned int busw = OP_BUSW_1;
	unsigned int len = bitlen / 8;
	int ret;

	if (dout && din) {
		if (((slave->mode & SPI_TX_QUAD) &&
		     !(slave->mode & SPI_RX_QUAD)) ||
		    ((slave->mode & SPI_TX_DUAL) &&
		     !(slave->mode & SPI_RX_DUAL)))
			return -ENOTSUPP;
	}

	if (din) {
		if (slave->mode & SPI_TX_QUAD)
			busw = OP_BUSW_4;
		else if (slave->mode & SPI_TX_DUAL)
			busw = OP_BUSW_2;
	} else if (dout) {
		if (slave->mode & SPI_RX_QUAD)
			busw = OP_BUSW_4;
		else if (slave->mode & SPI_RX_DUAL)
			busw = OP_BUSW_2;
	}

	writel(OP_CMD_BYTES(1) | OP_CMD_BUSW(busw) |
	       OP_DATA_BUSW(busw) | (din ? OP_READ : 0),
	       priv->regs + SS_CTRL(0));

	ret = mxic_spi_data_xfer(priv, dout, din, len);
	if (ret)
		return ret;

	return 0;
}

static int mxic_spi_probe(struct udevice *bus)
{
	struct mxic_spi_priv *priv = dev_get_priv(bus);

	priv->regs = dev_read_addr_ptr(bus);

	priv->send_clk = devm_clk_get(bus, "send_clk");
	if (IS_ERR(priv->send_clk))
		return PTR_ERR(priv->send_clk);

	priv->send_dly_clk = devm_clk_get(bus, "send_dly_clk");
	if (IS_ERR(priv->send_dly_clk))
		return PTR_ERR(priv->send_dly_clk);

	mxic_spi_hw_init(priv);

	return 0;
}

static const struct dm_spi_ops mxic_spi_ops = {
	.claim_bus	= mxic_spi_claim_bus,
	.release_bus	= mxic_spi_release_bus,
	.xfer		= mxic_spi_xfer,
	.set_speed	= mxic_spi_set_speed,
	.set_mode	= mxic_spi_set_mode,
	.mem_ops	= &mxic_spi_mem_ops,
};

static const struct udevice_id mxic_spi_ids[] = {
	{ .compatible = "mxicy,mx25f0a-spi", },
	{ }
};

U_BOOT_DRIVER(mxic_spi) = {
	.name	= "mxic_spi",
	.id	= UCLASS_SPI,
	.of_match = mxic_spi_ids,
	.ops	= &mxic_spi_ops,
	.priv_auto = sizeof(struct mxic_spi_priv),
	.probe	= mxic_spi_probe,
};
