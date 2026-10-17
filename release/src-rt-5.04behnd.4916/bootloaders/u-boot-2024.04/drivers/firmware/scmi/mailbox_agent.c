// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Linaro Limited.
 */

#define LOG_CATEGORY UCLASS_SCMI_AGENT

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <mailbox.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/compat.h>

#include "smt.h"

#define TIMEOUT_US_10MS			10000

/**
 * struct scmi_mbox_channel - Description of an SCMI mailbox transport
 * @smt:	Shared memory buffer
 * @mbox:	Mailbox channel description
 * @timeout_us:	Timeout in microseconds for the mailbox transfer
 */
struct scmi_mbox_channel {
	struct scmi_smt smt;
	struct mbox_chan mbox;
	ulong timeout_us;
};

/**
 * struct scmi_channel - Channel instance referenced in SCMI drivers
 * @ref: Reference to local channel instance
 **/
struct scmi_channel {
	struct scmi_mbox_channel ref;
};

static int scmi_mbox_process_msg(struct udevice *dev,
				 struct scmi_channel *channel,
				 struct scmi_msg *msg)
{
	struct scmi_mbox_channel *chan = &channel->ref;
	int ret;

	ret = scmi_write_msg_to_smt(dev, &chan->smt, msg);
	if (ret)
		return ret;

	/* Give shm addr to mbox in case it is meaningful */
	ret = mbox_send(&chan->mbox, chan->smt.buf);
	if (ret) {
		dev_err(dev, "Message send failed: %d\n", ret);
		goto out;
	}

	/* Receive the response */
	ret = mbox_recv(&chan->mbox, chan->smt.buf, chan->timeout_us);
	if (ret) {
		dev_err(dev, "Response failed: %d, abort\n", ret);
		goto out;
	}

	ret = scmi_read_resp_from_smt(dev, &chan->smt, msg);

out:
	scmi_clear_smt_channel(&chan->smt);

	return ret;
}

static int setup_channel(struct udevice *dev, struct scmi_mbox_channel *chan)
{
	int ret;

	ret = mbox_get_by_index(dev, 0, &chan->mbox);
	if (ret) {
		dev_err(dev, "Failed to find mailbox: %d\n", ret);
		return ret;
	}

	ret = scmi_dt_get_smt_buffer(dev, &chan->smt);
	if (ret) {
		dev_err(dev, "Failed to get shm resources: %d\n", ret);
		return ret;
	}

	chan->timeout_us = TIMEOUT_US_10MS;

	return 0;
}

static int scmi_mbox_get_channel(struct udevice *dev,
				 struct udevice *protocol,
				 struct scmi_channel **channel)
{
	struct scmi_mbox_channel *base_chan = dev_get_plat(dev);
	struct scmi_mbox_channel *chan;
	int ret;

	if (!dev_read_prop(protocol, "shmem", NULL)) {
		/* Uses agent base channel */
		*channel = container_of(base_chan, struct scmi_channel, ref);

		return 0;
	}

	chan = calloc(1, sizeof(*chan));
	if (!chan)
		return -ENOMEM;

	/* Setup a dedicated channel for the protocol */
	ret = setup_channel(protocol, chan);
	if (ret) {
		free(chan);
		return ret;
	}

	*channel = (void *)chan;

	return 0;
}

int scmi_mbox_of_to_plat(struct udevice *dev)
{
	struct scmi_mbox_channel *chan = dev_get_plat(dev);

	return setup_channel(dev, chan);
}

static const struct udevice_id scmi_mbox_ids[] = {
	{ .compatible = "arm,scmi" },
	{ }
};

static const struct scmi_agent_ops scmi_mbox_ops = {
	.of_get_channel = scmi_mbox_get_channel,
	.process_msg = scmi_mbox_process_msg,
};

U_BOOT_DRIVER(scmi_mbox) = {
	.name		= "scmi-over-mailbox",
	.id		= UCLASS_SCMI_AGENT,
	.of_match	= scmi_mbox_ids,
	.plat_auto	= sizeof(struct scmi_mbox_channel),
	.of_to_plat	= scmi_mbox_of_to_plat,
	.ops		= &scmi_mbox_ops,
};
