// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <mailbox-uclass.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/mbox.h>

#define SANDBOX_MBOX_CHANNELS 2

struct sandbox_mbox_chan {
	bool rx_msg_valid;
	uint32_t rx_msg;
};

struct sandbox_mbox {
	struct sandbox_mbox_chan chans[SANDBOX_MBOX_CHANNELS];
};

static int sandbox_mbox_request(struct mbox_chan *chan)
{
	debug("%s(chan=%p)\n", __func__, chan);

	if (chan->id >= SANDBOX_MBOX_CHANNELS)
		return -EINVAL;

	return 0;
}

static int sandbox_mbox_free(struct mbox_chan *chan)
{
	debug("%s(chan=%p)\n", __func__, chan);

	return 0;
}

static int sandbox_mbox_send(struct mbox_chan *chan, const void *data)
{
	struct sandbox_mbox *sbm = dev_get_priv(chan->dev);
	const uint32_t *pmsg = data;

	debug("%s(chan=%p, data=%p)\n", __func__, chan, data);

	sbm->chans[chan->id].rx_msg = *pmsg ^ SANDBOX_MBOX_PING_XOR;
	sbm->chans[chan->id].rx_msg_valid = true;

	return 0;
}

static int sandbox_mbox_recv(struct mbox_chan *chan, void *data)
{
	struct sandbox_mbox *sbm = dev_get_priv(chan->dev);
	uint32_t *pmsg = data;

	debug("%s(chan=%p, data=%p)\n", __func__, chan, data);

	if (!sbm->chans[chan->id].rx_msg_valid)
		return -ENODATA;

	*pmsg = sbm->chans[chan->id].rx_msg;
	sbm->chans[chan->id].rx_msg_valid = false;

	return 0;
}

static int sandbox_mbox_bind(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static int sandbox_mbox_probe(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	return 0;
}

static const struct udevice_id sandbox_mbox_ids[] = {
	{ .compatible = "sandbox,mbox" },
	{ }
};

struct mbox_ops sandbox_mbox_mbox_ops = {
	.request = sandbox_mbox_request,
	.rfree = sandbox_mbox_free,
	.send = sandbox_mbox_send,
	.recv = sandbox_mbox_recv,
};

U_BOOT_DRIVER(sandbox_mbox) = {
	.name = "sandbox_mbox",
	.id = UCLASS_MAILBOX,
	.of_match = sandbox_mbox_ids,
	.bind = sandbox_mbox_bind,
	.probe = sandbox_mbox_probe,
	.priv_auto	= sizeof(struct sandbox_mbox),
	.ops = &sandbox_mbox_mbox_ops,
};
