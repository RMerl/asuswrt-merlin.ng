// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/drivers/dma/bcm63xx-iudma.c:
 *	Copyright (C) 2015 Simon Arlott <simon@fire.lp0.eu>
 *
 * Derived from linux/drivers/net/ethernet/broadcom/bcm63xx_enet.c:
 *	Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 *
 * Derived from bcm963xx_4.12L.06B_consumer/shared/opensource/include/bcm963xx/63268_map_part.h:
 *	Copyright (C) 2000-2010 Broadcom Corporation
 *
 * Derived from bcm963xx_4.12L.06B_consumer/bcmdrivers/opensource/net/enet/impl4/bcmenet.c:
 *	Copyright (C) 2010 Broadcom Corporation
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dma-uclass.h>
#include <memalign.h>
#include <reset.h>
#include <asm/io.h>

#define DMA_RX_DESC	6
#define DMA_TX_DESC	1

/* DMA Channels */
#define DMA_CHAN_FLOWC(x)		((x) >> 1)
#define DMA_CHAN_MAX			16
#define DMA_CHAN_SIZE			0x10
#define DMA_CHAN_TOUT			500

/* DMA Global Configuration register */
#define DMA_CFG_REG			0x00
#define  DMA_CFG_ENABLE_SHIFT		0
#define  DMA_CFG_ENABLE_MASK		(1 << DMA_CFG_ENABLE_SHIFT)
#define  DMA_CFG_FLOWC_ENABLE(x)	BIT(DMA_CHAN_FLOWC(x) + 1)
#define  DMA_CFG_NCHANS_SHIFT		24
#define  DMA_CFG_NCHANS_MASK		(0xf << DMA_CFG_NCHANS_SHIFT)

/* DMA Global Flow Control registers */
#define DMA_FLOWC_THR_LO_REG(x)		(0x04 + DMA_CHAN_FLOWC(x) * 0x0c)
#define DMA_FLOWC_THR_HI_REG(x)		(0x08 + DMA_CHAN_FLOWC(x) * 0x0c)
#define DMA_FLOWC_ALLOC_REG(x)		(0x0c + DMA_CHAN_FLOWC(x) * 0x0c)
#define  DMA_FLOWC_ALLOC_FORCE_SHIFT	31
#define  DMA_FLOWC_ALLOC_FORCE_MASK	(1 << DMA_FLOWC_ALLOC_FORCE_SHIFT)

/* DMA Global Reset register */
#define DMA_RST_REG			0x34
#define  DMA_RST_CHAN_SHIFT		0
#define  DMA_RST_CHAN_MASK(x)		(1 << x)

/* DMA Channel Configuration register */
#define DMAC_CFG_REG(x)			(DMA_CHAN_SIZE * (x) + 0x00)
#define  DMAC_CFG_ENABLE_SHIFT		0
#define  DMAC_CFG_ENABLE_MASK		(1 << DMAC_CFG_ENABLE_SHIFT)
#define  DMAC_CFG_PKT_HALT_SHIFT	1
#define  DMAC_CFG_PKT_HALT_MASK		(1 << DMAC_CFG_PKT_HALT_SHIFT)
#define  DMAC_CFG_BRST_HALT_SHIFT	2
#define  DMAC_CFG_BRST_HALT_MASK	(1 << DMAC_CFG_BRST_HALT_SHIFT)

/* DMA Channel Max Burst Length register */
#define DMAC_BURST_REG(x)		(DMA_CHAN_SIZE * (x) + 0x0c)

/* DMA SRAM Descriptor Ring Start register */
#define DMAS_RSTART_REG(x)		(DMA_CHAN_SIZE * (x) + 0x00)

/* DMA SRAM State/Bytes done/ring offset register */
#define DMAS_STATE_DATA_REG(x)		(DMA_CHAN_SIZE * (x) + 0x04)

/* DMA SRAM Buffer Descriptor status and length register */
#define DMAS_DESC_LEN_STATUS_REG(x)	(DMA_CHAN_SIZE * (x) + 0x08)

/* DMA SRAM Buffer Descriptor status and length register */
#define DMAS_DESC_BASE_BUFPTR_REG(x)	(DMA_CHAN_SIZE * (x) + 0x0c)

