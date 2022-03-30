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

#define VIRTIO_PCI_DRV_NAME	"virtio-pci.m"

/* PCI device ID in the range 0x1040 to 0x107f */
#define VIRTIO_PCI_VENDOR_ID	0x1af4
#define VIRTIO_PCI_DEVICE_ID00	0x1040
#define VIRTIO_PCI_DEVICE_ID01	0x1041
#define VIRTIO_PCI_DEVICE_ID02	0x1042
#define VIRTIO_PCI_DEVICE_ID03	0x1043
#define VIRTIO_PCI_DEVICE_ID04	0x1044
#define VIRTIO_PCI_DEVICE_ID05	0x1045
#define VIRTIO_PCI_DEVICE_ID06	0x1046
#define VIRTIO_PCI_DEVICE_ID07	0x1047
#define VIRTIO_PCI_DEVICE_ID08	0x1048
#define VIRTIO_PCI_DEVICE_ID09	0x1049
#define VIRTIO_PCI_DEVICE_ID0A	0x104a
#define VIRTIO_PCI_DEVICE_ID0B	0x104b
#define VIRTIO_PCI_DEVICE_ID0C	0x104c
#define VIRTIO_PCI_DEVICE_ID0D	0x104d
#define VIRTIO_PCI_DEVICE_ID0E	0x104e
#define VIRTIO_PCI_DEVICE_ID0F	0x104f
#define VIRTIO_PCI_DEVICE_ID10	0x1050
#define VIRTIO_PCI_DEVICE_ID11	0x1051
#define VIRTIO_PCI_DEVICE_ID12	0x1052
#define VIRTIO_PCI_DEVICE_ID13	0x1053
#define VIRTIO_PCI_DEVICE_ID14	0x1054
#define VIRTIO_PCI_DEVICE_ID15	0x1055
#define VIRTIO_PCI_DEVICE_ID16	0x1056
#define VIRTIO_PCI_DEVICE_ID17	0x1057
#define VIRTIO_PCI_DEVICE_ID18	0x1058
#define VIRTIO_PCI_DEVICE_ID19	0x1059
#define VIRTIO_PCI_DEVICE_ID1A	0x105a
#define VIRTIO_PCI_DEVICE_ID1B	0x105b
#define VIRTIO_PCI_DEVICE_ID1C	0x105c
#define VIRTIO_PCI_DEVICE_ID1D	0x105d
#define VIRTIO_PCI_DEVICE_ID1E	0x105e
#define VIRTIO_PCI_DEVICE_ID1F	0x105f
#define VIRTIO_PCI_DEVICE_ID20	0x1060
#define VIRTIO_PCI_DEVICE_ID21	0x1061
#define VIRTIO_PCI_DEVICE_ID22	0x1062
#define VIRTIO_PCI_DEVICE_ID23	0x1063
#define VIRTIO_PCI_DEVICE_ID24	0x1064
#define VIRTIO_PCI_DEVICE_ID25	0x1065
#define VIRTIO_PCI_DEVICE_ID26	0x1066
#define VIRTIO_PCI_DEVICE_ID27	0x1067
#define VIRTIO_PCI_DEVICE_ID28	0x1068
#define VIRTIO_PCI_DEVICE_ID29	0x1069
#define VIRTIO_PCI_DEVICE_ID2A	0x106a
#define VIRTIO_PCI_DEVICE_ID2B	0x106b
#define VIRTIO_PCI_DEVICE_ID2C	0x106c
#define VIRTIO_PCI_DEVICE_ID2D	0x106d
#define VIRTIO_PCI_DEVICE_ID2E	0x106e
#define VIRTIO_PCI_DEVICE_ID2F	0x106f
#define VIRTIO_PCI_DEVICE_ID30	0x1070
#define VIRTIO_PCI_DEVICE_ID31	0x1071
#define VIRTIO_PCI_DEVICE_ID32	0x1072
#define VIRTIO_PCI_DEVICE_ID33	0x1073
#define VIRTIO_PCI_DEVICE_ID34	0x1074
#define VIRTIO_PCI_DEVICE_ID35	0x1075
#define VIRTIO_PCI_DEVICE_ID36	0x1076
#define VIRTIO_PCI_DEVICE_ID37	0x1077
#define VIRTIO_PCI_DEVICE_ID38	0x1078
#define VIRTIO_PCI_DEVICE_ID39	0x1079
#define VIRTIO_PCI_DEVICE_ID3A	0x107a
#define VIRTIO_PCI_DEVICE_ID3B	0x107b
#define VIRTIO_PCI_DEVICE_ID3C	0x107c
#define VIRTIO_PCI_DEVICE_ID3D	0x107d
#define VIRTIO_PCI_DEVICE_ID3E	0x107e
#define VIRTIO_PCI_DEVICE_ID3F	0x107f

