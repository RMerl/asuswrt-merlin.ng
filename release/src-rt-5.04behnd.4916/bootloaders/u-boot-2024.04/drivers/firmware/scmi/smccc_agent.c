// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Linaro Limited.
 */

#define LOG_CATEGORY UCLASS_SCMI_AGENT

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <dm/devres.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <linux/arm-smccc.h>
#include <linux/compat.h>

#include "smt.h"

#define SMCCC_RET_NOT_SUPPORTED         ((unsigned long)-1)

/**
 * struct scmi_smccc_channel - Description of an SCMI SMCCC transport
 * @func_id:	SMCCC function ID used by the SCMI transport
 * @smt:	Shared memory buffer
 */
struct scmi_smccc_channel {
	ulong func_id;
	struct scmi_smt smt;
};

/**
 * struct scmi_channel - Channel instance referenced in SCMI drivers
 * @ref: Reference to local channel instance
 **/
struct scmi_channel {
	struct scmi_smccc_channel ref;
};

static int scmi_smccc_process_msg(struct udevice *dev,
				  struct scmi_channel *channel,
				  struct scmi_msg *msg)
{
	struct scmi_smccc_channel *chan = &channel->ref;
	struct arm_smccc_res res;
	int ret;

	ret = scmi_write_msg_to_smt(dev, &chan->smt, msg);
	if (ret)
		return ret;

	arm_smccc_smc(chan->func_id, 0, 0, 0, 0, 0, 0, 0, &res);
	if (res.a0 == SMCCC_RET_NOT_SUPPORTED)
		ret = -ENXIO;
	else
		ret = scmi_read_resp_from_smt(dev, &chan->smt, msg);

	scmi_clear_smt_channel(&chan->smt);

	return ret;
}

static int setup_channel(struct udevice *dev, struct scmi_smccc_channel *chan)
{
	u32 func_id;
	int ret;

	if (dev_read_u32(dev, "arm,smc-id", &func_id)) {
		dev_err(dev, "Missing property func-id\n");
		return -EINVAL;
	}

	chan->func_id = func_id;

	ret = scmi_dt_get_smt_buffer(dev, &chan->smt);
	if (ret)
		dev_err(dev, "Failed to get smt resources: %d\n", ret);

	return ret;
}

static int scmi_smccc_get_channel(struct udevice *dev,
				  struct udevice *protocol,
				  struct scmi_channel **channel)
{
	struct scmi_smccc_channel *base_chan = dev_get_plat(dev);
	struct scmi_smccc_channel *chan;
	u32 func_id;
	int ret;

	if (dev_read_u32(protocol, "arm,smc-id", &func_id)) {
		/* Uses agent base channel */
		*channel = container_of(base_chan, struct scmi_channel, ref);

		return 0;
	}

	/* Setup a dedicated channel */
	chan = calloc(1, sizeof(*chan));
	if (!chan)
		return -ENOMEM;

	ret = setup_channel(protocol, chan);
	if (ret) {
		free(chan);
		return ret;
	}

	*channel = container_of(chan, struct scmi_channel, ref);

	return 0;
}

static int scmi_smccc_of_to_plat(struct udevice *dev)
{
	struct scmi_smccc_channel *chan = dev_get_plat(dev);

	return setup_channel(dev, chan);
}

static const struct udevice_id scmi_smccc_ids[] = {
	{ .compatible = "arm,scmi-smc" },
	{ }
};

static const struct scmi_agent_ops scmi_smccc_ops = {
	.of_get_channel = scmi_smccc_get_channel,
	.process_msg = scmi_smccc_process_msg,
};

U_BOOT_DRIVER(scmi_smccc) = {
	.name		= "scmi-over-smccc",
	.id		= UCLASS_SCMI_AGENT,
	.of_match	= scmi_smccc_ids,
	.plat_auto	= sizeof(struct scmi_smccc_channel),
	.of_to_plat	= scmi_smccc_of_to_plat,
	.ops		= &scmi_smccc_ops,
};
