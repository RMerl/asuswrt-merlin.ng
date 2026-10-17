// SPDX-License-Identifier: GPL-2.0+
/*
 * SCMI Base protocol as U-Boot device
 *
 * Copyright (C) 2023 Linaro Limited
 *		author: AKASHI Takahiro <takahiro.akashi@linaro.org>
 */

#include <common.h>
#include <dm.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <stdlib.h>
#include <string.h>
#include <asm/types.h>
#include <dm/device_compat.h>
#include <linux/kernel.h>

/**
 * scmi_generic_protocol_version - get protocol version
 * @dev:	SCMI device
 * @id:		SCMI protocol ID
 * @version:	Pointer to SCMI protocol version
 *
 * Obtain the protocol version number in @version.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_generic_protocol_version(struct udevice *dev,
				  enum scmi_std_protocol id, u32 *version)
{
	struct scmi_protocol_version_out out;
	struct scmi_msg msg = {
		.protocol_id = id,
		.message_id = SCMI_PROTOCOL_VERSION,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*version = out.version;

	return 0;
}

/**
 * scmi_base_protocol_version_int - get Base protocol version
 * @dev:	SCMI device
 * @version:	Pointer to SCMI protocol version
 *
 * Obtain the protocol version number in @version for Base protocol.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_base_protocol_version_int(struct udevice *dev, u32 *version)
{
	return scmi_generic_protocol_version(dev, SCMI_PROTOCOL_ID_BASE,
					     version);
}

/**
 * scmi_protocol_attrs_int - get protocol attributes
 * @dev:		SCMI device
 * @num_agents:		Number of SCMI agents
 * @num_protocols:	Number of SCMI protocols
 *
 * Obtain the protocol attributes, the number of agents and the number
 * of protocols, in @num_agents and @num_protocols respectively, that
 * the device provides.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_protocol_attrs_int(struct udevice *dev, u32 *num_agents,
				   u32 *num_protocols)
{
	struct scmi_protocol_attrs_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_PROTOCOL_ATTRIBUTES,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*num_agents = SCMI_PROTOCOL_ATTRS_NUM_AGENTS(out.attributes);
	*num_protocols = SCMI_PROTOCOL_ATTRS_NUM_PROTOCOLS(out.attributes);

	return 0;
}

/**
 * scmi_protocol_message_attrs_int - get message-specific attributes
 * @dev:		SCMI device
 * @message_id:		SCMI message ID
 * @attributes:		Message-specific attributes
 *
 * Obtain the message-specific attributes in @attributes.
 * This command succeeds if the message is implemented and available.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_protocol_message_attrs_int(struct udevice *dev, u32 message_id,
					   u32 *attributes)
{
	struct scmi_protocol_msg_attrs_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_PROTOCOL_MESSAGE_ATTRIBUTES,
		.in_msg = (u8 *)&message_id,
		.in_msg_sz = sizeof(message_id),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*attributes = out.attributes;

	return 0;
}

/**
 * scmi_base_discover_vendor_int - get vendor name
 * @dev:	SCMI device
 * @vendor:	Pointer to vendor name
 *
 * Obtain the vendor's name in @vendor.
 * It is a caller's responsibility to free @vendor.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_base_discover_vendor_int(struct udevice *dev, u8 **vendor)
{
	struct scmi_base_discover_vendor_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_BASE_DISCOVER_VENDOR,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	if (!vendor)
		return -EINVAL;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*vendor = strdup(out.vendor_identifier);
	if (!*vendor)
		return -ENOMEM;

	return 0;
}

/**
 * scmi_base_discover_sub_vendor_int - get sub-vendor name
 * @dev:	SCMI device
 * @sub_vendor:	Pointer to sub-vendor name
 *
 * Obtain the sub-vendor's name in @sub_vendor.
 * It is a caller's responsibility to free @sub_vendor.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_base_discover_sub_vendor_int(struct udevice *dev,
					     u8 **sub_vendor)
{
	struct scmi_base_discover_vendor_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_BASE_DISCOVER_SUB_VENDOR,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	if (!sub_vendor)
		return -EINVAL;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*sub_vendor = strdup(out.vendor_identifier);
	if (!*sub_vendor)
		return -ENOMEM;

	return 0;
}

/**
 * scmi_base_discover_impl_version_int - get implementation version
 * @dev:		SCMI device
 * @impl_version:	Pointer to implementation version
 *
 * Obtain the implementation version number in @impl_version.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_base_discover_impl_version_int(struct udevice *dev,
					       u32 *impl_version)
{
	struct scmi_base_discover_impl_version_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_BASE_DISCOVER_IMPL_VERSION,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*impl_version = out.impl_version;

	return 0;
}

/**
 * scmi_base_discover_list_protocols_int - get list of protocols
 * @dev:	SCMI device
 * @protocols:	Pointer to array of SCMI protocols
 *
 * Obtain the list of protocols provided in @protocols.
 * The number of elements in @protocols always match to the number of
 * protocols returned by smci_protocol_attrs() when this function succeeds.
 * It is a caller's responsibility to free @protocols.
 *
 * Return: the number of protocols in @protocols on success, error code on
 * failure
 */