/* DMA Descriptor Status */
#define DMAD_ST_CRC_SHIFT		8
#define DMAD_ST_CRC_MASK		(1 << DMAD_ST_CRC_SHIFT)
#define DMAD_ST_WRAP_SHIFT		12
#define DMAD_ST_WRAP_MASK		(1 << DMAD_ST_WRAP_SHIFT)
#define DMAD_ST_SOP_SHIFT		13
#define DMAD_ST_SOP_MASK		(1 << DMAD_ST_SOP_SHIFT)
#define DMAD_ST_EOP_SHIFT		14
#define DMAD_ST_EOP_MASK		(1 << DMAD_ST_EOP_SHIFT)
#define DMAD_ST_OWN_SHIFT		15
#define DMAD_ST_OWN_MASK		(1 << DMAD_ST_OWN_SHIFT)

#define DMAD6348_ST_OV_ERR_SHIFT	0
#define DMAD6348_ST_OV_ERR_MASK		(1 << DMAD6348_ST_OV_ERR_SHIFT)
#define DMAD6348_ST_CRC_ERR_SHIFT	1
#define DMAD6348_ST_CRC_ERR_MASK	(1 << DMAD6348_ST_CRC_ERR_SHIFT)
#define DMAD6348_ST_RX_ERR_SHIFT	2
#define DMAD6348_ST_RX_ERR_MASK		(1 << DMAD6348_ST_RX_ERR_SHIFT)
#define DMAD6348_ST_OS_ERR_SHIFT	4
#define DMAD6348_ST_OS_ERR_MASK		(1 << DMAD6348_ST_OS_ERR_SHIFT)
#define DMAD6348_ST_UN_ERR_SHIFT	9
#define DMAD6348_ST_UN_ERR_MASK		(1 << DMAD6348_ST_UN_ERR_SHIFT)

struct bcm6348_dma_desc {
	uint16_t length;
	uint16_t status;
	uint32_t address;
};

struct bcm6348_chan_priv {
	void __iomem *dma_ring;
	uint8_t dma_ring_size;
	uint8_t desc_id;
	uint8_t desc_cnt;
	bool *busy_desc;
	bool running;
};

struct bcm6348_iudma_hw {
	uint16_t err_mask;
};

struct bcm6348_iudma_priv {
	const struct bcm6348_iudma_hw *hw;
	void __iomem *base;
	void __iomem *chan;
	void __iomem *sram;
	struct bcm6348_chan_priv **ch_priv;
	uint8_t n_channels;
};

static inline bool bcm6348_iudma_chan_is_rx(uint8_t ch)
{
	return !(ch & 1);
}

static inline void bcm6348_iudma_fdc(void *ptr, ulong size)
{
	ulong start = (ulong) ptr;

	flush_dcache_range(start, start + size);
}

static inline void bcm6348_iudma_idc(void *ptr, ulong size)
{
	ulong start = (ulong) ptr;

	invalidate_dcache_range(start, start + size);
}

static void bcm6348_iudma_chan_stop(struct bcm6348_iudma_priv *priv,
				    uint8_t ch)
{
	unsigned int timeout = DMA_CHAN_TOUT;

	do {
		uint32_t cfg, halt;

		if (timeout > DMA_CHAN_TOUT / 2)
			halt = DMAC_CFG_PKT_HALT_MASK;
		else
			halt = DMAC_CFG_BRST_HALT_MASK;

		/* try to stop dma channel */
		writel_be(halt, priv->chan + DMAC_CFG_REG(ch));
		mb();

		/* check if channel was stopped */
		cfg = readl_be(priv->chan + DMAC_CFG_REG(ch));
		if (!(cfg & DMAC_CFG_ENABLE_MASK))
			break;

		udelay(1);
	} while (--timeout);

	if (!timeout)
		pr_err("unable to stop channel %u\n", ch);

	/* reset dma channel */
	setbits_be32(priv->base + DMA_RST_REG, DMA_RST_CHAN_MASK(ch));
	mb();
	clrbits_be32(priv->base + DMA_RST_REG, DMA_RST_CHAN_MASK(ch));
}