/**
 * virtio pci transport driver private data
 *
 * @common: pci transport device common register block base
 * @notify_base: pci transport device notify register block base
 * @device: pci transport device device-specific register block base
 * @device_len: pci transport device device-specific register block length
 * @notify_offset_multiplier: multiply queue_notify_off by this value
 */
struct virtio_pci_priv {
	struct virtio_pci_common_cfg __iomem *common;
	void __iomem *notify_base;
	void __iomem *device;
	u32 device_len;
	u32 notify_offset_multiplier;
};

static int virtio_pci_get_config(struct udevice *udev, unsigned int offset,
				 void *buf, unsigned int len)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);
	u8 b;
	__le16 w;
	__le32 l;

	WARN_ON(offset + len > priv->device_len);

	switch (len) {
	case 1:
		b = ioread8(priv->device + offset);
		memcpy(buf, &b, sizeof(b));
		break;
	case 2:
		w = cpu_to_le16(ioread16(priv->device + offset));
		memcpy(buf, &w, sizeof(w));
		break;
	case 4:
		l = cpu_to_le32(ioread32(priv->device + offset));
		memcpy(buf, &l, sizeof(l));
		break;
	case 8:
		l = cpu_to_le32(ioread32(priv->device + offset));
		memcpy(buf, &l, sizeof(l));
		l = cpu_to_le32(ioread32(priv->device + offset + sizeof(l)));
		memcpy(buf + sizeof(l), &l, sizeof(l));
		break;
	default:
		WARN_ON(true);
	}

	return 0;
}

static int virtio_pci_set_config(struct udevice *udev, unsigned int offset,
				 const void *buf, unsigned int len)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);
	u8 b;
	__le16 w;
	__le32 l;

	WARN_ON(offset + len > priv->device_len);

	switch (len) {
	case 1:
		memcpy(&b, buf, sizeof(b));
		iowrite8(b, priv->device + offset);
		break;
	case 2:
		memcpy(&w, buf, sizeof(w));
		iowrite16(le16_to_cpu(w), priv->device + offset);
		break;
	case 4:
		memcpy(&l, buf, sizeof(l));
		iowrite32(le32_to_cpu(l), priv->device + offset);
		break;
	case 8:
		memcpy(&l, buf, sizeof(l));
		iowrite32(le32_to_cpu(l), priv->device + offset);
		memcpy(&l, buf + sizeof(l), sizeof(l));
		iowrite32(le32_to_cpu(l), priv->device + offset + sizeof(l));
		break;
	default:
		WARN_ON(true);
	}

	return 0;
}

static int virtio_pci_generation(struct udevice *udev, u32 *counter)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);

	*counter = ioread8(&priv->common->config_generation);

	return 0;
}

static int virtio_pci_get_status(struct udevice *udev, u8 *status)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);

	*status = ioread8(&priv->common->device_status);

	return 0;
}

static int virtio_pci_set_status(struct udevice *udev, u8 status)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);

	/* We should never be setting status to 0 */
	WARN_ON(status == 0);

	iowrite8(status, &priv->common->device_status);

	return 0;
}

static int virtio_pci_reset(struct udevice *udev)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);

	/* 0 status means a reset */
	iowrite8(0, &priv->common->device_status);

	/*
	 * After writing 0 to device_status, the driver MUST wait for a read
	 * of device_status to return 0 before reinitializing the device.
	 * This will flush out the status write, and flush in device writes,
	 * including MSI-X interrupts, if any.
	 */
	while (ioread8(&priv->common->device_status))
		udelay(1000);

	return 0;
}

static int virtio_pci_get_features(struct udevice *udev, u64 *features)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);

	iowrite32(0, &priv->common->device_feature_select);
	*features = ioread32(&priv->common->device_feature);
	iowrite32(1, &priv->common->device_feature_select);
	*features |= ((u64)ioread32(&priv->common->device_feature) << 32);

	return 0;
}

