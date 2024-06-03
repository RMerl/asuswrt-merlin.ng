// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Tuomas Tynkkynen <tuomas.tynkkynen@iki.fi>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <net.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include "virtio_net.h"

/* Amount of buffers to keep in the RX virtqueue */
#define VIRTIO_NET_NUM_RX_BUFS	32

/*
 * This value comes from the VirtIO spec: 1500 for maximum packet size,
 * 14 for the Ethernet header, 12 for virtio_net_hdr. In total 1526 bytes.
 */
#define VIRTIO_NET_RX_BUF_SIZE	1526

struct virtio_net_priv {
	union {
		struct virtqueue *vqs[2];
		struct {
			struct virtqueue *rx_vq;
			struct virtqueue *tx_vq;
		};
	};

	char rx_buff[VIRTIO_NET_NUM_RX_BUFS][VIRTIO_NET_RX_BUF_SIZE];
	bool rx_running;
	int net_hdr_len;
};

/*
 * For simplicity, the driver only negotiates the VIRTIO_NET_F_MAC feature.
 * For the VIRTIO_NET_F_STATUS feature, we don't negotiate it, hence per spec
 * we should assume the link is always active.
 */
static const u32 feature[] = {
	VIRTIO_NET_F_MAC
};

static const u32 feature_legacy[] = {
	VIRTIO_NET_F_MAC
};

static int virtio_net_start(struct udevice *dev)
{
	struct virtio_net_priv *priv = dev_get_priv(dev);
	struct virtio_sg sg;
	struct virtio_sg *sgs[] = { &sg };
	int i;

	if (!priv->rx_running) {
		/* receive buffer length is always 1526 */
		sg.length = VIRTIO_NET_RX_BUF_SIZE;

		/* setup the receive buffer address */
		for (i = 0; i < VIRTIO_NET_NUM_RX_BUFS; i++) {
			sg.addr = priv->rx_buff[i];
			virtqueue_add(priv->rx_vq, sgs, 0, 1);
		}

		virtqueue_kick(priv->rx_vq);

		/* setup the receive queue only once */
		priv->rx_running = true;
	}

	return 0;
}

static int virtio_net_send(struct udevice *dev, void *packet, int length)
{
	struct virtio_net_priv *priv = dev_get_priv(dev);
	struct virtio_net_hdr hdr;
	struct virtio_net_hdr_v1 hdr_v1;
	struct virtio_sg hdr_sg;
	struct virtio_sg data_sg = { packet, length };
	struct virtio_sg *sgs[] = { &hdr_sg, &data_sg };
	int ret;

	if (priv->net_hdr_len == sizeof(struct virtio_net_hdr))
		hdr_sg.addr = &hdr;
	else
		hdr_sg.addr = &hdr_v1;
	hdr_sg.length = priv->net_hdr_len;

	memset(hdr_sg.addr, 0, priv->net_hdr_len);

	ret = virtqueue_add(priv->tx_vq, sgs, 2, 0);
	if (ret)
		return ret;

	virtqueue_kick(priv->tx_vq);

	while (1) {
		if (virtqueue_get_buf(priv->tx_vq, NULL))
			break;
	}

	return 0;
}

static int virtio_net_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct virtio_net_priv *priv = dev_get_priv(dev);
	unsigned int len;
	void *buf;

	buf = virtqueue_get_buf(priv->rx_vq, &len);
	if (!buf)
		return -EAGAIN;

	*packetp = buf + priv->net_hdr_len;
	return len - priv->net_hdr_len;
}

static int virtio_net_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct virtio_net_priv *priv = dev_get_priv(dev);
	void *buf = packet - priv->net_hdr_len;
	struct virtio_sg sg = { buf, VIRTIO_NET_RX_BUF_SIZE };
	struct virtio_sg *sgs[] = { &sg };

	/* Put the buffer back to the rx ring */
	virtqueue_add(priv->rx_vq, sgs, 0, 1);

	return 0;
}

static void virtio_net_stop(struct udevice *dev)
{
	/*
	 * There is no way to stop the queue from running, unless we issue
	 * a reset to the virtio device, and re-do the queue initialization
	 * from the beginning.
	 */
}

static int virtio_net_write_hwaddr(struct udevice *dev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(dev->parent);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	int i;

	/*
	 * v1.0 compliant device's MAC address is set through control channel,
	 * which we don't support for now.
	 */
	if (!uc_priv->legacy)
		return -ENOSYS;

	for (i = 0; i < sizeof(pdata->enetaddr); i++) {
		virtio_cwrite8(dev,
			       offsetof(struct virtio_net_config, mac) + i,
			       pdata->enetaddr[i]);
	}

	return 0;
}

static int virtio_net_read_rom_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	if (!pdata)
		return -ENOSYS;

	if (virtio_has_feature(dev, VIRTIO_NET_F_MAC)) {
		virtio_cread_bytes(dev,
				   offsetof(struct virtio_net_config, mac),
				   pdata->enetaddr, sizeof(pdata->enetaddr));
	}

	return 0;
}

static int virtio_net_bind(struct udevice *dev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(dev->parent);

	/* Indicate what driver features we support */
	virtio_driver_features_init(uc_priv, feature, ARRAY_SIZE(feature),
				    feature_legacy, ARRAY_SIZE(feature_legacy));

	return 0;
}

static int virtio_net_probe(struct udevice *dev)
{
	struct virtio_net_priv *priv = dev_get_priv(dev);
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(dev->parent);
	int ret;

	ret = virtio_find_vqs(dev, 2, priv->vqs);
	if (ret < 0)
		return ret;

	/*
	 * For v1.0 compliant device, it always assumes the member
	 * 'num_buffers' exists in the struct virtio_net_hdr while
	 * the legacy driver only presented 'num_buffers' when
	 * VIRTIO_NET_F_MRG_RXBUF was negotiated. Without that feature
	 * the structure was 2 bytes shorter.
	 */
	if (uc_priv->legacy)
		priv->net_hdr_len = sizeof(struct virtio_net_hdr);
	else
		priv->net_hdr_len = sizeof(struct virtio_net_hdr_v1);

	return 0;
}

static const struct eth_ops virtio_net_ops = {
	.start = virtio_net_start,
	.send = virtio_net_send,
	.recv = virtio_net_recv,
	.free_pkt = virtio_net_free_pkt,
	.stop = virtio_net_stop,
	.write_hwaddr = virtio_net_write_hwaddr,
	.read_rom_hwaddr = virtio_net_read_rom_hwaddr,
};

U_BOOT_DRIVER(virtio_net) = {
	.name	= VIRTIO_NET_DRV_NAME,
	.id	= UCLASS_ETH,
	.bind	= virtio_net_bind,
	.probe	= virtio_net_probe,
	.remove = virtio_reset,
	.ops	= &virtio_net_ops,
	.priv_auto_alloc_size = sizeof(struct virtio_net_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags	= DM_FLAG_ACTIVE_DMA,
};
