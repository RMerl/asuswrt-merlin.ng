/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-2020 Linaro Limited.
 */
#ifndef _SCMI_AGENT_UCLASS_H
#define _SCMI_AGENT_UCLASS_H

#include <scmi_protocols.h>
#include <dm/device.h>

struct scmi_msg;
struct scmi_channel;

/**
 * struct scmi_agent_priv - private data maintained by agent instance
 * @version:		Version
 * @num_agents:		Number of agents
 * @num_protocols:	Number of protocols
 * @impl_version:	Implementation version
 * @protocols:		Array of protocol IDs
 * @vendor:		Vendor name
 * @sub_vendor:		Sub-vendor name
 * @agent_name:		Agent name
 * @agent_id:		Identifier of agent
 * @base_dev:		SCMI base protocol device
 * @pwdom_dev:		SCMI power domain management protocol device
 * @clock_dev:		SCMI clock protocol device
 * @resetdom_dev:	SCMI reset domain protocol device
 * @voltagedom_dev:	SCMI voltage domain protocol device
 */
struct scmi_agent_priv {
	u32 version;
	u32 num_agents;
	u32 num_protocols;
	u32 impl_version;
	u8 *protocols;
	u8 *vendor;
	u8 *sub_vendor;
	u8 *agent_name;
	u32 agent_id;
	struct udevice *base_dev;
	struct udevice *pwdom_dev;
	struct udevice *clock_dev;
	struct udevice *resetdom_dev;
	struct udevice *voltagedom_dev;
};

static inline u32 scmi_version(struct udevice *dev)
{
	return ((struct scmi_agent_priv *)dev_get_uclass_plat(dev))->version;
}

static inline u32 scmi_num_agents(struct udevice *dev)
{
	return ((struct scmi_agent_priv *)dev_get_uclass_plat(dev))->num_agents;
}

static inline u32 scmi_num_protocols(struct udevice *dev)
{
	return ((struct scmi_agent_priv *)dev_get_uclass_plat(dev))->num_protocols;
}

static inline u32 scmi_impl_version(struct udevice *dev)
{
	return ((struct scmi_agent_priv *)dev_get_uclass_plat(dev))->impl_version;
}

static inline u8 *scmi_protocols(struct udevice *dev)
{
	return ((struct scmi_agent_priv *)dev_get_uclass_plat(dev))->protocols;
}

static inline u8 *scmi_vendor(struct udevice *dev)
{
	return ((struct scmi_agent_priv *)dev_get_uclass_plat(dev))->vendor;
}

static inline u8 *scmi_sub_vendor(struct udevice *dev)
{
	return ((struct scmi_agent_priv *)dev_get_uclass_plat(dev))->sub_vendor;
}

static inline u8 *scmi_agent_name(struct udevice *dev)
{
	return ((struct scmi_agent_priv *)dev_get_uclass_plat(dev))->agent_name;
}

static inline u32 scmi_agent_id(struct udevice *dev)
{
	return ((struct scmi_agent_priv *)dev_get_uclass_plat(dev))->agent_id;
}

/**
 * struct scmi_transport_ops - The functions that a SCMI transport layer must implement.
 */
struct scmi_agent_ops {
	/*
	 * of_get_channel - Get SCMI channel from SCMI agent device tree node
	 *
	 * @dev:		SCMI agent device using the transport
	 * @protocol:		SCMI protocol device using the transport
	 * @channel:		Output reference to SCMI channel upon success
	 * Return 0 upon success and a negative errno on failure
	 */
	int (*of_get_channel)(struct udevice *dev, struct udevice *protocol,
			      struct scmi_channel **channel);

	/*
	 * process_msg - Request transport to get the SCMI message processed
	 *
	 * @dev:		SCMI agent device using the transport
	 * @msg:		SCMI message to be transmitted
	 */
	int (*process_msg)(struct udevice *dev, struct scmi_channel *channel,
			   struct scmi_msg *msg);
};

#endif /* _SCMI_TRANSPORT_UCLASS_H */
