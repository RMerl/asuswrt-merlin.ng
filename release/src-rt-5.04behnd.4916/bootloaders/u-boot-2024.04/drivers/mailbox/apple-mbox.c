// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <dm.h>
#include <mailbox-uclass.h>
#include <asm/io.h>
#include <linux/apple-mailbox.h>
#include <linux/delay.h>

#define REG_A2I_STAT	0x110
#define  REG_A2I_STAT_EMPTY	BIT(17)
#define  REG_A2I_STAT_FULL	BIT(16)
#define REG_I2A_STAT	0x114
#define  REG_I2A_STAT_EMPTY	BIT(17)
#define  REG_I2A_STAT_FULL	BIT(16)
#define REG_A2I_MSG0	0x800
#define REG_A2I_MSG1	0x808
#define REG_I2A_MSG0	0x830
#define REG_I2A_MSG1	0x838

struct apple_mbox_priv {
	void *base;
};

static int apple_mbox_of_xlate(struct mbox_chan *chan,
			       struct ofnode_phandle_args *args)
{
	if (args->args_count != 0)
		return -EINVAL;

	return 0;
}

static int apple_mbox_send(struct mbox_chan *chan, const void *data)
{
	struct apple_mbox_priv *priv = dev_get_priv(chan->dev);
	const struct apple_mbox_msg *msg = data;

	writeq(msg->msg0, priv->base + REG_A2I_MSG0);
	writeq(msg->msg1, priv->base + REG_A2I_MSG1);
	while (readl(priv->base + REG_A2I_STAT) & REG_A2I_STAT_FULL)
		udelay(1);

	return 0;
}

static int apple_mbox_recv(struct mbox_chan *chan, void *data)
{
	struct apple_mbox_priv *priv = dev_get_priv(chan->dev);
	struct apple_mbox_msg *msg = data;

	if (readl(priv->base + REG_I2A_STAT) & REG_I2A_STAT_EMPTY)
		return -ENODATA;

	msg->msg0 = readq(priv->base + REG_I2A_MSG0);
	msg->msg1 = readq(priv->base + REG_I2A_MSG1);
	return 0;
}

struct mbox_ops apple_mbox_ops = {
	.of_xlate = apple_mbox_of_xlate,
	.send = apple_mbox_send,
	.recv = apple_mbox_recv,
};

static int apple_mbox_probe(struct udevice *dev)
{
	struct apple_mbox_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	return 0;
}

static const struct udevice_id apple_mbox_of_match[] = {
	{ .compatible = "apple,asc-mailbox-v4" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(apple_mbox) = {
	.name = "apple-mbox",
	.id = UCLASS_MAILBOX,
	.of_match = apple_mbox_of_match,
	.probe = apple_mbox_probe,
	.priv_auto = sizeof(struct apple_mbox_priv),
	.ops = &apple_mbox_ops,
};
