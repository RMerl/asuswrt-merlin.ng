// SPDX-License-Identifier: GPL-2.0
//
// Mediatek SPI-NOR controller driver
//
// Copyright (C) 2020 SkyLake Huang <SkyLake.Huang@mediatek.com>
//
// Some parts are based on drivers/spi/spi-mtk-nor.c of linux version

#include <clk.h>
#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/completion.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <spi.h>
#include <spi-mem.h>
#include <stdbool.h>
#include <watchdog.h>
#include <linux/dma-mapping.h>

#define DRIVER_NAME "mtk-spi-nor"

#define MTK_NOR_REG_CMD 0x00
#define MTK_NOR_CMD_WRSR BIT(5)
#define MTK_NOR_CMD_WRITE BIT(4)
#define MTK_NOR_CMD_PROGRAM BIT(2)
#define MTK_NOR_CMD_RDSR BIT(1)
#define MTK_NOR_CMD_READ BIT(0)
#define MTK_NOR_CMD_MASK GENMASK(5, 0)

#define MTK_NOR_REG_PRG_CNT 0x04
#define MTK_NOR_REG_RDSR 0x08
#define MTK_NOR_REG_RDATA 0x0c

#define MTK_NOR_REG_RADR0 0x10
#define MTK_NOR_REG_RADR(n) (MTK_NOR_REG_RADR0 + 4 * (n))
#define MTK_NOR_REG_RADR3 0xc8

#define MTK_NOR_REG_WDATA 0x1c

#define MTK_NOR_REG_PRGDATA0 0x20
#define MTK_NOR_REG_PRGDATA(n) (MTK_NOR_REG_PRGDATA0 + 4 * (n))
#define MTK_NOR_REG_PRGDATA_MAX 5

#define MTK_NOR_REG_SHIFT0 0x38
#define MTK_NOR_REG_SHIFT(n) (MTK_NOR_REG_SHIFT0 + 4 * (n))
#define MTK_NOR_REG_SHIFT_MAX 9

#define MTK_NOR_REG_CFG1 0x60
#define MTK_NOR_FAST_READ BIT(0)

#define MTK_NOR_REG_CFG2 0x64
#define MTK_NOR_WR_CUSTOM_OP_EN BIT(4)
#define MTK_NOR_WR_BUF_EN BIT(0)

#define MTK_NOR_REG_PP_DATA 0x98

#define MTK_NOR_REG_IRQ_STAT 0xa8
#define MTK_NOR_REG_IRQ_EN 0xac
#define MTK_NOR_IRQ_DMA BIT(7)
#define MTK_NOR_IRQ_WRSR BIT(5)
#define MTK_NOR_IRQ_MASK GENMASK(7, 0)

#define MTK_NOR_REG_CFG3 0xb4
#define MTK_NOR_DISABLE_WREN BIT(7)
#define MTK_NOR_DISABLE_SR_POLL BIT(5)

#define MTK_NOR_REG_WP 0xc4
#define MTK_NOR_ENABLE_SF_CMD 0x30

#define MTK_NOR_REG_BUSCFG 0xcc
#define MTK_NOR_4B_ADDR BIT(4)
#define MTK_NOR_QUAD_ADDR BIT(3)
#define MTK_NOR_QUAD_READ BIT(2)
#define MTK_NOR_DUAL_ADDR BIT(1)
#define MTK_NOR_DUAL_READ BIT(0)
#define MTK_NOR_BUS_MODE_MASK GENMASK(4, 0)

#define MTK_NOR_REG_DMA_CTL 0x718
#define MTK_NOR_DMA_START BIT(0)

#define MTK_NOR_REG_DMA_FADR 0x71c
#define MTK_NOR_REG_DMA_DADR 0x720
#define MTK_NOR_REG_DMA_END_DADR 0x724

#define MTK_NOR_PRG_MAX_SIZE 6
// Reading DMA src/dst addresses have to be 16-byte aligned
#define MTK_NOR_DMA_ALIGN 16
#define MTK_NOR_DMA_ALIGN_MASK (MTK_NOR_DMA_ALIGN - 1)
// and we allocate a bounce buffer if destination address isn't aligned.
#define MTK_NOR_BOUNCE_BUF_SIZE PAGE_SIZE

// Buffered page program can do one 128-byte transfer
#define MTK_NOR_PP_SIZE 128

#define CLK_TO_US(priv, clkcnt) DIV_ROUND_UP(clkcnt, (priv)->spi_freq / 1000000)

