// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * VirtIO PCI bus transport driver
 * Ported from Linux drivers/virtio/virtio_pci*.c
 */

#include <common.h>
#include <dm.h>
#include <virtio_types.h>
#include <virtio.h>
#include <virtio_ring.h>
#include <dm/device.h>
#include <linux/compat.h>
#include <linux/io.h>
#include "virtio_pci.h"

#define VIRTIO_PCI_DRV_NAME	"virtio-pci.l"

/* PCI device ID in the range 0x1000 to 0x103f */
#define VIRTIO_PCI_VENDOR_ID	0x1af4
#define VIRTIO_PCI_DEVICE_ID00	0x1000
#define VIRTIO_PCI_DEVICE_ID01	0x1001
#define VIRTIO_PCI_DEVICE_ID02	0x1002
#define VIRTIO_PCI_DEVICE_ID03	0x1003
#define VIRTIO_PCI_DEVICE_ID04	0x1004
#define VIRTIO_PCI_DEVICE_ID05	0x1005
#define VIRTIO_PCI_DEVICE_ID06	0x1006
#define VIRTIO_PCI_DEVICE_ID07	0x1007
#define VIRTIO_PCI_DEVICE_ID08	0x1008
#define VIRTIO_PCI_DEVICE_ID09	0x1009
#define VIRTIO_PCI_DEVICE_ID0A	0x100a
#define VIRTIO_PCI_DEVICE_ID0B	0x100b
#define VIRTIO_PCI_DEVICE_ID0C	0x100c
#define VIRTIO_PCI_DEVICE_ID0D	0x100d
#define VIRTIO_PCI_DEVICE_ID0E	0x100e
#define VIRTIO_PCI_DEVICE_ID0F	0x100f
#define VIRTIO_PCI_DEVICE_ID10	0x1010
#define VIRTIO_PCI_DEVICE_ID11	0x1011
#define VIRTIO_PCI_DEVICE_ID12	0x1012
#define VIRTIO_PCI_DEVICE_ID13	0x1013
#define VIRTIO_PCI_DEVICE_ID14	0x1014
#define VIRTIO_PCI_DEVICE_ID15	0x1015
#define VIRTIO_PCI_DEVICE_ID16	0x1016
#define VIRTIO_PCI_DEVICE_ID17	0x1017
#define VIRTIO_PCI_DEVICE_ID18	0x1018
#define VIRTIO_PCI_DEVICE_ID19	0x1019
#define VIRTIO_PCI_DEVICE_ID1A	0x101a
#define VIRTIO_PCI_DEVICE_ID1B	0x101b
#define VIRTIO_PCI_DEVICE_ID1C	0x101c
#define VIRTIO_PCI_DEVICE_ID1D	0x101d
#define VIRTIO_PCI_DEVICE_ID1E	0x101e
#define VIRTIO_PCI_DEVICE_ID1F	0x101f
#define VIRTIO_PCI_DEVICE_ID20	0x1020
#define VIRTIO_PCI_DEVICE_ID21	0x1021
#define VIRTIO_PCI_DEVICE_ID22	0x1022
#define VIRTIO_PCI_DEVICE_ID23	0x1023
#define VIRTIO_PCI_DEVICE_ID24	0x1024
#define VIRTIO_PCI_DEVICE_ID25	0x1025
#define VIRTIO_PCI_DEVICE_ID26	0x1026
#define VIRTIO_PCI_DEVICE_ID27	0x1027
#define VIRTIO_PCI_DEVICE_ID28	0x1028
#define VIRTIO_PCI_DEVICE_ID29	0x1029
#define VIRTIO_PCI_DEVICE_ID2A	0x102a
#define VIRTIO_PCI_DEVICE_ID2B	0x102b
#define VIRTIO_PCI_DEVICE_ID2C	0x102c
#define VIRTIO_PCI_DEVICE_ID2D	0x102d
#define VIRTIO_PCI_DEVICE_ID2E	0x102e
#define VIRTIO_PCI_DEVICE_ID2F	0x102f
#define VIRTIO_PCI_DEVICE_ID30	0x1030
#define VIRTIO_PCI_DEVICE_ID31	0x1031
#define VIRTIO_PCI_DEVICE_ID32	0x1032
#define VIRTIO_PCI_DEVICE_ID33	0x1033
#define VIRTIO_PCI_DEVICE_ID34	0x1034
#define VIRTIO_PCI_DEVICE_ID35	0x1035
#define VIRTIO_PCI_DEVICE_ID36	0x1036
#define VIRTIO_PCI_DEVICE_ID37	0x1037
#define VIRTIO_PCI_DEVICE_ID38	0x1038
#define VIRTIO_PCI_DEVICE_ID39	0x1039
#define VIRTIO_PCI_DEVICE_ID3A	0x103a
#define VIRTIO_PCI_DEVICE_ID3B	0x103b
#define VIRTIO_PCI_DEVICE_ID3C	0x103c
#define VIRTIO_PCI_DEVICE_ID3D	0x103d
#define VIRTIO_PCI_DEVICE_ID3E	0x103e
#define VIRTIO_PCI_DEVICE_ID3F	0x103f

