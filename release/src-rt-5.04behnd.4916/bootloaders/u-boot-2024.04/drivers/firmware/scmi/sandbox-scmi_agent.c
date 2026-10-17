// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Linaro Limited
 */

#define LOG_CATEGORY UCLASS_SCMI_AGENT

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <scmi_protocols.h>
#include <asm/io.h>
#include <asm/scmi_test.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/kernel.h>

/*
 * The sandbox SCMI agent driver simulates to some extend a SCMI message
 * processing. It simulates few of the SCMI services for some of the
 * SCMI protocols embedded in U-Boot. Currently:
 * - SCMI base protocol
 * - SCMI clock protocol emulates an agent exposing 2 clocks
 * - SCMI reset protocol emulates an agent exposing a reset controller
 * - SCMI voltage domain protocol emulates an agent exposing 2 regulators
 *
 * As per DT bindings, the device node name shall be scmi.
 *
 * All clocks and regulators are default disabled and reset controller down.
 *
 * This driver exports sandbox_scmi_service_ctx() for the test sequence to
 * get the state of the simulated services (clock state, rate, ...) and
 * check back-end device state reflects the request send through the
 * various uclass devices, as clocks and reset controllers.
 */

#define SANDBOX_SCMI_BASE_PROTOCOL_VERSION SCMI_BASE_PROTOCOL_VERSION
#define SANDBOX_SCMI_VENDOR "U-Boot"
#define SANDBOX_SCMI_SUB_VENDOR "Sandbox"
#define SANDBOX_SCMI_IMPL_VERSION 0x1
#define SANDBOX_SCMI_AGENT_NAME "OSPM"
#define SANDBOX_SCMI_PLATFORM_NAME "platform"

#define SANDBOX_SCMI_PWD_PROTOCOL_VERSION SCMI_PWD_PROTOCOL_VERSION

/**
 * struct sandbox_channel - Description of sandbox transport
 * @channel_id:		Channel identifier
 *
 * Dummy channel. This will be used to test if a protocol-specific
 * channel is properly used.
 * Id 0 means a channel for the sandbox agent.
 */
struct sandbox_channel {
	unsigned int channel_id;
};

/**
 * struct scmi_channel - Channel instance referenced in SCMI drivers
 * @ref: Reference to local channel instance
 **/
struct scmi_channel {
	struct sandbox_channel ref;
};

static u8 protocols[] = {
	CONFIG_IS_ENABLED(SCMI_POWER_DOMAIN, (SCMI_PROTOCOL_ID_POWER_DOMAIN,))
	CONFIG_IS_ENABLED(CLK_SCMI, (SCMI_PROTOCOL_ID_CLOCK,))
	CONFIG_IS_ENABLED(RESET_SCMI, (SCMI_PROTOCOL_ID_RESET_DOMAIN,))
	CONFIG_IS_ENABLED(DM_REGULATOR_SCMI, (SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN,))
};

#define NUM_PROTOCOLS ARRAY_SIZE(protocols)

static struct sandbox_scmi_pwd scmi_pwdom[] = {
	{ .id = 0 },
	{ .id = 1 },
	{ .id = 2 },
};

static struct sandbox_scmi_clk scmi_clk[] = {
	{ .rate = 333 },
	{ .rate = 200 },
	{ .rate = 1000 },
};

static struct sandbox_scmi_reset scmi_reset[] = {
	{ .id = 3 },
};

static struct sandbox_scmi_voltd scmi_voltd[] = {
	{ .id = 0, .voltage_uv = 3300000 },
	{ .id = 1, .voltage_uv = 1800000 },
};

struct sandbox_scmi_agent *sandbox_scmi_agent_ctx(struct udevice *dev)
{
	return dev_get_priv(dev);
}

static void debug_print_agent_state(struct udevice *dev, char *str)
{
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);

	dev_dbg(dev, "Dump sandbox_scmi_agent: %s\n", str);
	dev_dbg(dev, " scmi_clk   (%zu): %d/%ld, %d/%ld, %d/%ld, ...\n",
		agent->clk_count,
		agent->clk_count ? agent->clk[0].enabled : -1,
		agent->clk_count ? agent->clk[0].rate : -1,
		agent->clk_count > 1 ? agent->clk[1].enabled : -1,
		agent->clk_count > 1 ? agent->clk[1].rate : -1,
		agent->clk_count > 2 ? agent->clk[2].enabled : -1,
		agent->clk_count > 2 ? agent->clk[2].rate : -1);
	dev_dbg(dev, " scmi_reset (%zu): %d, %d, ...\n",
		agent->reset_count,
		agent->reset_count ? agent->reset[0].asserted : -1,
		agent->reset_count > 1 ? agent->reset[1].asserted : -1);
	dev_dbg(dev, " scmi_voltd (%zu): %u/%d, %u/%d, ...\n",
		agent->voltd_count,
		agent->voltd_count ? agent->voltd[0].enabled : -1,
		agent->voltd_count ? agent->voltd[0].voltage_uv : -1,
		agent->voltd_count ? agent->voltd[1].enabled : -1,
		agent->voltd_count ? agent->voltd[1].voltage_uv : -1);
};