#define MTK_NOR_UNLOCK_ALL 0x0

struct mtk_snor_priv {
	struct device *dev;
	void __iomem *base;
	u8 *buffer;
	struct clk spi_clk;
	struct clk ctlr_clk;
	unsigned int spi_freq;
	bool wbuf_en;
};

static inline void mtk_snor_rmw(struct mtk_snor_priv *priv, u32 reg, u32 set,
				u32 clr)
{
	u32 val = readl(priv->base + reg);

	val &= ~clr;
	val |= set;
	writel(val, priv->base + reg);
}

static inline int mtk_snor_cmd_exec(struct mtk_snor_priv *priv, u32 cmd,
				    ulong clk)
{
	unsigned long long delay = CLK_TO_US(priv, clk);
	u32 reg;
	int ret;

	writel(cmd, priv->base + MTK_NOR_REG_CMD);
	delay = (delay + 1) * 200;
	ret = readl_poll_timeout(priv->base + MTK_NOR_REG_CMD, reg,
				 !(reg & cmd), delay);
	if (ret < 0)
		dev_err(priv->dev, "command %u timeout.\n", cmd);
	return ret;
}

static void mtk_snor_set_addr(struct mtk_snor_priv *priv,
			      const struct spi_mem_op *op)
{
	u32 addr = op->addr.val;
	int i;

	for (i = 0; i < 3; i++) {
		writeb(addr & 0xff, priv->base + MTK_NOR_REG_RADR(i));
		addr >>= 8;
	}
	if (op->addr.nbytes == 4) {
		writeb(addr & 0xff, priv->base + MTK_NOR_REG_RADR3);
		mtk_snor_rmw(priv, MTK_NOR_REG_BUSCFG, MTK_NOR_4B_ADDR, 0);
	} else {
		mtk_snor_rmw(priv, MTK_NOR_REG_BUSCFG, 0, MTK_NOR_4B_ADDR);
	}
}

static bool need_bounce(const struct spi_mem_op *op)
{
	return ((uintptr_t)op->data.buf.in & MTK_NOR_DMA_ALIGN_MASK);
}

static int mtk_snor_adjust_op_size(struct spi_slave *slave,
				   struct spi_mem_op *op)
{
	if (!op->data.nbytes)
		return 0;

	if (op->addr.nbytes == 3 || op->addr.nbytes == 4) {
		if (op->data.dir == SPI_MEM_DATA_IN) { //&&
			// limit size to prevent timeout calculation overflow
			if (op->data.nbytes > 0x400000)
				op->data.nbytes = 0x400000;
			if (op->addr.val & MTK_NOR_DMA_ALIGN_MASK ||
			    op->data.nbytes < MTK_NOR_DMA_ALIGN)
				op->data.nbytes = 1;
			else if (!need_bounce(op))
				op->data.nbytes &= ~MTK_NOR_DMA_ALIGN_MASK;
			else if (op->data.nbytes > MTK_NOR_BOUNCE_BUF_SIZE)
				op->data.nbytes = MTK_NOR_BOUNCE_BUF_SIZE;
			return 0;
		} else if (op->data.dir == SPI_MEM_DATA_OUT) {
			if (op->data.nbytes >= MTK_NOR_PP_SIZE)
				op->data.nbytes = MTK_NOR_PP_SIZE;
			else
				op->data.nbytes = 1;
			return 0;
		}
	}

	return 0;
}

static bool mtk_snor_supports_op(struct spi_slave *slave,
				 const struct spi_mem_op *op)
{
	/* This controller only supports 1-1-1 write mode */
	if (op->data.dir == SPI_MEM_DATA_OUT &&
	    (op->cmd.buswidth != 1 || op->data.buswidth != 1))
		return false;

	return true;
}

static void mtk_snor_setup_bus(struct mtk_snor_priv *priv,
			       const struct spi_mem_op *op)
{
	u32 reg = 0;

	if (op->addr.nbytes == 4)
		reg |= MTK_NOR_4B_ADDR;

	if (op->data.buswidth == 4) {
		reg |= MTK_NOR_QUAD_READ;
		writeb(op->cmd.opcode, priv->base + MTK_NOR_REG_PRGDATA(4));
		if (op->addr.buswidth == 4)
			reg |= MTK_NOR_QUAD_ADDR;
	} else if (op->data.buswidth == 2) {
		reg |= MTK_NOR_DUAL_READ;
		writeb(op->cmd.opcode, priv->base + MTK_NOR_REG_PRGDATA(3));
		if (op->addr.buswidth == 2)
			reg |= MTK_NOR_DUAL_ADDR;
	} else {
		if (op->cmd.opcode == 0x0b)
			mtk_snor_rmw(priv, MTK_NOR_REG_CFG1, MTK_NOR_FAST_READ,
				     0);
		else
			mtk_snor_rmw(priv, MTK_NOR_REG_CFG1, 0,
				     MTK_NOR_FAST_READ);
	}
	mtk_snor_rmw(priv, MTK_NOR_REG_BUSCFG, reg, MTK_NOR_BUS_MODE_MASK);
}