static int virtio_pci_set_features(struct udevice *udev)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);

	if (!__virtio_test_bit(udev, VIRTIO_F_VERSION_1)) {
		debug("virtio: device uses modern interface but does not have VIRTIO_F_VERSION_1\n");
		return -EINVAL;
	}

	iowrite32(0, &priv->common->guest_feature_select);
	iowrite32((u32)uc_priv->features, &priv->common->guest_feature);
	iowrite32(1, &priv->common->guest_feature_select);
	iowrite32(uc_priv->features >> 32, &priv->common->guest_feature);

	return 0;
}

static struct virtqueue *virtio_pci_setup_vq(struct udevice *udev,
					     unsigned int index)
{
	struct virtio_pci_priv *priv = dev_get_priv(udev);
	struct virtio_pci_common_cfg __iomem *cfg = priv->common;
	struct virtqueue *vq;
	u16 num;
	u64 addr;
	int err;

	if (index >= ioread16(&cfg->num_queues))
		return ERR_PTR(-ENOENT);

	/* Select the queue we're interested in */
	iowrite16(index, &cfg->queue_select);

	/* Check if queue is either not available or already active */
	num = ioread16(&cfg->queue_size);
	if (!num || ioread16(&cfg->queue_enable))
		return ERR_PTR(-ENOENT);

	if (num & (num - 1)) {
		printf("(%s): bad queue size %u", udev->name, num);
		return ERR_PTR(-EINVAL);
	}

	/* Create the vring */
	vq = vring_create_virtqueue(index, num, VIRTIO_PCI_VRING_ALIGN, udev);
	if (!vq) {
		err = -ENOMEM;
		goto error_available;
	}

	/* Activate the queue */
	iowrite16(virtqueue_get_vring_size(vq), &cfg->queue_size);

	addr = virtqueue_get_desc_addr(vq);
	iowrite32((u32)addr, &cfg->queue_desc_lo);
	iowrite32(addr >> 32, &cfg->queue_desc_hi);

	addr = virtqueue_get_avail_addr(vq);
	iowrite32((u32)addr, &cfg->queue_avail_lo);
	iowrite32(addr >> 32, &cfg->queue_avail_hi);

	addr = virtqueue_get_used_addr(vq);
	iowrite32((u32)addr, &cfg->queue_used_lo);
	iowrite32(addr >> 32, &cfg->queue_used_hi);

	iowrite16(1, &cfg->queue_enable);

	return vq;

error_available:
	return ERR_PTR(err);
}

static void virtio_pci_del_vq(struct virtqueue *vq)
{
	struct virtio_pci_priv *priv = dev_get_priv(vq->vdev);
	unsigned int index = vq->index;

	iowrite16(index, &priv->common->queue_select);

	/* Select and deactivate the queue */
	iowrite16(0, &priv->common->queue_enable);

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
	u16 off;

	/* Select the queue we're interested in */
	iowrite16(vq->index, &priv->common->queue_select);

	/* get offset of notification word for this vq */
	off = ioread16(&priv->common->queue_notify_off);

	/*
	 * We write the queue's selector into the notification register
	 * to signal the other end
	 */
	iowrite16(vq->index,
		  priv->notify_base + off * priv->notify_offset_multiplier);

	return 0;
}

/**
 * virtio_pci_find_capability - walk capabilities to find device info
 *
 * @udev:	the transport device
 * @cfg_type:	the VIRTIO_PCI_CAP_* value we seek
 *
 * @return offset of the configuration structure
 */
static int virtio_pci_find_capability(struct udevice *udev, u8 cfg_type)
{
	int pos;
	int offset;
	u8 type, bar;

	for (pos = dm_pci_find_capability(udev, PCI_CAP_ID_VNDR);
	     pos > 0;
	     pos = dm_pci_find_next_capability(udev, pos, PCI_CAP_ID_VNDR)) {
		offset = pos + offsetof(struct virtio_pci_cap, cfg_type);
		dm_pci_read_config8(udev, offset, &type);
		offset = pos + offsetof(struct virtio_pci_cap, bar);
		dm_pci_read_config8(udev, offset, &bar);

		/* Ignore structures with reserved BAR values */
		if (bar > 0x5)
			continue;

		if (type == cfg_type)
			return pos;
	}

	return 0;
}

/**
 * virtio_pci_map_capability - map base address of the capability
 *
 * @udev:	the transport device
 * @off:	offset of the configuration structure
 *
 * @return base address of the capability
 */
