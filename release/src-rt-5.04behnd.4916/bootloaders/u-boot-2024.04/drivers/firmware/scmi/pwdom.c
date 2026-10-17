// SPDX-License-Identifier: GPL-2.0+
/*
 * SCMI Power domain management protocol
 *
 * Copyright (C) 2023 Linaro Limited
 *		author: AKASHI Takahiro <takahiro.akashi@linaro.org>
 */

#include <dm.h>
#include <malloc.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <string.h>
#include <asm/types.h>

int scmi_pwd_protocol_attrs(struct udevice *dev, int *num_pwdoms,
			    u64 *stats_addr, size_t *stats_len)
{
	struct scmi_pwd_protocol_attrs_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_POWER_DOMAIN,
		.message_id = SCMI_PROTOCOL_ATTRIBUTES,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	if (!dev || !num_pwdoms || !stats_addr || !stats_len)
		return -EINVAL;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*num_pwdoms = SCMI_PWD_PROTO_ATTRS_NUM_PWD(out.attributes);
	*stats_addr = ((u64)out.stats_addr_high << 32) + out.stats_addr_low;
	*stats_len = out.stats_len;

	return 0;
}

int scmi_pwd_protocol_message_attrs(struct udevice *dev, s32 message_id,
				    u32 *attributes)
{
	struct scmi_pwd_protocol_msg_attrs_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_POWER_DOMAIN,
		.message_id = SCMI_PROTOCOL_MESSAGE_ATTRIBUTES,
		.in_msg = (u8 *)&message_id,
		.in_msg_sz = sizeof(message_id),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	if (!dev || !attributes)
		return -EINVAL;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*attributes = out.attributes;

	return 0;
}

int scmi_pwd_attrs(struct udevice *dev, u32 domain_id, u32 *attributes,
		   u8 **name)
{
	struct scmi_pwd_attrs_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_POWER_DOMAIN,
		.message_id = SCMI_PWD_ATTRIBUTES,
		.in_msg = (u8 *)&domain_id,
		.in_msg_sz = sizeof(domain_id),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	if (!dev || !attributes || !name)
		return -EINVAL;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*name = strdup(out.name);
	if (!*name)
		return -ENOMEM;

	*attributes = out.attributes;

	return 0;
}

int scmi_pwd_state_set(struct udevice *dev, u32 flags, u32 domain_id,
		       u32 pstate)
{
	struct scmi_pwd_state_set_in in;
	s32 status;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_POWER_DOMAIN,
		.message_id = SCMI_PWD_STATE_SET,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&status,
		.out_msg_sz = sizeof(status),
	};
	int ret;

	if (!dev)
		return -EINVAL;

	in.flags = flags;
	in.domain_id = domain_id;
	in.pstate = pstate;
	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (status)
		return scmi_to_linux_errno(status);

	return 0;
}

int scmi_pwd_state_get(struct udevice *dev, u32 domain_id, u32 *pstate)
{
	struct scmi_pwd_state_get_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_POWER_DOMAIN,
		.message_id = SCMI_PWD_STATE_GET,
		.in_msg = (u8 *)&domain_id,
		.in_msg_sz = sizeof(domain_id),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	if (!dev || !pstate)
		return -EINVAL;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*pstate = out.pstate;

	return 0;
}

int scmi_pwd_name_get(struct udevice *dev, u32 domain_id, u8 **name)
{
	struct scmi_pwd_name_get_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_POWER_DOMAIN,
		.message_id = SCMI_PWD_NAME_GET,
		.in_msg = (u8 *)&domain_id,
		.in_msg_sz = sizeof(domain_id),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	if (!dev || !name)
		return -EINVAL;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;
	if (out.status)
		return scmi_to_linux_errno(out.status);

	*name = strdup(out.extended_name);
	if (!*name)
		return -ENOMEM;

	return 0;
}