static int mtk_snor_dma_exec(struct mtk_snor_priv *priv, u32 from,
			     unsigned int length, dma_addr_t dma_addr)
{
	int ret = 0;
	ulong delay;
	u32 reg;

	writel(from, priv->base + MTK_NOR_REG_DMA_FADR);
	writel(dma_addr, priv->base + MTK_NOR_REG_DMA_DADR);
	writel(dma_addr + length, priv->base + MTK_NOR_REG_DMA_END_DADR);

	mtk_snor_rmw(priv, MTK_NOR_REG_DMA_CTL, MTK_NOR_DMA_START, 0);

	delay = CLK_TO_US(priv, (length + 5) * BITS_PER_BYTE);

	delay = (delay + 1) * 100;
	ret = readl_poll_timeout(priv->base + MTK_NOR_REG_DMA_CTL, reg,
				 !(reg & MTK_NOR_DMA_START), delay);

	if (ret < 0)
		dev_err(priv->dev, "dma read timeout.\n");

	return ret;
}

static int mtk_snor_read_bounce(struct mtk_snor_priv *priv,
				const struct spi_mem_op *op)
{
	unsigned int rdlen;
	int ret;

	if (op->data.nbytes & MTK_NOR_DMA_ALIGN_MASK)
		rdlen = (op->data.nbytes + MTK_NOR_DMA_ALIGN) &
			~MTK_NOR_DMA_ALIGN_MASK;
	else
		rdlen = op->data.nbytes;

	ret = mtk_snor_dma_exec(priv, op->addr.val, rdlen,
				(dma_addr_t)priv->buffer);

	if (!ret)
		memcpy(op->data.buf.in, priv->buffer, op->data.nbytes);

	return ret;
}

static int mtk_snor_read_dma(struct mtk_snor_priv *priv,
			     const struct spi_mem_op *op)
{
	int ret;
	dma_addr_t dma_addr;

	if (need_bounce(op))
		return mtk_snor_read_bounce(priv, op);

	dma_addr = dma_map_single(op->data.buf.in, op->data.nbytes,
				  DMA_FROM_DEVICE);

	if (dma_mapping_error(priv->dev, dma_addr))
		return -EINVAL;

	ret = mtk_snor_dma_exec(priv, op->addr.val, op->data.nbytes, dma_addr);

	dma_unmap_single(dma_addr, op->data.nbytes, DMA_FROM_DEVICE);

	return ret;
}

static int mtk_snor_read_pio(struct mtk_snor_priv *priv,
			     const struct spi_mem_op *op)
{
	u8 *buf = op->data.buf.in;
	int ret;

	ret = mtk_snor_cmd_exec(priv, MTK_NOR_CMD_READ, 6 * BITS_PER_BYTE);
	if (!ret)
		buf[0] = readb(priv->base + MTK_NOR_REG_RDATA);
	return ret;
}

static int mtk_snor_write_buffer_enable(struct mtk_snor_priv *priv)
{
	int ret;
	u32 val;

	if (priv->wbuf_en)
		return 0;

	val = readl(priv->base + MTK_NOR_REG_CFG2);
	writel(val | MTK_NOR_WR_BUF_EN, priv->base + MTK_NOR_REG_CFG2);
	ret = readl_poll_timeout(priv->base + MTK_NOR_REG_CFG2, val,
				 val & MTK_NOR_WR_BUF_EN, 10000);
	if (!ret)
		priv->wbuf_en = true;
	return ret;
}

static int mtk_snor_write_buffer_disable(struct mtk_snor_priv *priv)
{
	int ret;
	u32 val;

	if (!priv->wbuf_en)
		return 0;
	val = readl(priv->base + MTK_NOR_REG_CFG2);
	writel(val & ~MTK_NOR_WR_BUF_EN, priv->base + MTK_NOR_REG_CFG2);
	ret = readl_poll_timeout(priv->base + MTK_NOR_REG_CFG2, val,
				 !(val & MTK_NOR_WR_BUF_EN), 10000);
	if (!ret)
		priv->wbuf_en = false;
	return ret;
}

