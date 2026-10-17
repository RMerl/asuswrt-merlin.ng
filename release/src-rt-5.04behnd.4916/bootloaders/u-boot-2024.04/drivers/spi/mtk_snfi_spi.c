// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <spi.h>
#include <spi-mem.h>
#include <stdbool.h>
#include <watchdog.h>
#include <dm/pinctrl.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/iopoll.h>

#define SNFI_MAC_CTL			0x500
#define MAC_XIO_SEL			BIT(4)
#define SF_MAC_EN			BIT(3)
#define SF_TRIG				BIT(2)
#define WIP_READY			BIT(1)
#define WIP				BIT(0)

#define SNFI_MAC_OUTL			0x504
#define SNFI_MAC_INL			0x508

#define SNFI_MISC_CTL			0x538
#define SW_RST				BIT(28)
#define FIFO_RD_LTC_SHIFT		25
#define FIFO_RD_LTC			GENMASK(26, 25)
#define LATCH_LAT_SHIFT			8
#define LATCH_LAT			GENMASK(9, 8)
#define CS_DESELECT_CYC_SHIFT		0
#define CS_DESELECT_CYC			GENMASK(4, 0)

#define SNF_STA_CTL1			0x550
#define SPI_STATE			GENMASK(3, 0)

#define SNFI_GPRAM_OFFSET		0x800
#define SNFI_GPRAM_SIZE			0x80

#define SNFI_POLL_INTERVAL		500000
#define SNFI_RST_POLL_INTERVAL		1000000

struct mtk_snfi_priv {
	void __iomem *base;

	struct clk nfi_clk;
	struct clk pad_clk;
};

static int mtk_snfi_adjust_op_size(struct spi_slave *slave,
				   struct spi_mem_op *op)
{
	u32 nbytes;

	/*
	 * When there is input data, it will be appended after the output
	 * data in the GPRAM. So the total size of either pure output data
	 * or the output+input data must not exceed the GPRAM size.
	 */

	nbytes = op->cmd.nbytes + op->addr.nbytes + op->dummy.nbytes;

	if (nbytes + op->data.nbytes <= SNFI_GPRAM_SIZE)
		return 0;

	if (nbytes >= SNFI_GPRAM_SIZE)
		return -ENOTSUPP;

	op->data.nbytes = SNFI_GPRAM_SIZE - nbytes;

	return 0;
}

static bool mtk_snfi_supports_op(struct spi_slave *slave,
				 const struct spi_mem_op *op)
{
	if (op->cmd.buswidth > 1 || op->addr.buswidth > 1 ||
	    op->dummy.buswidth > 1 || op->data.buswidth > 1)
		return false;

	return true;
}

static int mtk_snfi_mac_trigger(struct mtk_snfi_priv *priv,
				struct udevice *bus, u32 outlen, u32 inlen)
{
	int ret;
	u32 val;

#ifdef CONFIG_PINCTRL
	pinctrl_select_state(bus, "snfi");
#endif

	writel(SF_MAC_EN, priv->base + SNFI_MAC_CTL);
	writel(outlen, priv->base + SNFI_MAC_OUTL);
	writel(inlen, priv->base + SNFI_MAC_INL);

	writel(SF_MAC_EN | SF_TRIG, priv->base + SNFI_MAC_CTL);

	ret = readl_poll_timeout(priv->base + SNFI_MAC_CTL, val,
				 val & WIP_READY, SNFI_POLL_INTERVAL);
	if (ret) {
		printf("%s: timed out waiting for WIP_READY\n", __func__);
		goto cleanup;
	}

	ret = readl_poll_timeout(priv->base + SNFI_MAC_CTL, val,
				 !(val & WIP), SNFI_POLL_INTERVAL);
	if (ret)
		printf("%s: timed out waiting for WIP cleared\n", __func__);

	writel(0, priv->base + SNFI_MAC_CTL);

cleanup:
#ifdef CONFIG_PINCTRL
	pinctrl_select_state(bus, "default");
#endif

	return ret;
}

static int mtk_snfi_mac_reset(struct mtk_snfi_priv *priv)
{
	int ret;
	u32 val;

	setbits_32(priv->base + SNFI_MISC_CTL, SW_RST);

	ret = readl_poll_timeout(priv->base + SNF_STA_CTL1, val,
				 !(val & SPI_STATE), SNFI_POLL_INTERVAL);
	if (ret)
		printf("%s: failed to reset snfi mac\n", __func__);

	writel((2 << FIFO_RD_LTC_SHIFT) |
		(10 << CS_DESELECT_CYC_SHIFT),
		priv->base + SNFI_MISC_CTL);

	return ret;
}

static void mtk_snfi_copy_to_gpram(struct mtk_snfi_priv *priv,
				   const void *data, size_t len)
{
	void __iomem *gpram = priv->base + SNFI_GPRAM_OFFSET;
	size_t i, n = (len + sizeof(u32) - 1) / sizeof(u32);
	const u32 *buff = data;

	/*
	 * The output data will always be copied to the beginning of
	 * the GPRAM. Uses word write for better performance.
	 *
	 * Trailing bytes in the last word are not cared.
	 */

	for (i = 0; i < n; i++)
		writel(buff[i], gpram + i * sizeof(u32));
}