static int bcm6348_iudma_disable(struct dma *dma)
{
	struct bcm6348_iudma_priv *priv = dev_get_priv(dma->dev);
	struct bcm6348_chan_priv *ch_priv = priv->ch_priv[dma->id];

	/* stop dma channel */
	bcm6348_iudma_chan_stop(priv, dma->id);

	/* dma flow control */
	if (bcm6348_iudma_chan_is_rx(dma->id))
		writel_be(DMA_FLOWC_ALLOC_FORCE_MASK,
			  DMA_FLOWC_ALLOC_REG(dma->id));

	/* init channel config */
	ch_priv->running = false;
	ch_priv->desc_id = 0;
	if (bcm6348_iudma_chan_is_rx(dma->id))
		ch_priv->desc_cnt = 0;
	else
		ch_priv->desc_cnt = ch_priv->dma_ring_size;

	return 0;
}

static int bcm6348_iudma_enable(struct dma *dma)
{
	const struct bcm6348_iudma_priv *priv = dev_get_priv(dma->dev);
	struct bcm6348_chan_priv *ch_priv = priv->ch_priv[dma->id];
	struct bcm6348_dma_desc *dma_desc = ch_priv->dma_ring;
	uint8_t i;

	/* dma ring init */
	for (i = 0; i < ch_priv->desc_cnt; i++) {
		if (bcm6348_iudma_chan_is_rx(dma->id)) {
			ch_priv->busy_desc[i] = false;
			dma_desc->status |= DMAD_ST_OWN_MASK;
		} else {
			dma_desc->status = 0;
			dma_desc->length = 0;
			dma_desc->address = 0;
		}

		if (i == ch_priv->desc_cnt - 1)
			dma_desc->status |= DMAD_ST_WRAP_MASK;

		dma_desc++;
	}

	/* init to first descriptor */
	ch_priv->desc_id = 0;

	/* force cache writeback */
	bcm6348_iudma_fdc(ch_priv->dma_ring,
			  sizeof(*dma_desc) * ch_priv->desc_cnt);

	/* clear sram */
	writel_be(0, priv->sram + DMAS_STATE_DATA_REG(dma->id));
	writel_be(0, priv->sram + DMAS_DESC_LEN_STATUS_REG(dma->id));
	writel_be(0, priv->sram + DMAS_DESC_BASE_BUFPTR_REG(dma->id));

	/* set dma ring start */
	writel_be(virt_to_phys(ch_priv->dma_ring),
		  priv->sram + DMAS_RSTART_REG(dma->id));

	/* set flow control */
	if (bcm6348_iudma_chan_is_rx(dma->id)) {
		u32 val;

		setbits_be32(priv->base + DMA_CFG_REG,
			     DMA_CFG_FLOWC_ENABLE(dma->id));

		val = ch_priv->desc_cnt / 3;
		writel_be(val, priv->base + DMA_FLOWC_THR_LO_REG(dma->id));

		val = (ch_priv->desc_cnt * 2) / 3;
		writel_be(val, priv->base + DMA_FLOWC_THR_HI_REG(dma->id));

		writel_be(0, priv->base + DMA_FLOWC_ALLOC_REG(dma->id));
	}

	/* set dma max burst */
	writel_be(ch_priv->desc_cnt,
		  priv->chan + DMAC_BURST_REG(dma->id));

	/* kick rx dma channel */
	if (bcm6348_iudma_chan_is_rx(dma->id))
		setbits_be32(priv->chan + DMAC_CFG_REG(dma->id),
			     DMAC_CFG_ENABLE_MASK);

	/* channel is now enabled */
	ch_priv->running = true;

	return 0;
}