static int mtk_snor_pp_buffered(struct mtk_snor_priv *priv,
				const struct spi_mem_op *op)
{
	const u8 *buf = op->data.buf.out;
	u32 val;
	int ret, i;

	ret = mtk_snor_write_buffer_enable(priv);
	if (ret < 0)
		return ret;

	for (i = 0; i < op->data.nbytes; i += 4) {
		val = buf[i + 3] << 24 | buf[i + 2] << 16 | buf[i + 1] << 8 |
		      buf[i];
		writel(val, priv->base + MTK_NOR_REG_PP_DATA);
	}
	mtk_snor_cmd_exec(priv, MTK_NOR_CMD_WRITE,
			  (op->data.nbytes + 5) * BITS_PER_BYTE);
	return mtk_snor_write_buffer_disable(priv);
}

static int mtk_snor_pp_unbuffered(struct mtk_snor_priv *priv,
				  const struct spi_mem_op *op)
{
	const u8 *buf = op->data.buf.out;
	int ret;

	ret = mtk_snor_write_buffer_disable(priv);
	if (ret < 0)
		return ret;
	writeb(buf[0], priv->base + MTK_NOR_REG_WDATA);
	return mtk_snor_cmd_exec(priv, MTK_NOR_CMD_WRITE, 6 * BITS_PER_BYTE);
}

static int mtk_snor_cmd_program(struct mtk_snor_priv *priv,
				const struct spi_mem_op *op)
{
	u32 tx_len = 0;
	u32 trx_len = 0;
	int reg_offset = MTK_NOR_REG_PRGDATA_MAX;
	void __iomem *reg;
	u8 *txbuf;
	int tx_cnt = 0;
	u8 *rxbuf = op->data.buf.in;
	int i = 0;

	tx_len = 1 + op->addr.nbytes + op->dummy.nbytes;
	trx_len = tx_len + op->data.nbytes;
	if (op->data.dir == SPI_MEM_DATA_OUT)
		tx_len += op->data.nbytes;

	txbuf = kmalloc_array(tx_len, sizeof(u8), GFP_KERNEL);
	memset(txbuf, 0x0, tx_len * sizeof(u8));

	/* Join all bytes to be transferred */
	txbuf[tx_cnt] = op->cmd.opcode;
	tx_cnt++;
	for (i = op->addr.nbytes; i > 0; i--, tx_cnt++)
		txbuf[tx_cnt] = ((u8 *)&op->addr.val)[i - 1];
	for (i = op->dummy.nbytes; i > 0; i--, tx_cnt++)
		txbuf[tx_cnt] = 0x0;
	if (op->data.dir == SPI_MEM_DATA_OUT)
		for (i = op->data.nbytes; i > 0; i--, tx_cnt++)
			txbuf[tx_cnt] = ((u8 *)op->data.buf.out)[i - 1];

	for (i = MTK_NOR_REG_PRGDATA_MAX; i >= 0; i--)
		writeb(0, priv->base + MTK_NOR_REG_PRGDATA(i));

	for (i = 0; i < tx_len; i++, reg_offset--)
		writeb(txbuf[i], priv->base + MTK_NOR_REG_PRGDATA(reg_offset));

	kfree(txbuf);

	writel(trx_len * BITS_PER_BYTE, priv->base + MTK_NOR_REG_PRG_CNT);

	mtk_snor_cmd_exec(priv, MTK_NOR_CMD_PROGRAM, trx_len * BITS_PER_BYTE);

	reg_offset = op->data.nbytes - 1;
	for (i = 0; i < op->data.nbytes; i++, reg_offset--) {
		reg = priv->base + MTK_NOR_REG_SHIFT(reg_offset);
		rxbuf[i] = readb(reg);
	}

	return 0;
}