/**
 * virtio pci transport driver private data
 *
 * @ioaddr:	pci transport device register base
 * @version:	pci transport device version
 */
struct virtio_pci_priv {
	void __iomem *ioaddr;
};

static int virtio_pci_get_config(struct udevice *udev, unsigned int offset,
				 void *buf, unsigned int len)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);
	void __iomem *ioaddr = priv->ioaddr + VIRTIO_PCI_CONFIG_OFF(false);
	u8 *ptr = buf;
	int i;

	for (i = 0; i < len; i++)
		ptr[i] = ioread8(ioaddr + i);

	return 0;
}

static int virtio_pci_set_config(struct udevice *udev, unsigned int offset,
				 const void *buf, unsigned int len)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);
	void __iomem *ioaddr = priv->ioaddr + VIRTIO_PCI_CONFIG_OFF(false);
	const u8 *ptr = buf;
	int i;

	for (i = 0; i < len; i++)
		iowrite8(ptr[i], ioaddr + i);

	return 0;
}

static int virtio_pci_get_status(struct udevice *udev, u8 *status)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);

	*status = ioread8(priv->ioaddr + VIRTIO_PCI_STATUS);

	return 0;
}

static int virtio_pci_set_status(struct udevice *udev, u8 status)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);

	/* We should never be setting status to 0 */
	WARN_ON(status == 0);

	iowrite8(status, priv->ioaddr + VIRTIO_PCI_STATUS);

	return 0;
}

static int virtio_pci_reset(struct udevice *udev)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);

	/* 0 status means a reset */
	iowrite8(0, priv->ioaddr + VIRTIO_PCI_STATUS);

	/*
	 * Flush out the status write, and flush in device writes,
	 * including MSI-X interrupts, if any.
	 */
	ioread8(priv->ioaddr + VIRTIO_PCI_STATUS);

	return 0;
}

static int virtio_pci_get_features(struct udevice *udev, u64 *features)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);

	/*
	 * When someone needs more than 32 feature bits, we'll need to
	 * steal a bit to indicate that the rest are somewhere else.
	 */
	*features = ioread32(priv->ioaddr + VIRTIO_PCI_HOST_FEATURES);

	return 0;
}

static int virtio_pci_set_features(struct udevice *udev)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);

	/* Make sure we don't have any features > 32 bits! */
	WARN_ON((u32)uc_priv->features != uc_priv->features);

	/* We only support 32 feature bits */
	iowrite32(uc_priv->features, priv->ioaddr + VIRTIO_PCI_GUEST_FEATURES);

	return 0;
}

static struct virtqueue *virtio_pci_setup_vq(struct udevice *udev,
					     unsigned int index)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);
	struct virtqueue *vq;
	unsigned int num;
	int err;

	/* Select the queue we're interested in */
	iowrite16(index, priv->ioaddr + VIRTIO_PCI_QUEUE_SEL);

	/* Check if queue is either not available or already active */
	num = ioread16(priv->ioaddr + VIRTIO_PCI_QUEUE_NUM);
	if (!num || ioread32(priv->ioaddr + VIRTIO_PCI_QUEUE_PFN)) {
		err = -ENOENT;
		goto error_available;
	}

	/* Create the vring */
	vq = vring_create_virtqueue(index, num, VIRTIO_PCI_VRING_ALIGN, udev);
	if (!vq) {
		err = -ENOMEM;
		goto error_available;
	}

	/* Activate the queue */
	iowrite32(virtqueue_get_desc_addr(vq) >> VIRTIO_PCI_QUEUE_ADDR_SHIFT,
		  priv->ioaddr + VIRTIO_PCI_QUEUE_PFN);

	return vq;

error_available:
	return ERR_PTR(err);
}

static void virtio_pci_del_vq(struct virtqueue *vq)
{
	struct virtio_pci_priv *priv = dev_get_priv(vq->vdev);
	unsigned int index = vq->index;

	iowrite16(index, priv->ioaddr + VIRTIO_PCI_QUEUE_SEL);

	/* Select and deactivate the queue */
	iowrite32(0, priv->ioaddr + VIRTIO_PCI_QUEUE_PFN);

	vring_del_virtqueue(vq);
}

static int virtio_pci_del_vqs(struct udevice *udev)
{
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);
	struct virtqueue *vq, *n;

	list_for_each_entry_safe(vq, n, &uc_priv->vqs, list)
		virtio_pci_del_vq(vq);

	return 0;
}

static int virtio_pci_find_vqs(struct udevice *udev, unsigned int nvqs,
			       struct virtqueue *vqs[])
{
	int i;