static int bcm6348_iudma_request(struct dma *dma)
{
	const struct bcm6348_iudma_priv *priv = dev_get_priv(dma->dev);
	struct bcm6348_chan_priv *ch_priv;

	/* check if channel is valid */
	if (dma->id >= priv->n_channels)
		return -ENODEV;

	/* alloc channel private data */
	priv->ch_priv[dma->id] = calloc(1, sizeof(struct bcm6348_chan_priv));
	if (!priv->ch_priv[dma->id])
		return -ENOMEM;
	ch_priv = priv->ch_priv[dma->id];

	/* alloc dma ring */
	if (bcm6348_iudma_chan_is_rx(dma->id))
		ch_priv->dma_ring_size = DMA_RX_DESC;
	else
		ch_priv->dma_ring_size = DMA_TX_DESC;

	ch_priv->dma_ring =
		malloc_cache_aligned(sizeof(struct bcm6348_dma_desc) *
				     ch_priv->dma_ring_size);
	if (!ch_priv->dma_ring)
		return -ENOMEM;

	/* init channel config */
	ch_priv->running = false;
	ch_priv->desc_id = 0;
	if (bcm6348_iudma_chan_is_rx(dma->id)) {
		ch_priv->desc_cnt = 0;
		ch_priv->busy_desc = calloc(ch_priv->desc_cnt, sizeof(bool));
	} else {
		ch_priv->desc_cnt = ch_priv->dma_ring_size;
		ch_priv->busy_desc = NULL;
	}

	return 0;
}

static int bcm6348_iudma_receive(struct dma *dma, void **dst, void *metadata)
{
	const struct bcm6348_iudma_priv *priv = dev_get_priv(dma->dev);
	const struct bcm6348_iudma_hw *hw = priv->hw;
	struct bcm6348_chan_priv *ch_priv = priv->ch_priv[dma->id];
	struct bcm6348_dma_desc *dma_desc = dma_desc = ch_priv->dma_ring;
	int ret;

	if (!ch_priv->running)
		return -EINVAL;

	/* get dma ring descriptor address */
	dma_desc += ch_priv->desc_id;

	/* invalidate cache data */
	bcm6348_iudma_idc(dma_desc, sizeof(*dma_desc));

	/* check dma own */
	if (dma_desc->status & DMAD_ST_OWN_MASK)
		return -EAGAIN;

	/* check pkt */
	if (!(dma_desc->status & DMAD_ST_EOP_MASK) ||
	    !(dma_desc->status & DMAD_ST_SOP_MASK) ||
	    (dma_desc->status & hw->err_mask)) {
		pr_err("invalid pkt received (ch=%ld desc=%u) (st=%04x)\n",
		       dma->id, ch_priv->desc_id, dma_desc->status);
		ret = -EAGAIN;
	} else {
		/* set dma buffer address */
		*dst = phys_to_virt(dma_desc->address);

		/* invalidate cache data */
		bcm6348_iudma_idc(*dst, dma_desc->length);

		/* return packet length */
		ret = dma_desc->length;
	}

	/* busy dma descriptor */
	ch_priv->busy_desc[ch_priv->desc_id] = true;

	/* increment dma descriptor */
	ch_priv->desc_id = (ch_priv->desc_id + 1) % ch_priv->desc_cnt;

	return ret;
}

static int bcm6348_iudma_send(struct dma *dma, void *src, size_t len,
			      void *metadata)
{
	const struct bcm6348_iudma_priv *priv = dev_get_priv(dma->dev);
	struct bcm6348_chan_priv *ch_priv = priv->ch_priv[dma->id];
	struct bcm6348_dma_desc *dma_desc;
	uint16_t status;

	if (!ch_priv->running)
                return -EINVAL;

	/* flush cache */
	bcm6348_iudma_fdc(src, len);

	/* get dma ring descriptor address */
	dma_desc = ch_priv->dma_ring;
	dma_desc += ch_priv->desc_id;

	/* config dma descriptor */
	status = (DMAD_ST_OWN_MASK |
		  DMAD_ST_EOP_MASK |
		  DMAD_ST_CRC_MASK |
		  DMAD_ST_SOP_MASK);
	if (ch_priv->desc_id == ch_priv->desc_cnt - 1)
		status |= DMAD_ST_WRAP_MASK;

	/* set dma descriptor */
	dma_desc->address = virt_to_phys(src);
	dma_desc->length = len;
	dma_desc->status = status;

	/* flush cache */
	bcm6348_iudma_fdc(dma_desc, sizeof(*dma_desc));

	/* kick tx dma channel */
	setbits_be32(priv->chan + DMAC_CFG_REG(dma->id), DMAC_CFG_ENABLE_MASK);

	/* poll dma status */
	do {
		/* invalidate cache */
		bcm6348_iudma_idc(dma_desc, sizeof(*dma_desc));

		if (!(dma_desc->status & DMAD_ST_OWN_MASK))
			break;
	} while(1);

	/* increment dma descriptor */
	ch_priv->desc_id = (ch_priv->desc_id + 1) % ch_priv->desc_cnt;

	return 0;
}

