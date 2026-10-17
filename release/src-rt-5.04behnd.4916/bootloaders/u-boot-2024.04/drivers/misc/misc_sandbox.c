// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <dm.h>
#include <misc.h>

struct misc_sandbox_priv {
	u8 mem[128];
	ulong last_ioctl;
	bool enabled;
};

int misc_sandbox_read(struct udevice *dev, int offset, void *buf, int size)
{
	struct misc_sandbox_priv *priv = dev_get_priv(dev);

	memcpy(buf, priv->mem + offset, size);

	return size;
}

int misc_sandbox_write(struct udevice *dev, int offset, const void *buf,
		       int size)
{
	struct misc_sandbox_priv *priv = dev_get_priv(dev);

	memcpy(priv->mem + offset, buf, size);

	return size;
}

int misc_sandbox_ioctl(struct udevice *dev, unsigned long request, void *buf)
{
	struct misc_sandbox_priv *priv = dev_get_priv(dev);

	priv->last_ioctl = request;

	return 0;
}

int misc_sandbox_call(struct udevice *dev, int msgid, void *tx_msg,
		      int tx_size, void *rx_msg, int rx_size)
{
	struct misc_sandbox_priv *priv = dev_get_priv(dev);

	if (msgid == 0) {
		int num = *(int *)tx_msg;

		switch (num) {
		case 0:
			strncpy(rx_msg, "Zero", rx_size);
			break;
		case 1:
			strncpy(rx_msg, "One", rx_size);
			break;
		case 2:
			strncpy(rx_msg, "Two", rx_size);
			break;
		default:
			return -EINVAL;
		}
	}

	if (msgid == 1) {
		int num = *(int *)tx_msg;

		switch (num) {
		case 0:
			strncpy(rx_msg, "Forty", rx_size);
			break;
		case 1:
			strncpy(rx_msg, "Forty-one", rx_size);
			break;
		case 2:
			strncpy(rx_msg, "Forty-two", rx_size);
			break;
		default:
			return -EINVAL;
		}
	}

	if (msgid == 2)
		memcpy(rx_msg, &priv->last_ioctl, sizeof(priv->last_ioctl));

	if (msgid == 3)
		memcpy(rx_msg, &priv->enabled, sizeof(priv->enabled));

	return 0;
}

int misc_sandbox_set_enabled(struct udevice *dev, bool val)
{
	struct misc_sandbox_priv *priv = dev_get_priv(dev);

	priv->enabled = !priv->enabled;

	return 0;
}

static const struct misc_ops misc_sandbox_ops = {
	.read = misc_sandbox_read,
	.write = misc_sandbox_write,
	.ioctl = misc_sandbox_ioctl,
	.call = misc_sandbox_call,
	.set_enabled = misc_sandbox_set_enabled,
};

int misc_sandbox_probe(struct udevice *dev)
{
	struct misc_sandbox_priv *priv = dev_get_priv(dev);
	/* For eth5 */
	const u8 mac[] = { 0x02, 0x00, 0x11, 0x22, 0x33, 0x46 };

	priv->enabled = true;
	memcpy(&priv->mem[16], mac, sizeof(mac));

	return 0;
}

static const struct udevice_id misc_sandbox_ids[] = {
	{ .compatible = "sandbox,misc_sandbox" },
	{ }
};

U_BOOT_DRIVER(misc_sandbox) = {
	.name           = "misc_sandbox",
	.id             = UCLASS_MISC,
	.ops		= &misc_sandbox_ops,
	.of_match       = misc_sandbox_ids,
	.probe          = misc_sandbox_probe,
	.priv_auto	= sizeof(struct misc_sandbox_priv),
};