static struct sandbox_scmi_clk *get_scmi_clk_state(uint clock_id)
{
	if (clock_id < ARRAY_SIZE(scmi_clk))
		return scmi_clk + clock_id;

	return NULL;
}

static struct sandbox_scmi_reset *get_scmi_reset_state(uint reset_id)
{
	size_t n;

	for (n = 0; n < ARRAY_SIZE(scmi_reset); n++)
		if (scmi_reset[n].id == reset_id)
			return scmi_reset + n;

	return NULL;
}

static struct sandbox_scmi_voltd *get_scmi_voltd_state(uint domain_id)
{
	size_t n;

	for (n = 0; n < ARRAY_SIZE(scmi_voltd); n++)
		if (scmi_voltd[n].id == domain_id)
			return scmi_voltd + n;

	return NULL;
}

/*
 * Sandbox SCMI agent ops
 */

/* Base Protocol */

/**
 * sandbox_scmi_base_protocol_version - implement SCMI_BASE_PROTOCOL_VERSION
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_PROTOCOL_VERSION command.
 */
static int sandbox_scmi_base_protocol_version(struct udevice *dev,
					      struct scmi_msg *msg)
{
	struct scmi_protocol_version_out *out = NULL;

	if (!msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	out = (struct scmi_protocol_version_out *)msg->out_msg;
	out->version = SANDBOX_SCMI_BASE_PROTOCOL_VERSION;
	out->status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_base_protocol_attrs - implement SCMI_BASE_PROTOCOL_ATTRIBUTES
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_PROTOCOL_ATTRIBUTES command.
 */
static int sandbox_scmi_base_protocol_attrs(struct udevice *dev,
					    struct scmi_msg *msg)
{
	struct scmi_protocol_attrs_out *out = NULL;

	if (!msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	out = (struct scmi_protocol_attrs_out *)msg->out_msg;
	out->attributes = FIELD_PREP(0xff00, 2) | NUM_PROTOCOLS;
	out->status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_base_message_attrs - implement
 *					SCMI_BASE_PROTOCOL_MESSAGE_ATTRIBUTES
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_PROTOCOL_MESSAGE_ATTRIBUTES command.
 */
static int sandbox_scmi_base_message_attrs(struct udevice *dev,
					   struct scmi_msg *msg)
{
	u32 message_id;
	struct scmi_protocol_msg_attrs_out *out = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(message_id) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	message_id = *(u32 *)msg->in_msg;
	out = (struct scmi_protocol_msg_attrs_out *)msg->out_msg;

	if (message_id >= SCMI_PROTOCOL_VERSION &&
	    message_id <= SCMI_BASE_RESET_AGENT_CONFIGURATION &&
	    message_id != SCMI_BASE_NOTIFY_ERRORS) {
		out->attributes = 0;
		out->status = SCMI_SUCCESS;
	} else {
		out->status = SCMI_NOT_FOUND;
	}

		return 0;
}

/**
 * sandbox_scmi_base_discover_vendor - implement SCMI_BASE_DISCOVER_VENDOR
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_DISCOVER_VENDOR command
 */
static int sandbox_scmi_base_discover_vendor(struct udevice *dev,
					     struct scmi_msg *msg)
{
	struct scmi_base_discover_vendor_out *out = NULL;

	if (!msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	out = (struct scmi_base_discover_vendor_out *)msg->out_msg;
	strcpy(out->vendor_identifier, SANDBOX_SCMI_VENDOR);
	out->status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_base_discover_sub_vendor - implement
 *						SCMI_BASE_DISCOVER_SUB_VENDOR
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_DISCOVER_SUB_VENDOR command
 */
static int sandbox_scmi_base_discover_sub_vendor(struct udevice *dev,
						 struct scmi_msg *msg)
{
	struct scmi_base_discover_vendor_out *out = NULL;

	if (!msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	out = (struct scmi_base_discover_vendor_out *)msg->out_msg;
	strcpy(out->vendor_identifier, SANDBOX_SCMI_SUB_VENDOR);
	out->status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_base_discover_impl_version - implement
 *				SCMI_BASE_DISCOVER_IMPLEMENTATION_VERSION
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_DISCOVER_IMPLEMENTATION_VERSION command
 */
static int sandbox_scmi_base_discover_impl_version(struct udevice *dev,
						   struct scmi_msg *msg)
{
	struct scmi_base_discover_impl_version_out *out = NULL;

	if (!msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	out = (struct scmi_base_discover_impl_version_out *)msg->out_msg;
	out->impl_version = SANDBOX_SCMI_IMPL_VERSION;
	out->status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_base_discover_list_protocols - implement
 *					SCMI_BASE_DISCOVER_LIST_PROTOCOLS
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_DISCOVER_LIST_PROTOCOLS command
 */
static int sandbox_scmi_base_discover_list_protocols(struct udevice *dev,
						     struct scmi_msg *msg)
{
	struct scmi_base_discover_list_protocols_out *out = NULL;

	if (!msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	out = (struct scmi_base_discover_list_protocols_out *)msg->out_msg;
	memcpy(out->protocols, protocols, sizeof(protocols));
	out->num_protocols = NUM_PROTOCOLS;
	out->status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_base_discover_agent - implement SCMI_BASE_DISCOVER_AGENT
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_DISCOVER_AGENT command
 */
static int sandbox_scmi_base_discover_agent(struct udevice *dev,
					    struct scmi_msg *msg)
{
	u32 agent_id;
	struct scmi_base_discover_agent_out *out = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(agent_id) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	agent_id = *(u32 *)msg->in_msg;
	out = (struct scmi_base_discover_agent_out *)msg->out_msg;
	out->status = SCMI_SUCCESS;
	if (agent_id == 0xffffffff || agent_id == 1) {
		out->agent_id = 1;
		strcpy(out->name, SANDBOX_SCMI_AGENT_NAME);
	} else if (!agent_id) {
		out->agent_id = agent_id;
		strcpy(out->name, SANDBOX_SCMI_PLATFORM_NAME);
	} else {
		out->status = SCMI_NOT_FOUND;
	}

	return 0;
}

/**
 * sandbox_scmi_base_set_device_permissions - implement
 *						SCMI_BASE_SET_DEVICE_PERMISSIONS
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_SET_DEVICE_PERMISSIONS command
 */
static int sandbox_scmi_base_set_device_permissions(struct udevice *dev,
						    struct scmi_msg *msg)
{
	struct scmi_base_set_device_permissions_in *in = NULL;
	u32 *status;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*status))
		return -EINVAL;

	in = (struct scmi_base_set_device_permissions_in *)msg->in_msg;
	status = (u32 *)msg->out_msg;

	if (in->agent_id != 1 || in->device_id != 0)
		*status = SCMI_NOT_FOUND;
	else if (in->flags & ~SCMI_BASE_SET_DEVICE_PERMISSIONS_ACCESS)
		*status = SCMI_INVALID_PARAMETERS;
	else if (in->flags & SCMI_BASE_SET_DEVICE_PERMISSIONS_ACCESS)
		*status = SCMI_SUCCESS;
	else
		/* unset not allowed */
		*status = SCMI_DENIED;

	return 0;
}

/**
 * sandbox_scmi_base_set_protocol_permissions - implement
 *					SCMI_BASE_SET_PROTOCOL_PERMISSIONS
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_SET_PROTOCOL_PERMISSIONS command
 */
static int sandbox_scmi_base_set_protocol_permissions(struct udevice *dev,
						      struct scmi_msg *msg)
{
	struct scmi_base_set_protocol_permissions_in *in = NULL;
	u32 *status;
	int i;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*status))
		return -EINVAL;

	in = (struct scmi_base_set_protocol_permissions_in *)msg->in_msg;
	status = (u32 *)msg->out_msg;

	for (i = 0; i < ARRAY_SIZE(protocols); i++)
		if (protocols[i] == in->command_id)
			break;
	if (in->agent_id != 1 || in->device_id != 0 ||
	    i == ARRAY_SIZE(protocols))
		*status = SCMI_NOT_FOUND;
	else if (in->flags & ~SCMI_BASE_SET_PROTOCOL_PERMISSIONS_ACCESS)
		*status = SCMI_INVALID_PARAMETERS;
	else if (in->flags & SCMI_BASE_SET_PROTOCOL_PERMISSIONS_ACCESS)
		*status = SCMI_SUCCESS;
	else
		/* unset not allowed */
		*status = SCMI_DENIED;

	return 0;
}

/**
 * sandbox_scmi_base_reset_agent_configuration - implement
 *					SCMI_BASE_RESET_AGENT_CONFIGURATION
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_BASE_RESET_AGENT_CONFIGURATION command
 */
static int sandbox_scmi_base_reset_agent_configuration(struct udevice *dev,
						       struct scmi_msg *msg)
{
	struct scmi_base_reset_agent_configuration_in *in = NULL;
	u32 *status;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*status))
		return -EINVAL;

	in = (struct scmi_base_reset_agent_configuration_in *)msg->in_msg;
	status = (u32 *)msg->out_msg;

	if (in->agent_id != 1)
		*status = SCMI_NOT_FOUND;
	else if (in->flags & ~SCMI_BASE_RESET_ALL_ACCESS_PERMISSIONS)
		*status = SCMI_INVALID_PARAMETERS;
	else
		*status = SCMI_DENIED;

	return 0;
}

/* Power Domain Management Protocol */

/**
 * sandbox_scmi_pwd_protocol_version - implement SCMI_PROTOCOL_VERSION
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_PROTOCOL_VERSION command.
 *
 * Return:	0 on success, error code on failure
 */
static int sandbox_scmi_pwd_protocol_version(struct udevice *dev,
					     struct scmi_msg *msg)
{
	struct scmi_protocol_version_out *out = NULL;

	if (!msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	out = (struct scmi_protocol_version_out *)msg->out_msg;
	out->version = SANDBOX_SCMI_PWD_PROTOCOL_VERSION;
	out->status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_pwd_protocol_attribs - implement SCMI_PWD_PROTOCOL_ATTRS
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_PWD_PROTOCOL_ATTRS command.
 *
 * Return:	0 on success, error code on failure
 */
static int sandbox_scmi_pwd_protocol_attribs(struct udevice *dev,
					     struct scmi_msg *msg)
{
	struct scmi_pwd_protocol_attrs_out *out;

	if (!msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	out = (struct scmi_pwd_protocol_attrs_out *)msg->out_msg;

	out->attributes = ARRAY_SIZE(scmi_pwdom);
	out->stats_addr_low = 0;
	out->stats_addr_high = 0;
	out->stats_len = 0;
	out->status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_pwd_protocol_msg_attribs - implement
					SCMI_PWD_PROTOCOL_MESSAGE_ATTRS
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_PWD_PROTOCOL_MESSAGE_ATTRS command.
 *
 * Return:	0 on success, error code on failure
 */
static int sandbox_scmi_pwd_protocol_msg_attribs(struct udevice *dev,
						 struct scmi_msg *msg)
{
	u32 message_id;
	struct scmi_pwd_protocol_msg_attrs_out *out;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(message_id) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	message_id = *(u32 *)msg->in_msg;

	out = (struct scmi_pwd_protocol_msg_attrs_out *)msg->out_msg;
	if (message_id <= SCMI_PWD_STATE_GET ||
	    message_id == SCMI_PWD_NAME_GET) {
		out->attributes = 0;
		out->status = SCMI_SUCCESS;
	} else {
		out->status = SCMI_NOT_FOUND;
	}

	return 0;
}

/**
 * sandbox_scmi_pwd_attribs - implement SCMI_PWD_ATTRS
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_PWD_ATTRS command.
 *
 * Return:	0 on success, error code on failure
 */
static int sandbox_scmi_pwd_attribs(struct udevice *dev, struct scmi_msg *msg)
{
	u32 domain_id;
	struct scmi_pwd_attrs_out *out;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(domain_id) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	domain_id = *(u32 *)msg->in_msg;
	out = (struct scmi_pwd_attrs_out *)msg->out_msg;

	if (domain_id >= ARRAY_SIZE(scmi_pwdom)) {
		out->status = SCMI_NOT_FOUND;

		return 0;
	}

	out->attributes =
		SCMI_PWD_ATTR_PSTATE_SYNC | SCMI_PWD_ATTR_EXTENDED_NAME;
	/* just 15-char + NULL */
	snprintf(out->name, SCMI_PWD_NAME_LENGTH_MAX, "power-domain--%d",
		 domain_id);
	out->status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_pwd_state_set - implement SCMI_PWD_STATE_SET
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_PWD_STATE_SET command.
 *
 * Return:	0 on success, error code on failure
 */
static int sandbox_scmi_pwd_state_set(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_pwd_state_set_in *in;
	s32 *status;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*status))
		return -EINVAL;

	in = (struct scmi_pwd_state_set_in *)msg->in_msg;
	status = (s32 *)msg->out_msg;

	if (in->domain_id >= ARRAY_SIZE(scmi_pwdom)) {
		*status = SCMI_NOT_FOUND;

		return 0;
	}

	if ((in->flags & SCMI_PWD_SET_FLAGS_ASYNC) ||
	    (in->pstate != SCMI_PWD_PSTATE_TYPE_LOST && in->pstate)) {
		*status = SCMI_INVALID_PARAMETERS;

		return 0;
	}

	scmi_pwdom[in->domain_id].pstate = in->pstate;
	*status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_pwd_state_get - implement SCMI_PWD_STATE_GET
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_PWD_STATE_GET command.
 *
 * Return:	0 on success, error code on failure
 */
static int sandbox_scmi_pwd_state_get(struct udevice *dev, struct scmi_msg *msg)
{
	u32 domain_id;
	struct scmi_pwd_state_get_out *out;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(domain_id) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	domain_id = *(u32 *)msg->in_msg;
	out = (struct scmi_pwd_state_get_out *)msg->out_msg;

	if (domain_id >= ARRAY_SIZE(scmi_pwdom)) {
		out->status = SCMI_NOT_FOUND;

		return 0;
	}

	out->pstate = scmi_pwdom[domain_id].pstate;
	out->status = SCMI_SUCCESS;

	return 0;
}

/**
 * sandbox_scmi_pwd_name_get - implement SCMI_PWD_NAME_GET
 * @dev:	SCMI device
 * @msg:	SCMI message
 *
 * Implement SCMI_PWD_NAME_GET command.
 *
 * Return:	0 on success, error code on failure
 */
static int sandbox_scmi_pwd_name_get(struct udevice *dev, struct scmi_msg *msg)
{
	u32 domain_id;
	struct scmi_pwd_name_get_out *out;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(domain_id) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	domain_id = *(u32 *)msg->in_msg;
	out = (struct scmi_pwd_name_get_out *)msg->out_msg;

	if (domain_id >= ARRAY_SIZE(scmi_pwdom)) {
		out->status = SCMI_NOT_FOUND;

		return 0;
	}

	snprintf(out->extended_name, SCMI_PWD_EXTENDED_NAME_MAX,
		 "power-domain--%d-extended", domain_id);
	out->status = SCMI_SUCCESS;

	return 0;
}

/* Clock Protocol */

static int sandbox_scmi_clock_protocol_attribs(struct udevice *dev,
					       struct scmi_msg *msg)
{
	struct scmi_clk_protocol_attr_out *out = NULL;

	if (!msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	out = (struct scmi_clk_protocol_attr_out *)msg->out_msg;
	out->attributes = ARRAY_SIZE(scmi_clk);
	out->status = SCMI_SUCCESS;

	return 0;
}

static int sandbox_scmi_clock_attribs(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_clk_attribute_in *in = NULL;
	struct scmi_clk_attribute_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;
	int ret;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_attribute_in *)msg->in_msg;
	out = (struct scmi_clk_attribute_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(in->clock_id);
	if (!clk_state) {
		dev_err(dev, "Unexpected clock ID %u\n", in->clock_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		memset(out, 0, sizeof(*out));

		if (clk_state->enabled)
			out->attributes = 1;

		ret = snprintf(out->clock_name, sizeof(out->clock_name),
			       "clk%u", in->clock_id);
		assert(ret > 0 && ret < sizeof(out->clock_name));

		out->status = SCMI_SUCCESS;
	}

	return 0;
}
static int sandbox_scmi_clock_rate_set(struct udevice *dev,
				       struct scmi_msg *msg)
{
	struct scmi_clk_rate_set_in *in = NULL;
	struct scmi_clk_rate_set_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_rate_set_in *)msg->in_msg;
	out = (struct scmi_clk_rate_set_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(in->clock_id);
	if (!clk_state) {
		dev_err(dev, "Unexpected clock ID %u\n", in->clock_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		u64 rate = ((u64)in->rate_msb << 32) + in->rate_lsb;

		clk_state->rate = (ulong)rate;

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_clock_rate_get(struct udevice *dev,
				       struct scmi_msg *msg)
{
	struct scmi_clk_rate_get_in *in = NULL;
	struct scmi_clk_rate_get_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_rate_get_in *)msg->in_msg;
	out = (struct scmi_clk_rate_get_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(in->clock_id);
	if (!clk_state) {
		dev_err(dev, "Unexpected clock ID %u\n", in->clock_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		out->rate_msb = (u32)((u64)clk_state->rate >> 32);
		out->rate_lsb = (u32)clk_state->rate;

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_clock_gate(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_clk_state_in *in = NULL;
	struct scmi_clk_state_out *out = NULL;
	struct sandbox_scmi_clk *clk_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_clk_state_in *)msg->in_msg;
	out = (struct scmi_clk_state_out *)msg->out_msg;

	clk_state = get_scmi_clk_state(in->clock_id);
	if (!clk_state) {
		dev_err(dev, "Unexpected clock ID %u\n", in->clock_id);

		out->status = SCMI_NOT_FOUND;
	} else if (in->attributes > 1) {
		out->status = SCMI_PROTOCOL_ERROR;
	} else {
		clk_state->enabled = in->attributes;

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_rd_attribs(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_rd_attr_in *in = NULL;
	struct scmi_rd_attr_out *out = NULL;
	struct sandbox_scmi_reset *reset_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_rd_attr_in *)msg->in_msg;
	out = (struct scmi_rd_attr_out *)msg->out_msg;

	reset_state = get_scmi_reset_state(in->domain_id);
	if (!reset_state) {
		dev_err(dev, "Unexpected reset domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		memset(out, 0, sizeof(*out));
		snprintf(out->name, sizeof(out->name), "rd%u", in->domain_id);

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_rd_reset(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_rd_reset_in *in = NULL;
	struct scmi_rd_reset_out *out = NULL;
	struct sandbox_scmi_reset *reset_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_rd_reset_in *)msg->in_msg;
	out = (struct scmi_rd_reset_out *)msg->out_msg;

	reset_state = get_scmi_reset_state(in->domain_id);
	if (!reset_state) {
		dev_err(dev, "Unexpected reset domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else if (in->reset_state > 1) {
		dev_err(dev, "Invalid reset domain input attribute value\n");

		out->status = SCMI_INVALID_PARAMETERS;
	} else {
		if (in->flags & SCMI_RD_RESET_FLAG_CYCLE) {
			if (in->flags & SCMI_RD_RESET_FLAG_ASYNC) {
				out->status = SCMI_NOT_SUPPORTED;
			} else {
				/* Ends deasserted whatever current state */
				reset_state->asserted = false;
				out->status = SCMI_SUCCESS;
			}
		} else {
			reset_state->asserted = in->flags &
						SCMI_RD_RESET_FLAG_ASSERT;

			out->status = SCMI_SUCCESS;
		}
	}

	return 0;
}

static int sandbox_scmi_voltd_attribs(struct udevice *dev, struct scmi_msg *msg)
{
	struct scmi_voltd_attr_in *in = NULL;
	struct scmi_voltd_attr_out *out = NULL;
	struct sandbox_scmi_voltd *voltd_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_voltd_attr_in *)msg->in_msg;
	out = (struct scmi_voltd_attr_out *)msg->out_msg;

	voltd_state = get_scmi_voltd_state(in->domain_id);
	if (!voltd_state) {
		dev_err(dev, "Unexpected domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		memset(out, 0, sizeof(*out));
		snprintf(out->name, sizeof(out->name), "regu%u", in->domain_id);

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_voltd_config_set(struct udevice *dev,
					 struct scmi_msg *msg)
{
	struct scmi_voltd_config_set_in *in = NULL;
	struct scmi_voltd_config_set_out *out = NULL;
	struct sandbox_scmi_voltd *voltd_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_voltd_config_set_in *)msg->in_msg;
	out = (struct scmi_voltd_config_set_out *)msg->out_msg;

	voltd_state = get_scmi_voltd_state(in->domain_id);
	if (!voltd_state) {
		dev_err(dev, "Unexpected domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else if (in->config & ~SCMI_VOLTD_CONFIG_MASK) {
		dev_err(dev, "Invalid config value 0x%x\n", in->config);

		out->status = SCMI_INVALID_PARAMETERS;
	} else if (in->config != SCMI_VOLTD_CONFIG_ON &&
		   in->config != SCMI_VOLTD_CONFIG_OFF) {
		dev_err(dev, "Unexpected custom value 0x%x\n", in->config);

		out->status = SCMI_INVALID_PARAMETERS;
	} else {
		voltd_state->enabled = in->config == SCMI_VOLTD_CONFIG_ON;
		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_voltd_config_get(struct udevice *dev,
					 struct scmi_msg *msg)
{
	struct scmi_voltd_config_get_in *in = NULL;
	struct scmi_voltd_config_get_out *out = NULL;
	struct sandbox_scmi_voltd *voltd_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_voltd_config_get_in *)msg->in_msg;
	out = (struct scmi_voltd_config_get_out *)msg->out_msg;

	voltd_state = get_scmi_voltd_state(in->domain_id);
	if (!voltd_state) {
		dev_err(dev, "Unexpected domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		if (voltd_state->enabled)
			out->config = SCMI_VOLTD_CONFIG_ON;
		else
			out->config = SCMI_VOLTD_CONFIG_OFF;

		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_voltd_level_set(struct udevice *dev,
					 struct scmi_msg *msg)
{
	struct scmi_voltd_level_set_in *in = NULL;
	struct scmi_voltd_level_set_out *out = NULL;
	struct sandbox_scmi_voltd *voltd_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_voltd_level_set_in *)msg->in_msg;
	out = (struct scmi_voltd_level_set_out *)msg->out_msg;

	voltd_state = get_scmi_voltd_state(in->domain_id);
	if (!voltd_state) {
		dev_err(dev, "Unexpected domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		voltd_state->voltage_uv = in->voltage_level;
		out->status = SCMI_SUCCESS;
	}

	return 0;
}

static int sandbox_scmi_voltd_level_get(struct udevice *dev,
					struct scmi_msg *msg)
{
	struct scmi_voltd_level_get_in *in = NULL;
	struct scmi_voltd_level_get_out *out = NULL;
	struct sandbox_scmi_voltd *voltd_state = NULL;

	if (!msg->in_msg || msg->in_msg_sz < sizeof(*in) ||
	    !msg->out_msg || msg->out_msg_sz < sizeof(*out))
		return -EINVAL;

	in = (struct scmi_voltd_level_get_in *)msg->in_msg;
	out = (struct scmi_voltd_level_get_out *)msg->out_msg;

	voltd_state = get_scmi_voltd_state(in->domain_id);
	if (!voltd_state) {
		dev_err(dev, "Unexpected domain ID %u\n", in->domain_id);

		out->status = SCMI_NOT_FOUND;
	} else {
		out->voltage_level = voltd_state->voltage_uv;
		out->status = SCMI_SUCCESS;
	}

	return 0;
}

/**
 * sandbox_scmi_of_get_channel - assigne a channel
 * @dev:	SCMI agent device
 * @protocol:	SCMI protocol device
 * @channel:	Pointer to channel info
 *
 * Assign a channel for the protocol, @protocol, in @channel,
 * based on a device tree's property.
 *
 * Return: 0 on success, error code on failure
 */
static int sandbox_scmi_of_get_channel(struct udevice *dev,
				       struct udevice *protocol,
				       struct scmi_channel **channel)
{
	struct sandbox_channel *agent_chan = dev_get_plat(dev);
	struct sandbox_channel *chan;
	u32 channel_id;

	if (dev_read_u32(protocol, "linaro,sandbox-channel-id", &channel_id)) {
		/* Uses agent channel */
		*channel = container_of(agent_chan, struct scmi_channel, ref);

		return 0;
	}

	/* Setup a dedicated channel */
	chan = calloc(1, sizeof(*chan));
	if (!chan)
		return -ENOMEM;

	chan->channel_id = channel_id;

	*channel = container_of(chan, struct scmi_channel, ref);

	return 0;
}

/**
 * sandbox_scmi_of_to_plat - assigne a channel to agent
 * @dev:	SCMI agent device
 *
 * Assign a channel for the agent, @protocol.
 *
 * Return: always 0
 */
static int sandbox_scmi_of_to_plat(struct udevice *dev)
{
	struct sandbox_channel *chan = dev_get_plat(dev);

	/* The channel for agent is always 0 */
	chan->channel_id = 0;

	return 0;
}

unsigned int sandbox_scmi_channel_id(struct udevice *dev)
{
	struct scmi_agent_proto_priv *priv;
	struct sandbox_channel *chan;

	priv = dev_get_parent_priv(dev);
	chan = (struct sandbox_channel *)&priv->channel->ref;

	return chan->channel_id;
}

static int sandbox_proto_not_supported(struct scmi_msg *msg)
{
	*(u32 *)msg->out_msg = SCMI_NOT_SUPPORTED;

	return 0;
}

static int sandbox_scmi_test_process_msg(struct udevice *dev,
					 struct scmi_channel *channel,
					 struct scmi_msg *msg)
{
	switch (msg->protocol_id) {
	case SCMI_PROTOCOL_ID_BASE:
		switch (msg->message_id) {
		case SCMI_PROTOCOL_VERSION:
			return sandbox_scmi_base_protocol_version(dev, msg);
		case SCMI_PROTOCOL_ATTRIBUTES:
			return sandbox_scmi_base_protocol_attrs(dev, msg);
		case SCMI_PROTOCOL_MESSAGE_ATTRIBUTES:
			return sandbox_scmi_base_message_attrs(dev, msg);
		case SCMI_BASE_DISCOVER_VENDOR:
			return sandbox_scmi_base_discover_vendor(dev, msg);
		case SCMI_BASE_DISCOVER_SUB_VENDOR:
			return sandbox_scmi_base_discover_sub_vendor(dev, msg);
		case SCMI_BASE_DISCOVER_IMPL_VERSION:
			return sandbox_scmi_base_discover_impl_version(dev, msg);
		case SCMI_BASE_DISCOVER_LIST_PROTOCOLS:
			return sandbox_scmi_base_discover_list_protocols(dev, msg);
		case SCMI_BASE_DISCOVER_AGENT:
			return sandbox_scmi_base_discover_agent(dev, msg);
		case SCMI_BASE_NOTIFY_ERRORS:
			break;
		case SCMI_BASE_SET_DEVICE_PERMISSIONS:
			return sandbox_scmi_base_set_device_permissions(dev, msg);
		case SCMI_BASE_SET_PROTOCOL_PERMISSIONS:
			return sandbox_scmi_base_set_protocol_permissions(dev, msg);
		case SCMI_BASE_RESET_AGENT_CONFIGURATION:
			return sandbox_scmi_base_reset_agent_configuration(dev, msg);
		default:
			break;
		}
		break;
	case SCMI_PROTOCOL_ID_POWER_DOMAIN:
		if (!CONFIG_IS_ENABLED(SCMI_POWER_DOMAIN))
			return sandbox_proto_not_supported(msg);

		switch (msg->message_id) {
		case SCMI_PROTOCOL_VERSION:
			return sandbox_scmi_pwd_protocol_version(dev, msg);
		case SCMI_PROTOCOL_ATTRIBUTES:
			return sandbox_scmi_pwd_protocol_attribs(dev, msg);
		case SCMI_PROTOCOL_MESSAGE_ATTRIBUTES:
			return sandbox_scmi_pwd_protocol_msg_attribs(dev, msg);
		case SCMI_PWD_ATTRIBUTES:
			return sandbox_scmi_pwd_attribs(dev, msg);
		case SCMI_PWD_STATE_SET:
			return sandbox_scmi_pwd_state_set(dev, msg);
		case SCMI_PWD_STATE_GET:
			return sandbox_scmi_pwd_state_get(dev, msg);
		case SCMI_PWD_NAME_GET:
			return sandbox_scmi_pwd_name_get(dev, msg);
		default:
			break;
		}
		break;
	case SCMI_PROTOCOL_ID_CLOCK:
		if (!CONFIG_IS_ENABLED(CLK_SCMI))
			return sandbox_proto_not_supported(msg);

		switch (msg->message_id) {
		case SCMI_PROTOCOL_ATTRIBUTES:
			return sandbox_scmi_clock_protocol_attribs(dev, msg);
		case SCMI_CLOCK_ATTRIBUTES:
			return sandbox_scmi_clock_attribs(dev, msg);
		case SCMI_CLOCK_RATE_SET:
			return sandbox_scmi_clock_rate_set(dev, msg);
		case SCMI_CLOCK_RATE_GET:
			return sandbox_scmi_clock_rate_get(dev, msg);
		case SCMI_CLOCK_CONFIG_SET:
			return sandbox_scmi_clock_gate(dev, msg);
		default:
			break;
		}
		break;
	case SCMI_PROTOCOL_ID_RESET_DOMAIN:
		if (!CONFIG_IS_ENABLED(RESET_SCMI))
			return sandbox_proto_not_supported(msg);

		switch (msg->message_id) {
		case SCMI_RESET_DOMAIN_ATTRIBUTES:
			return sandbox_scmi_rd_attribs(dev, msg);
		case SCMI_RESET_DOMAIN_RESET:
			return sandbox_scmi_rd_reset(dev, msg);
		default:
			break;
		}
		break;
	case SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN:
		if (!CONFIG_IS_ENABLED(DM_REGULATOR_SCMI))
			return sandbox_proto_not_supported(msg);

		switch (msg->message_id) {
		case SCMI_VOLTAGE_DOMAIN_ATTRIBUTES:
			return sandbox_scmi_voltd_attribs(dev, msg);
		case SCMI_VOLTAGE_DOMAIN_CONFIG_SET:
			return sandbox_scmi_voltd_config_set(dev, msg);
		case SCMI_VOLTAGE_DOMAIN_CONFIG_GET:
			return sandbox_scmi_voltd_config_get(dev, msg);
		case SCMI_VOLTAGE_DOMAIN_LEVEL_SET:
			return sandbox_scmi_voltd_level_set(dev, msg);
		case SCMI_VOLTAGE_DOMAIN_LEVEL_GET:
			return sandbox_scmi_voltd_level_get(dev, msg);
		default:
			break;
		}
		break;
	case SCMI_PROTOCOL_ID_SYSTEM:
	case SCMI_PROTOCOL_ID_PERF:
	case SCMI_PROTOCOL_ID_SENSOR:
		return sandbox_proto_not_supported(msg);
	default:
		break;
	}

	dev_err(dev, "%s(%s): Unhandled protocol_id %#x/message_id %#x\n",
		__func__, dev->name, msg->protocol_id, msg->message_id);

	if (msg->out_msg_sz < sizeof(u32))
		return -EINVAL;

	/* Intentionnaly report unhandled IDs through the SCMI return code */
	*(u32 *)msg->out_msg = SCMI_PROTOCOL_ERROR;
	return 0;
}

static int sandbox_scmi_test_remove(struct udevice *dev)
{
	debug_print_agent_state(dev, "removed");

	return 0;
}

static int sandbox_scmi_test_probe(struct udevice *dev)
{
	struct sandbox_scmi_agent *agent = dev_get_priv(dev);

	*agent = (struct sandbox_scmi_agent){
		.pwdom_version = SANDBOX_SCMI_PWD_PROTOCOL_VERSION,
		.pwdom = scmi_pwdom,
		.pwdom_count = ARRAY_SIZE(scmi_pwdom),
		.clk = scmi_clk,
		.clk_count = ARRAY_SIZE(scmi_clk),
		.reset = scmi_reset,
		.reset_count = ARRAY_SIZE(scmi_reset),
		.voltd = scmi_voltd,
		.voltd_count = ARRAY_SIZE(scmi_voltd),
	};

	debug_print_agent_state(dev, "probed");

	return 0;
};

static const struct udevice_id sandbox_scmi_test_ids[] = {
	{ .compatible = "sandbox,scmi-agent" },
	{ }
};

struct scmi_agent_ops sandbox_scmi_test_ops = {
	.of_get_channel = sandbox_scmi_of_get_channel,
	.process_msg = sandbox_scmi_test_process_msg,
};

U_BOOT_DRIVER(sandbox_scmi_agent) = {
	.name = "sandbox-scmi_agent",
	.id = UCLASS_SCMI_AGENT,
	.of_match = sandbox_scmi_test_ids,
	.priv_auto	= sizeof(struct sandbox_scmi_agent),
	.plat_auto	= sizeof(struct sandbox_channel),
	.of_to_plat	= sandbox_scmi_of_to_plat,
	.probe = sandbox_scmi_test_probe,
	.remove = sandbox_scmi_test_remove,
	.ops = &sandbox_scmi_test_ops,
};