static void __iomem *virtio_pci_map_capability(struct udevice *udev, int off)
{
	u8 bar;
	u32 offset;
	ulong base;
	void __iomem *p;

	if (!off)
		return NULL;

	offset = off + offsetof(struct virtio_pci_cap, bar);
	dm_pci_read_config8(udev, offset, &bar);
	offset = off + offsetof(struct virtio_pci_cap, offset);
	dm_pci_read_config32(udev, offset, &offset);

	/*
	 * TODO: adding 64-bit BAR support
	 *
	 * Per spec, the BAR is permitted to be either 32-bit or 64-bit.
	 * For simplicity, only read the BAR address as 32-bit.
	 */
	base = dm_pci_read_bar32(udev, bar);
	p = (void __iomem *)base + offset;

	return p;
}

static int virtio_pci_bind(struct udevice *udev)
{
	static int num_devs;
	char name[20];

	/* Create a unique device name  */
	sprintf(name, "%s#%u", VIRTIO_PCI_DRV_NAME, num_devs++);
	device_set_name(udev, name);

	return 0;
}

static int virtio_pci_probe(struct udevice *udev)
{
	struct pci_child_platdata *pplat = dev_get_parent_platdata(udev);
	struct virtio_dev_priv *uc_priv = dev_get_uclass_priv(udev);
	struct virtio_pci_priv *priv = dev_get_priv(udev);
	u16 subvendor;
	u8 revision;
	int common, notify, device;
	int offset;

	/* We only own devices >= 0x1040 and <= 0x107f: leave the rest. */
	if (pplat->device < 0x1040 || pplat->device > 0x107f)
		return -ENODEV;

	/* Transitional devices must not have a PCI revision ID of 0 */
	dm_pci_read_config8(udev, PCI_REVISION_ID, &revision);

	/* Modern devices: simply use PCI device id, but start from 0x1040. */
	uc_priv->device = pplat->device - 0x1040;
	dm_pci_read_config16(udev, PCI_SUBSYSTEM_VENDOR_ID, &subvendor);
	uc_priv->vendor = subvendor;

	/* Check for a common config: if not, use legacy mode (bar 0) */
	common = virtio_pci_find_capability(udev, VIRTIO_PCI_CAP_COMMON_CFG);
	if (!common) {
		printf("(%s): leaving for legacy driver\n", udev->name);
		return -ENODEV;
	}

	/* If common is there, notify should be too */
	notify = virtio_pci_find_capability(udev, VIRTIO_PCI_CAP_NOTIFY_CFG);
	if (!notify) {
		printf("(%s): missing capabilities %i/%i\n", udev->name,
		       common, notify);
		return -EINVAL;
	}

	/*
	 * Device capability is only mandatory for devices that have
	 * device-specific configuration.
	 */
	device = virtio_pci_find_capability(udev, VIRTIO_PCI_CAP_DEVICE_CFG);
	if (device) {
		offset = notify + offsetof(struct virtio_pci_cap, length);
		dm_pci_read_config32(udev, offset, &priv->device_len);
	}

	/* Map configuration structures */
	priv->common = virtio_pci_map_capability(udev, common);
	priv->notify_base = virtio_pci_map_capability(udev, notify);
	priv->device = virtio_pci_map_capability(udev, device);
	debug("(%p): common @ %p, notify base @ %p, device @ %p\n",
	      udev, priv->common, priv->notify_base, priv->device);

	/* Read notify_off_multiplier from config space */
	offset = notify + offsetof(struct virtio_pci_notify_cap,
				   notify_off_multiplier);
	dm_pci_read_config32(udev, offset, &priv->notify_offset_multiplier);

	debug("(%s): device (%d) vendor (%08x) version (%d)\n", udev->name,
	      uc_priv->device, uc_priv->vendor, revision);

	return 0;
}

static const struct dm_virtio_ops virtio_pci_ops = {
	.get_config	= virtio_pci_get_config,
	.set_config	= virtio_pci_set_config,
	.generation	= virtio_pci_generation,
	.get_status	= virtio_pci_get_status,
	.set_status	= virtio_pci_set_status,
	.reset		= virtio_pci_reset,
	.get_features	= virtio_pci_get_features,
	.set_features	= virtio_pci_set_features,
	.find_vqs	= virtio_pci_find_vqs,
	.del_vqs	= virtio_pci_del_vqs,
	.notify		= virtio_pci_notify,
};

U_BOOT_DRIVER(virtio_pci_modern) = {
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

U_BOOT_PCI_DEVICE(virtio_pci_modern, virtio_pci_supported);
