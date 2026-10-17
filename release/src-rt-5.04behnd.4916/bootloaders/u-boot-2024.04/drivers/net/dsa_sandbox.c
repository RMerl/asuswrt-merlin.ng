// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019-2021 NXP
 */

#include <asm/eth.h>
#include <net/dsa.h>
#include <net.h>

#define DSA_SANDBOX_MAGIC	0x00415344
#define DSA_SANDBOX_TAG_LEN	sizeof(struct dsa_sandbox_tag)

struct dsa_sandbox_priv {
	struct eth_sandbox_priv *master_priv;
	int port_en_mask;
};

struct dsa_sandbox_tag {
	u32 magic;
	u32 port;
};

static bool sb_dsa_port_enabled(struct udevice *dev, int port)
{
	struct dsa_sandbox_priv *priv = dev_get_priv(dev);

	return priv->port_en_mask & BIT(port);
}

static bool sb_dsa_master_enabled(struct udevice *dev)
{
	struct dsa_sandbox_priv *priv = dev_get_priv(dev);

	return !priv->master_priv->disabled;
}

static int dsa_sandbox_port_enable(struct udevice *dev, int port,
				   struct phy_device *phy)
{
	struct dsa_sandbox_priv *priv = dev_get_priv(dev);

	if (!sb_dsa_master_enabled(dev))
		return -EFAULT;

	priv->port_en_mask |= BIT(port);

	return 0;
}

static void dsa_sandbox_port_disable(struct udevice *dev, int port,
				     struct phy_device *phy)
{
	struct dsa_sandbox_priv *priv = dev_get_priv(dev);

	priv->port_en_mask &= ~BIT(port);
}

static int dsa_sandbox_xmit(struct udevice *dev, int port, void *packet,
			    int length)
{
	struct dsa_sandbox_tag *tag = packet;

	if (!sb_dsa_master_enabled(dev))
		return -EFAULT;

	if (!sb_dsa_port_enabled(dev, port))
		return -EFAULT;

	tag->magic = DSA_SANDBOX_MAGIC;
	tag->port = port;

	return 0;
}

static int dsa_sandbox_rcv(struct udevice *dev, int *port, void *packet,
			   int length)
{
	struct dsa_sandbox_tag *tag = packet;

	if (!sb_dsa_master_enabled(dev))
		return -EFAULT;

	if (tag->magic != DSA_SANDBOX_MAGIC)
		return -EFAULT;

	*port = tag->port;
	if (!sb_dsa_port_enabled(dev, tag->port))
		return -EFAULT;

	return 0;
}

static const struct dsa_ops dsa_sandbox_ops = {
	.port_enable = dsa_sandbox_port_enable,
	.port_disable = dsa_sandbox_port_disable,
	.xmit = dsa_sandbox_xmit,
	.rcv = dsa_sandbox_rcv,
};

static int sb_dsa_handler(struct udevice *dev, void *packet,
			  unsigned int len)
{
	struct eth_sandbox_priv *master_priv;
	struct dsa_sandbox_tag *tag = packet;
	struct udevice *dsa_dev;
	u32 port_index;
	void *rx_buf;
	int i;

	/* this emulates the switch hw and the network side */
	if (tag->magic != DSA_SANDBOX_MAGIC)
		return -EFAULT;

	port_index = tag->port;
	master_priv = dev_get_priv(dev);
	dsa_dev = master_priv->priv;
	if (!sb_dsa_port_enabled(dsa_dev, port_index))
		return -EFAULT;

	packet += DSA_SANDBOX_TAG_LEN;
	len -= DSA_SANDBOX_TAG_LEN;

	if (!sandbox_eth_arp_req_to_reply(dev, packet, len))
		goto dsa_tagging;
	if (!sandbox_eth_ping_req_to_reply(dev, packet, len))
		goto dsa_tagging;

	return 0;

dsa_tagging:
	master_priv->recv_packets--;
	i = master_priv->recv_packets;
	rx_buf = master_priv->recv_packet_buffer[i];
	len = master_priv->recv_packet_length[i];
	memmove(rx_buf + DSA_SANDBOX_TAG_LEN, rx_buf, len);

	tag = rx_buf;
	tag->magic = DSA_SANDBOX_MAGIC;
	tag->port = port_index;
	len += DSA_SANDBOX_TAG_LEN;
	master_priv->recv_packet_length[i] = len;
	master_priv->recv_packets++;

	return 0;
}

static int dsa_sandbox_probe(struct udevice *dev)
{
	struct dsa_sandbox_priv *priv = dev_get_priv(dev);
	struct udevice *master = dsa_get_master(dev);
	struct eth_sandbox_priv *master_priv;

	if (!master)
		return -ENODEV;

	dsa_set_tagging(dev, DSA_SANDBOX_TAG_LEN, 0);

	master_priv = dev_get_priv(master);
	master_priv->priv = dev;
	master_priv->tx_handler = sb_dsa_handler;

	priv->master_priv = master_priv;

	return 0;
}

static const struct udevice_id dsa_sandbox_ids[] = {
	{ .compatible = "sandbox,dsa" },
	{ }
};

U_BOOT_DRIVER(dsa_sandbox) = {
	.name		= "dsa_sandbox",
	.id		= UCLASS_DSA,
	.of_match	= dsa_sandbox_ids,
	.probe		= dsa_sandbox_probe,
	.ops		= &dsa_sandbox_ops,
	.priv_auto	= sizeof(struct dsa_sandbox_priv),
};