static int scmi_base_discover_list_protocols_int(struct udevice *dev,
						 u8 **protocols)
{
	struct scmi_base_discover_list_protocols_out out;
	int cur;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_BASE_DISCOVER_LIST_PROTOCOLS,
		.in_msg = (u8 *)&cur,
		.in_msg_sz = sizeof(cur),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	u32 num_agents, num_protocols;
	u8 *buf;
	int i, ret;

	ret = scmi_base_protocol_attrs(dev, &num_agents, &num_protocols);
	if (ret)
		return ret;

	buf = calloc(sizeof(u8), num_protocols);
	if (!buf)
		return -ENOMEM;

	cur = 0;
	do {
		ret = devm_scmi_process_msg(dev, &msg);
		if (ret)
			goto err;
		if (out.status) {
			ret = scmi_to_linux_errno(out.status);
			goto err;
		}

		for (i = 0; i < out.num_protocols; i++, cur++)
			buf[cur] = out.protocols[i / 4] >> ((i % 4) * 8);
	} while (cur < num_protocols);

	*protocols = buf;

	return num_protocols;
err:
	free(buf);

	return ret;
}

/**
 * scmi_base_discover_agent_int - identify agent
 * @dev:		SCMI device
 * @agent_id:		SCMI agent ID
 * @ret_agent_id:	Pointer to SCMI agent ID
 * @name:		Pointer to SCMI agent name
 *
 * Obtain the agent's name in @name. If @agent_id is equal to 0xffffffff,
 * this function returns the caller's agent id in @ret_agent_id.
 * It is a caller's responsibility to free @name.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_base_discover_agent_int(struct udevice *dev, u32 agent_id,
					u32 *ret_agent_id, u8 **name)
{
	struct scmi_base_discover_agent_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_BASE_DISCOVER_AGENT,
		.in_msg = (u8 *)&agent_id,
		.in_msg_sz = sizeof(agent_id),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	if (ret_agent_id)
		*ret_agent_id = out.agent_id;
	if (name) {
		*name = strdup(out.name);
		if (!*name)
			return -ENOMEM;
	}

	return 0;
}

/**
 * scmi_base_set_device_permissions_int - configure access permission to device
 * @dev:	SCMI device
 * @agent_id:	SCMI agent ID
 * @device_id:	ID of device to access
 * @flags:	A set of flags
 *
 * Ask for allowing or denying access permission to the device, @device_id.
 * The meaning of @flags is defined in SCMI specification.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_base_set_device_permissions_int(struct udevice *dev, u32 agent_id,
						u32 device_id, u32 flags)
{
	struct scmi_base_set_device_permissions_in in = {
		.agent_id = agent_id,
		.device_id = device_id,
		.flags = flags,
	};
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_BASE_SET_DEVICE_PERMISSIONS,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&status,
		.out_msg_sz = sizeof(status),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (status)
		return scmi_to_linux_errno(status);

	return 0;
}

/**
 * scmi_base_set_protocol_permissions_int - configure access permission to
 *					    protocol on device
 * @dev:	SCMI device
 * @agent_id:	SCMI agent ID
 * @device_id:	ID of device to access
 * @command_id:	SCMI command ID
 * @flags:	A set of flags
 *
 * Ask for allowing or denying access permission to the protocol, @command_id,
 * on the device, @device_id.
 * The meaning of @flags is defined in SCMI specification.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_base_set_protocol_permissions_int(struct udevice *dev,
						  u32 agent_id, u32 device_id,
						  u32 command_id, u32 flags)
{
	struct scmi_base_set_protocol_permissions_in in = {
		.agent_id = agent_id,
		.device_id = device_id,
		.command_id = command_id,
		.flags = flags,
	};
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_BASE_SET_PROTOCOL_PERMISSIONS,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&status,
		.out_msg_sz = sizeof(status),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (status)
		return scmi_to_linux_errno(status);

	return 0;
}

/**
 * scmi_base_reset_agent_configuration_int - reset resource settings
 * @dev:	SCMI device
 * @agent_id:	SCMI agent ID
 * @flags:	A set of flags
 *
 * Reset all the resource settings against @agent_id.
 * The meaning of @flags is defined in SCMI specification.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_base_reset_agent_configuration_int(struct udevice *dev,
						   u32 agent_id, u32 flags)
{
	struct scmi_base_reset_agent_configuration_in in = {
		.agent_id = agent_id,
		.flags = flags,
	};
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_BASE_RESET_AGENT_CONFIGURATION,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&status,
		.out_msg_sz = sizeof(status),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (status)
		return scmi_to_linux_errno(status);

	return 0;
}

/**
 * scmi_base_probe - probe base protocol device
 * @dev:	SCMI device
 *
 * Probe the device for SCMI base protocol and initialize the private data.
 *
 * Return: 0 on success, error code on failure
 */