static int mtk_snor_exec_op(struct spi_slave *slave,
			    const struct spi_mem_op *op)
{
	struct udevice *bus = dev_get_parent(slave->dev);
	struct mtk_snor_priv *priv = dev_get_priv(bus);
	int ret;

	if (op->data.dir == SPI_MEM_NO_DATA || op->addr.nbytes == 0) {
		return mtk_snor_cmd_program(priv, op);
	} else if (op->data.dir == SPI_MEM_DATA_OUT) {
		mtk_snor_set_addr(priv, op);
		writeb(op->cmd.opcode, priv->base + MTK_NOR_REG_PRGDATA0);
		if (op->data.nbytes == MTK_NOR_PP_SIZE)
			return mtk_snor_pp_buffered(priv, op);
		return mtk_snor_pp_unbuffered(priv, op);
	} else if (op->data.dir == SPI_MEM_DATA_IN) {
		ret = mtk_snor_write_buffer_disable(priv);
		if (ret < 0)
			return ret;
		mtk_snor_setup_bus(priv, op);
		if (op->data.nbytes == 1) {
			mtk_snor_set_addr(priv, op);
			return mtk_snor_read_pio(priv, op);
		} else {
			return mtk_snor_read_dma(priv, op);
		}
	}

	return -ENOTSUPP;
}

static int mtk_snor_probe(struct udevice *bus)
{
	struct mtk_snor_priv *priv = dev_get_priv(bus);
	u8 *buffer;
	int ret;
	u32 reg;

	priv->base = devfdt_get_addr_ptr(bus);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_name(bus, "spi", &priv->spi_clk);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(bus, "sf", &priv->ctlr_clk);
	if (ret < 0)
		return ret;

	buffer = devm_kmalloc(bus, MTK_NOR_BOUNCE_BUF_SIZE + MTK_NOR_DMA_ALIGN,
			      GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;
	if ((ulong)buffer & MTK_NOR_DMA_ALIGN_MASK)
		buffer = (u8 *)(((ulong)buffer + MTK_NOR_DMA_ALIGN) &
				~MTK_NOR_DMA_ALIGN_MASK);
	priv->buffer = buffer;

	clk_enable(&priv->spi_clk);
	clk_enable(&priv->ctlr_clk);

	priv->spi_freq = clk_get_rate(&priv->spi_clk);
	printf("spi frequency: %d Hz\n", priv->spi_freq);

	/* With this setting, we issue one command at a time to
	 * accommodate to SPI-mem framework.
	 */
	writel(MTK_NOR_ENABLE_SF_CMD, priv->base + MTK_NOR_REG_WP);
	mtk_snor_rmw(priv, MTK_NOR_REG_CFG2, MTK_NOR_WR_CUSTOM_OP_EN, 0);
	mtk_snor_rmw(priv, MTK_NOR_REG_CFG3,
		     MTK_NOR_DISABLE_WREN | MTK_NOR_DISABLE_SR_POLL, 0);

	/* Unlock all blocks using write status command.
	 * SPI-MEM hasn't implemented unlock procedure on MXIC devices.
	 * We may remove this later.
	 */
	writel(2 * BITS_PER_BYTE, priv->base + MTK_NOR_REG_PRG_CNT);
	writel(MTK_NOR_UNLOCK_ALL, priv->base + MTK_NOR_REG_PRGDATA(5));
	writel(MTK_NOR_IRQ_WRSR, priv->base + MTK_NOR_REG_IRQ_EN);
	writel(MTK_NOR_CMD_WRSR, priv->base + MTK_NOR_REG_CMD);
	ret = readl_poll_timeout(priv->base + MTK_NOR_REG_IRQ_STAT, reg,
				 !(reg & MTK_NOR_IRQ_WRSR),
				 ((3 * BITS_PER_BYTE) + 1) * 200);

	return 0;
}

static int mtk_snor_set_speed(struct udevice *bus, uint speed)
{
	/* MTK's SNOR controller does not have a bus clock divider.
	 * We setup maximum bus clock in dts.
	 */

	return 0;
}

static int mtk_snor_set_mode(struct udevice *bus, uint mode)
{
	/* We set up mode later for each transmission.
	 */
	return 0;
}

static const struct spi_controller_mem_ops mtk_snor_mem_ops = {
	.adjust_op_size = mtk_snor_adjust_op_size,
	.supports_op = mtk_snor_supports_op,
	.exec_op = mtk_snor_exec_op
};

static const struct dm_spi_ops mtk_snor_ops = {
	.mem_ops = &mtk_snor_mem_ops,
	.set_speed = mtk_snor_set_speed,
	.set_mode = mtk_snor_set_mode,
};

static const struct udevice_id mtk_snor_ids[] = {
	{ .compatible = "mediatek,mtk-snor" },
	{}
};

U_BOOT_DRIVER(mtk_snor) = {
	.name = "mtk_snor",
	.id = UCLASS_SPI,
	.of_match = mtk_snor_ids,
	.ops = &mtk_snor_ops,
	.priv_auto = sizeof(struct mtk_snor_priv),
	.probe = mtk_snor_probe,
};