static void mtk_snfi_copy_from_gpram(struct mtk_snfi_priv *priv, u8 *cache,
				     void *data, size_t pos, size_t len)
{
	void __iomem *gpram = priv->base + SNFI_GPRAM_OFFSET;
	u32 *buff = (u32 *)cache;
	size_t i, off, end;

	/* Start position in the buffer */
	off = pos & (sizeof(u32) - 1);

	/* End position for copy */
	end = (len + pos + sizeof(u32) - 1) & (~(sizeof(u32) - 1));

	/* Start position for copy */
	pos &= ~(sizeof(u32) - 1);

	/*
	 * Read aligned data from GPRAM to buffer first.
	 * Uses word read for better performance.
	 */
	i = 0;
	while (pos < end) {
		buff[i++] = readl(gpram + pos);
		pos += sizeof(u32);
	}

	/* Copy rx data */
	memcpy(data, cache + off, len);
}

static int mtk_snfi_exec_op(struct spi_slave *slave,
			    const struct spi_mem_op *op)
{
	struct udevice *bus = dev_get_parent(slave->dev);
	struct mtk_snfi_priv *priv = dev_get_priv(bus);
	u8 gpram_cache[SNFI_GPRAM_SIZE];
	u32 i, len = 0, inlen = 0;
	int addr_sh;
	int ret;

	schedule();

	ret = mtk_snfi_mac_reset(priv);
	if (ret)
		return ret;

	/* Put opcode */
	gpram_cache[len++] = op->cmd.opcode;

	/* Put address */
	addr_sh = (op->addr.nbytes - 1) * 8;
	while (addr_sh >= 0) {
		gpram_cache[len++] = (op->addr.val >> addr_sh) & 0xff;
		addr_sh -= 8;
	}

	/* Put dummy bytes */
	for (i = 0; i < op->dummy.nbytes; i++)
		gpram_cache[len++] = 0;

	/* Put output data */
	if (op->data.nbytes && op->data.dir == SPI_MEM_DATA_OUT) {
		memcpy(gpram_cache + len, op->data.buf.out, op->data.nbytes);
		len += op->data.nbytes;
	}

	/* Copy final output data to GPRAM */
	mtk_snfi_copy_to_gpram(priv, gpram_cache, len);

	/* Start one SPI transaction */
	if (op->data.nbytes && op->data.dir == SPI_MEM_DATA_IN)
		inlen = op->data.nbytes;

	ret = mtk_snfi_mac_trigger(priv, bus, len, inlen);
	if (ret)
		return ret;

	/* Copy input data from GPRAM */
	if (inlen)
		mtk_snfi_copy_from_gpram(priv, gpram_cache, op->data.buf.in,
					 len, inlen);

	return 0;
}

static int mtk_snfi_spi_probe(struct udevice *bus)
{
	struct mtk_snfi_priv *priv = dev_get_priv(bus);
	int ret;

	priv->base = dev_read_addr_ptr(bus);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_name(bus, "nfi_clk", &priv->nfi_clk);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(bus, "pad_clk", &priv->pad_clk);
	if (ret < 0)
		return ret;

	clk_enable(&priv->nfi_clk);
	clk_enable(&priv->pad_clk);

	return 0;
}

static int mtk_snfi_set_speed(struct udevice *bus, uint speed)
{
	/*
	 * The SNFI does not have a bus clock divider.
	 * The bus clock is set in dts (pad_clk, UNIVPLL2_D8 = 50MHz).
	 */

	return 0;
}

static int mtk_snfi_set_mode(struct udevice *bus, uint mode)
{
	/* The SNFI supports only mode 0 */

	if (mode)
		return -EINVAL;

	return 0;
}

static const struct spi_controller_mem_ops mtk_snfi_mem_ops = {
	.adjust_op_size = mtk_snfi_adjust_op_size,
	.supports_op = mtk_snfi_supports_op,
	.exec_op = mtk_snfi_exec_op,
};

static const struct dm_spi_ops mtk_snfi_spi_ops = {
	.mem_ops	= &mtk_snfi_mem_ops,
	.set_speed	= mtk_snfi_set_speed,
	.set_mode	= mtk_snfi_set_mode,
};

static const struct udevice_id mtk_snfi_spi_ids[] = {
	{ .compatible = "mediatek,mtk-snfi-spi" },
	{ }
};

U_BOOT_DRIVER(mtk_snfi_spi) = {
	.name			= "mtk_snfi_spi",
	.id			= UCLASS_SPI,
	.of_match		= mtk_snfi_spi_ids,
	.ops			= &mtk_snfi_spi_ops,
	.priv_auto	= sizeof(struct mtk_snfi_priv),
	.probe			= mtk_snfi_spi_probe,
};