	for (i = 0; i < nvqs; ++i) {
		vqs[i] = virtio_pci_setup_vq(udev, i);
		if (IS_ERR(vqs[i])) {
			virtio_pci_del_vqs(udev);
			return PTR_ERR(vqs[i]);
		}
	}

	return 0;
}

static int virtio_pci_notify(struct udevice *udev, struct virtqueue *vq)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);

	/*
	 * We write the queue's selector into the notification register
	 * to signal the other end
	 */
	iowrite16(vq->index, priv->ioaddr + VIRTIO_PCI_QUEUE_NOTIFY);

	return 0;
}

static int virtio_pci_bind(struct udevice *udev)
{
	static int num_devs;
	char name[20];

	/* Create a unique device name for PCI type devices */
	sprintf(name, "%s#%u", VIRTIO_PCI_DRV_NAME, num_devs++);
	device_set_name(udev, name);

	return 0;
}

static int virtio_pci_probe(struct udevice *udev)
{
	struct pci_child_platdata *pplat = dev_get_parent_platdata(udev);
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);
	struct virtio_pci_priv *priv = dev_get_priv(udev);
	u16 subvendor, subdevice;
	u8 revision;

	/* We only own devices >= 0x1000 and <= 0x103f: leave the rest. */
	if (pplat->device < 0x1000 || pplat->device > 0x103f)
		return -ENODEV;

	/* Transitional devices must have a PCI revision ID of 0 */
	dm_pci_read_config8(udev, PCI_REVISION_ID, &revision);
	if (revision != VIRTIO_PCI_ABI_VERSION) {
		printf("(%s): virtio_pci expected ABI version %d, got %d\n",
		       udev->name, VIRTIO_PCI_ABI_VERSION, revision);
		return -ENODEV;
	}

	/*
	 * Transitional devices must have the PCI subsystem device ID matching
	 * the virtio device ID
	 */
	dm_pci_read_config16(udev, PCI_SUBSYSTEM_ID, &subdevice);
	dm_pci_read_config16(udev, PCI_SUBSYSTEM_VENDOR_ID, &subvendor);
	uc_priv->device = subdevice;
	uc_priv->vendor = subvendor;

	priv->ioaddr = dm_pci_map_bar(udev, PCI_BASE_ADDRESS_0, PCI_REGION_IO);
	if (!priv->ioaddr)
		return -ENXIO;
	debug("(%s): virtio legacy device reg base %04lx\n",
	      udev->name, (ulong)priv->ioaddr);

	debug("(%s): device (%d) vendor (%08x) version (%d)\n", udev->name,
	      uc_priv->device, uc_priv->vendor, revision);

	return 0;
}

static const struct dm_virtio_ops virtio_pci_ops = {
	.get_config	= virtio_pci_get_config,
	.set_config	= virtio_pci_set_config,
	.get_status	= virtio_pci_get_status,
	.set_status	= virtio_pci_set_status,
	.reset		= virtio_pci_reset,
	.get_features	= virtio_pci_get_features,
	.set_features	= virtio_pci_set_features,
	.find_vqs	= virtio_pci_find_vqs,
	.del_vqs	= virtio_pci_del_vqs,
	.notify		= virtio_pci_notify,
};

U_BOOT_DRIVER(virtio_pci_legacy) = {
	.name	= VIRTIO_PCI_DRV_NAME,
	.id	= UCLASS_VIRTIO,
	.ops	= &virtio_pci_ops,
	.bind	= virtio_pci_bind,
	.probe	= virtio_pci_probe,
	.priv_auto_alloc_size = sizeof(struct virtio_pci_priv),
};

static struct pci_device_id virtio_pci_supported[] = {
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID00) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID01) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID02) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID03) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID04) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID05) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID06) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID07) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID08) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID09) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID0A) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID0B) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID0C) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID0D) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID0E) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID0F) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID10) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID11) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID12) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID13) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID14) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID15) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID16) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID17) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID18) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID19) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID1A) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID1B) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID1C) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID1D) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID1E) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID1F) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID20) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID21) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID22) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID23) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID24) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID25) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID26) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID27) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID28) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID29) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID2A) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID2B) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID2C) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID2D) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID2E) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID2F) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID30) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID31) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID32) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID33) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID34) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID35) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID36) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID37) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID38) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID39) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID3A) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID3B) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID3C) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID3D) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID3E) },
	{ PCI_DEVICE(VIRTIO_PCI_VENDOR_ID, VIRTIO_PCI_DEVICE_ID3F) },
	{},
};

U_BOOT_PCI_DEVICE(virtio_pci_legacy, virtio_pci_supported);
