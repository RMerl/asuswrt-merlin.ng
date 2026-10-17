// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * LiteX Liteeth Ethernet
 *
 * Copyright 2021 Joel Stanley <joel@jms.id.au>, IBM Corp.
 */

#include <linux/litex.h>

#include <dm.h>
#include <dm/device_compat.h>
#include <net.h>

#define LITEETH_WRITER_SLOT       0x00
#define LITEETH_WRITER_LENGTH     0x04
#define LITEETH_WRITER_ERRORS     0x08
#define LITEETH_WRITER_EV_STATUS  0x0C
#define LITEETH_WRITER_EV_PENDING 0x10
#define LITEETH_WRITER_EV_ENABLE  0x14
#define LITEETH_READER_START      0x18
#define LITEETH_READER_READY      0x1C
#define LITEETH_READER_LEVEL      0x20
#define LITEETH_READER_SLOT       0x24
#define LITEETH_READER_LENGTH     0x28
#define LITEETH_READER_EV_STATUS  0x2C
#define LITEETH_READER_EV_PENDING 0x30
#define LITEETH_READER_EV_ENABLE  0x34
#define LITEETH_PREAMBLE_CRC      0x38
#define LITEETH_PREAMBLE_ERRORS   0x3C
#define LITEETH_CRC_ERRORS        0x40

struct liteeth {
	struct udevice *dev;

	void __iomem *base;
	u32 slot_size;

	/* Tx */
	u32 tx_slot;
	u32 num_tx_slots;
	void __iomem *tx_base;

	/* Rx */
	u32 rx_slot;
	u32 num_rx_slots;
	void __iomem *rx_base;
};

static int liteeth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct liteeth *priv = dev_get_priv(dev);
	u8 rx_slot;
	int len;

	if (!litex_read8(priv->base + LITEETH_WRITER_EV_PENDING)) {
		debug("liteeth: No packet ready\n");
		return -EAGAIN;
	}

	rx_slot = litex_read8(priv->base + LITEETH_WRITER_SLOT);
	len = litex_read32(priv->base + LITEETH_WRITER_LENGTH);

	debug("%s: slot %d len 0x%x\n", __func__, rx_slot, len);

	*packetp = priv->rx_base + rx_slot * priv->slot_size;

	return len;
}

static int liteeth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct liteeth *priv = dev_get_priv(dev);

	litex_write8(priv->base + LITEETH_WRITER_EV_PENDING, 1);

	return 0;
}

static int liteeth_start(struct udevice *dev)
{
	struct liteeth *priv = dev_get_priv(dev);

	/* Clear pending events */
	litex_write8(priv->base + LITEETH_WRITER_EV_PENDING, 1);
	litex_write8(priv->base + LITEETH_READER_EV_PENDING, 1);

	/* Enable events */
	litex_write8(priv->base + LITEETH_WRITER_EV_ENABLE, 1);
	litex_write8(priv->base + LITEETH_READER_EV_ENABLE, 1);

	return 0;
}

static void liteeth_stop(struct udevice *dev)
{
	struct liteeth *priv = dev_get_priv(dev);

	litex_write8(priv->base + LITEETH_WRITER_EV_ENABLE, 0);
	litex_write8(priv->base + LITEETH_READER_EV_ENABLE, 0);
}

static int liteeth_send(struct udevice *dev, void *packet, int len)
{
	struct liteeth *priv = dev_get_priv(dev);
	void __iomem *txbuffer;

	if (!litex_read8(priv->base + LITEETH_READER_READY)) {
		printf("liteeth: reader not ready\n");
		return -EAGAIN;
	}

	/* Reject oversize packets */
	if (unlikely(len > priv->slot_size))
		return -EMSGSIZE;

	txbuffer = priv->tx_base + priv->tx_slot * priv->slot_size;
	memcpy_toio(txbuffer, packet, len);
	litex_write8(priv->base + LITEETH_READER_SLOT, priv->tx_slot);
	litex_write16(priv->base + LITEETH_READER_LENGTH, len);
	litex_write8(priv->base + LITEETH_READER_START, 1);

	priv->tx_slot = (priv->tx_slot + 1) % priv->num_tx_slots;

	return 0;
}

static void liteeth_setup_slots(struct liteeth *priv)
{
	int err;

	err = ofnode_read_u32(dev_ofnode(priv->dev), "litex,rx-slots", &priv->num_rx_slots);
	if (err) {
		dev_dbg(priv->dev, "unable to get litex,rx-slots, using 2\n");
		priv->num_rx_slots = 2;
	}

	err = ofnode_read_u32(dev_ofnode(priv->dev), "litex,tx-slots", &priv->num_tx_slots);
	if (err) {
		dev_dbg(priv->dev, "unable to get litex,tx-slots, using 2\n");
		priv->num_tx_slots = 2;
	}

	err = ofnode_read_u32(dev_ofnode(priv->dev), "litex,slot-size", &priv->slot_size);
	if (err) {
		dev_dbg(priv->dev, "unable to get litex,slot-size, using 0x800\n");
		priv->slot_size = 0x800;
	}
}

static int liteeth_remove(struct udevice *dev)
{
	liteeth_stop(dev);

	return 0;
}

static const struct eth_ops liteeth_ops = {
	.start = liteeth_start,
	.stop = liteeth_stop,
	.send = liteeth_send,
	.recv = liteeth_recv,
	.free_pkt = liteeth_free_pkt,
};

static int liteeth_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct liteeth *priv = dev_get_priv(dev);
	void __iomem *buf_base;

	pdata->iobase = dev_read_addr(dev);

	priv->dev = dev;

	priv->base = dev_remap_addr_name(dev, "mac");
	if (!priv->base) {
		dev_err(dev, "failed to map registers\n");
		return -EINVAL;
	}

	buf_base = dev_remap_addr_name(dev, "buffer");
	if (!buf_base) {
		dev_err(dev, "failed to map buffer\n");
		return -EINVAL;
	}

	liteeth_setup_slots(priv);

	/* Rx slots */
	priv->rx_base = buf_base;
	priv->rx_slot = 0;

	/* Tx slots come after Rx slots */
	priv->tx_base = buf_base + priv->num_rx_slots * priv->slot_size;
	priv->tx_slot = 0;

	return 0;
}

static const struct udevice_id liteeth_ids[] = {
	{ .compatible = "litex,liteeth" },
	{}
};

U_BOOT_DRIVER(liteeth) = {
	.name = "liteeth",
	.id = UCLASS_ETH,
	.of_match = liteeth_ids,
	.of_to_plat = liteeth_of_to_plat,
	.plat_auto = sizeof(struct eth_pdata),
	.remove = liteeth_remove,
	.ops = &liteeth_ops,
	.priv_auto = sizeof(struct liteeth),
};