static int scmi_base_probe(struct udevice *dev)
{
	u32 version;
	int ret;

	ret = devm_scmi_of_get_channel(dev);
	if (ret) {
		dev_err(dev, "get_channel failed\n");
		return ret;
	}
	ret = scmi_base_protocol_version_int(dev, &version);
	if (ret) {
		dev_err(dev, "getting protocol version failed\n");
		return ret;
	}
	if (version < SCMI_BASE_PROTOCOL_VERSION)
		return -EINVAL;

	return ret;
}

static struct scmi_base_ops scmi_base_ops = {
	/* Commands */
	.protocol_version = scmi_base_protocol_version_int,
	.protocol_attrs = scmi_protocol_attrs_int,
	.protocol_message_attrs = scmi_protocol_message_attrs_int,
	.base_discover_vendor = scmi_base_discover_vendor_int,
	.base_discover_sub_vendor = scmi_base_discover_sub_vendor_int,
	.base_discover_impl_version = scmi_base_discover_impl_version_int,
	.base_discover_list_protocols = scmi_base_discover_list_protocols_int,
	.base_discover_agent = scmi_base_discover_agent_int,
	.base_notify_errors = NULL,
	.base_set_device_permissions = scmi_base_set_device_permissions_int,
	.base_set_protocol_permissions = scmi_base_set_protocol_permissions_int,
	.base_reset_agent_configuration =
			scmi_base_reset_agent_configuration_int,
};

int scmi_base_protocol_version(struct udevice *dev, u32 *version)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->protocol_version)
		return (*ops->protocol_version)(dev, version);

	return -EOPNOTSUPP;
}

int scmi_base_protocol_attrs(struct udevice *dev, u32 *num_agents,
			     u32 *num_protocols)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->protocol_attrs)
		return (*ops->protocol_attrs)(dev, num_agents, num_protocols);

	return -EOPNOTSUPP;
}

int scmi_base_protocol_message_attrs(struct udevice *dev, u32 message_id,
				     u32 *attributes)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->protocol_message_attrs)
		return (*ops->protocol_message_attrs)(dev, message_id,
						      attributes);

	return -EOPNOTSUPP;
}

int scmi_base_discover_vendor(struct udevice *dev, u8 **vendor)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->base_discover_vendor)
		return (*ops->base_discover_vendor)(dev, vendor);

	return -EOPNOTSUPP;
}

int scmi_base_discover_sub_vendor(struct udevice *dev, u8 **sub_vendor)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->base_discover_sub_vendor)
		return (*ops->base_discover_sub_vendor)(dev, sub_vendor);

	return -EOPNOTSUPP;
}

int scmi_base_discover_impl_version(struct udevice *dev, u32 *impl_version)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->base_discover_impl_version)
		return (*ops->base_discover_impl_version)(dev, impl_version);

	return -EOPNOTSUPP;
}

int scmi_base_discover_list_protocols(struct udevice *dev, u8 **protocols)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->base_discover_list_protocols)
		return (*ops->base_discover_list_protocols)(dev, protocols);

	return -EOPNOTSUPP;
}

int scmi_base_discover_agent(struct udevice *dev, u32 agent_id,
			     u32 *ret_agent_id, u8 **name)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->base_discover_agent)
		return (*ops->base_discover_agent)(dev, agent_id, ret_agent_id,
						   name);

	return -EOPNOTSUPP;
}

int scmi_base_notify_errors(struct udevice *dev, u32 enable)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->base_notify_errors)
		return (*ops->base_notify_errors)(dev, enable);

	return -EOPNOTSUPP;
}

int scmi_base_set_device_permissions(struct udevice *dev, u32 agent_id,
				     u32 device_id, u32 flags)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->base_set_device_permissions)
		return (*ops->base_set_device_permissions)(dev, agent_id,
							   device_id, flags);

	return -EOPNOTSUPP;
}

int scmi_base_set_protocol_permissions(struct udevice *dev,
				       u32 agent_id, u32 device_id,
				       u32 command_id, u32 flags)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->base_set_protocol_permissions)
		return (*ops->base_set_protocol_permissions)(dev, agent_id,
							     device_id,
							     command_id,
							     flags);

	return -EOPNOTSUPP;
}

int scmi_base_reset_agent_configuration(struct udevice *dev, u32 agent_id,
					u32 flags)
{
	const struct scmi_base_ops *ops = device_get_ops(dev);

	if (ops->base_reset_agent_configuration)
		return (*ops->base_reset_agent_configuration)(dev, agent_id,
							      flags);

	return -EOPNOTSUPP;
}

U_BOOT_DRIVER(scmi_base_drv) = {
	.id = UCLASS_SCMI_BASE,
	.name = "scmi_base_drv",
	.ops = &scmi_base_ops,
	.probe = scmi_base_probe,
};

UCLASS_DRIVER(scmi_base) = {
	.id		= UCLASS_SCMI_BASE,
	.name		= "scmi_base",
};