static int bcm6348_iudma_free_rcv_buf(struct dma *dma, void *dst, size_t size)
{
	const struct bcm6348_iudma_priv *priv = dev_get_priv(dma->dev);
	struct bcm6348_chan_priv *ch_priv = priv->ch_priv[dma->id];
	struct bcm6348_dma_desc *dma_desc = ch_priv->dma_ring;
	uint16_t status;
	uint8_t i;
	u32 cfg;

	/* get dirty dma descriptor */
	for (i = 0; i < ch_priv->desc_cnt; i++) {
		if (phys_to_virt(dma_desc->address) == dst)
			break;

		dma_desc++;
	}

	/* dma descriptor not found */
	if (i == ch_priv->desc_cnt) {
		pr_err("dirty dma descriptor not found\n");
		return -ENOENT;
	}

	/* invalidate cache */
	bcm6348_iudma_idc(ch_priv->dma_ring,
			  sizeof(*dma_desc) * ch_priv->desc_cnt);

	/* free dma descriptor */
	ch_priv->busy_desc[i] = false;

	status = DMAD_ST_OWN_MASK;
	if (i == ch_priv->desc_cnt - 1)
		status |= DMAD_ST_WRAP_MASK;

	dma_desc->status |= status;
	dma_desc->length = PKTSIZE_ALIGN;

	/* tell dma we allocated one buffer */
	writel_be(1, DMA_FLOWC_ALLOC_REG(dma->id));

	/* flush cache */
	bcm6348_iudma_fdc(ch_priv->dma_ring,
			  sizeof(*dma_desc) * ch_priv->desc_cnt);

	/* kick rx dma channel if disabled */
	cfg = readl_be(priv->chan + DMAC_CFG_REG(dma->id));
	if (!(cfg & DMAC_CFG_ENABLE_MASK))
		setbits_be32(priv->chan + DMAC_CFG_REG(dma->id),
			     DMAC_CFG_ENABLE_MASK);

	return 0;
}

static int bcm6348_iudma_add_rcv_buf(struct dma *dma, void *dst, size_t size)
{
	const struct bcm6348_iudma_priv *priv = dev_get_priv(dma->dev);
	struct bcm6348_chan_priv *ch_priv = priv->ch_priv[dma->id];
	struct bcm6348_dma_desc *dma_desc = ch_priv->dma_ring;

	/* no more dma descriptors available */
	if (ch_priv->desc_cnt == ch_priv->dma_ring_size) {
		pr_err("max number of buffers reached\n");
		return -EINVAL;
	}

	/* get next dma descriptor */
	dma_desc += ch_priv->desc_cnt;

	/* init dma descriptor */
	dma_desc->address = virt_to_phys(dst);
	dma_desc->length = size;
	dma_desc->status = 0;

	/* flush cache */
	bcm6348_iudma_fdc(dma_desc, sizeof(*dma_desc));

	/* increment dma descriptors */
	ch_priv->desc_cnt++;

	return 0;
}

static int bcm6348_iudma_prepare_rcv_buf(struct dma *dma, void *dst,
					 size_t size)
{
	const struct bcm6348_iudma_priv *priv = dev_get_priv(dma->dev);
	struct bcm6348_chan_priv *ch_priv = priv->ch_priv[dma->id];

	/* only add new rx buffers if channel isn't running */
	if (ch_priv->running)
		return bcm6348_iudma_free_rcv_buf(dma, dst, size);
	else
		return bcm6348_iudma_add_rcv_buf(dma, dst, size);
}

static const struct dma_ops bcm6348_iudma_ops = {
	.disable = bcm6348_iudma_disable,
	.enable = bcm6348_iudma_enable,
	.prepare_rcv_buf = bcm6348_iudma_prepare_rcv_buf,
	.request = bcm6348_iudma_request,
	.receive = bcm6348_iudma_receive,
	.send = bcm6348_iudma_send,
};

static const struct bcm6348_iudma_hw bcm6348_hw = {
	.err_mask = (DMAD6348_ST_OV_ERR_MASK |
		     DMAD6348_ST_CRC_ERR_MASK |
		     DMAD6348_ST_RX_ERR_MASK |
		     DMAD6348_ST_OS_ERR_MASK |
		     DMAD6348_ST_UN_ERR_MASK),
};

static const struct bcm6348_iudma_hw bcm6368_hw = {
	.err_mask = 0,
};

static const struct udevice_id bcm6348_iudma_ids[] = {
	{
		.compatible = "brcm,bcm6348-iudma",
		.data = (ulong)&bcm6348_hw,
	}, {
		.compatible = "brcm,bcm6368-iudma",
		.data = (ulong)&bcm6368_hw,
	}, { /* sentinel */ }
};

static int bcm6348_iudma_probe(struct udevice *dev)
{
	struct dma_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct bcm6348_iudma_priv *priv = dev_get_priv(dev);
	const struct bcm6348_iudma_hw *hw =
		(const struct bcm6348_iudma_hw *)dev_get_driver_data(dev);
	uint8_t ch;
	int i;

	uc_priv->supported = (DMA_SUPPORTS_DEV_TO_MEM |
			      DMA_SUPPORTS_MEM_TO_DEV);
	priv->hw = hw;

	/* dma global base address */
	priv->base = dev_remap_addr_name(dev, "dma");
	if (!priv->base)
		return -EINVAL;

	/* dma channels base address */
	priv->chan = dev_remap_addr_name(dev, "dma-channels");
	if (!priv->chan)
		return -EINVAL;

	/* dma sram base address */
	priv->sram = dev_remap_addr_name(dev, "dma-sram");
	if (!priv->sram)
		return -EINVAL;

	/* get number of channels */
	priv->n_channels = dev_read_u32_default(dev, "dma-channels", 8);
	if (priv->n_channels > DMA_CHAN_MAX)
		return -EINVAL;

	/* try to enable clocks */
	for (i = 0; ; i++) {
		struct clk clk;
		int ret;

		ret = clk_get_by_index(dev, i, &clk);
		if (ret < 0)
			break;

		ret = clk_enable(&clk);
		if (ret < 0) {
			pr_err("error enabling clock %d\n", i);
			return ret;
		}

		ret = clk_free(&clk);
		if (ret < 0) {
			pr_err("error freeing clock %d\n", i);
			return ret;
		}
	}

	/* try to perform resets */
	for (i = 0; ; i++) {
		struct reset_ctl reset;
		int ret;

		ret = reset_get_by_index(dev, i, &reset);
		if (ret < 0)
			break;

		ret = reset_deassert(&reset);
		if (ret < 0) {
			pr_err("error deasserting reset %d\n", i);
			return ret;
		}

		ret = reset_free(&reset);
		if (ret < 0) {
			pr_err("error freeing reset %d\n", i);
			return ret;
		}
	}

	/* disable dma controller */
	clrbits_be32(priv->base + DMA_CFG_REG, DMA_CFG_ENABLE_MASK);

	/* alloc channel private data pointers */
	priv->ch_priv = calloc(priv->n_channels,
			       sizeof(struct bcm6348_chan_priv*));
	if (!priv->ch_priv)
		return -ENOMEM;

	/* stop dma channels */
	for (ch = 0; ch < priv->n_channels; ch++)
		bcm6348_iudma_chan_stop(priv, ch);

	/* enable dma controller */
	setbits_be32(priv->base + DMA_CFG_REG, DMA_CFG_ENABLE_MASK);

	return 0;
}

U_BOOT_DRIVER(bcm6348_iudma) = {
	.name = "bcm6348_iudma",
	.id = UCLASS_DMA,
	.of_match = bcm6348_iudma_ids,
	.ops = &bcm6348_iudma_ops,
	.priv_auto_alloc_size = sizeof(struct bcm6348_iudma_priv),
	.probe = bcm6348_iudma_probe,
};
