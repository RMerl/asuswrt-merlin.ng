/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (C) 2019-2020, Linaro Limited
 */
#ifndef _SCMI_PROTOCOLS_H
#define _SCMI_PROTOCOLS_H

#include <linux/bitops.h>
#include <asm/types.h>

/*
 * Subset the SCMI protocols definition
 * based on SCMI specification v2.0 (DEN0056B)
 * https://developer.arm.com/docs/den0056/b
 */

enum scmi_std_protocol {
	SCMI_PROTOCOL_ID_BASE = 0x10,
	SCMI_PROTOCOL_ID_POWER_DOMAIN = 0x11,
	SCMI_PROTOCOL_ID_SYSTEM = 0x12,
	SCMI_PROTOCOL_ID_PERF = 0x13,
	SCMI_PROTOCOL_ID_CLOCK = 0x14,
	SCMI_PROTOCOL_ID_SENSOR = 0x15,
	SCMI_PROTOCOL_ID_RESET_DOMAIN = 0x16,
	SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN = 0x17,
};

enum scmi_status_code {
	SCMI_SUCCESS =  0,
	SCMI_NOT_SUPPORTED = -1,
	SCMI_INVALID_PARAMETERS = -2,
	SCMI_DENIED = -3,
	SCMI_NOT_FOUND = -4,
	SCMI_OUT_OF_RANGE = -5,
	SCMI_BUSY = -6,
	SCMI_COMMS_ERROR = -7,
	SCMI_GENERIC_ERROR = -8,
	SCMI_HARDWARE_ERROR = -9,
	SCMI_PROTOCOL_ERROR = -10,
};

/*
 * Generic message IDs
 */
enum scmi_discovery_id {
	SCMI_PROTOCOL_VERSION = 0x0,
	SCMI_PROTOCOL_ATTRIBUTES = 0x1,
	SCMI_PROTOCOL_MESSAGE_ATTRIBUTES = 0x2,
};

/*
 * SCMI Base Protocol
 */
#define SCMI_BASE_PROTOCOL_VERSION 0x20000

enum scmi_base_message_id {
	SCMI_BASE_DISCOVER_VENDOR = 0x3,
	SCMI_BASE_DISCOVER_SUB_VENDOR = 0x4,
	SCMI_BASE_DISCOVER_IMPL_VERSION = 0x5,
	SCMI_BASE_DISCOVER_LIST_PROTOCOLS = 0x6,
	SCMI_BASE_DISCOVER_AGENT = 0x7,
	SCMI_BASE_NOTIFY_ERRORS = 0x8,
	SCMI_BASE_SET_DEVICE_PERMISSIONS = 0x9,
	SCMI_BASE_SET_PROTOCOL_PERMISSIONS = 0xa,
	SCMI_BASE_RESET_AGENT_CONFIGURATION = 0xb,
};

#define SCMI_BASE_NAME_LENGTH_MAX 16

/**
 * struct scmi_protocol_version_out - Response for SCMI_PROTOCOL_VERSION
 *					command
 * @status:	SCMI command status
 * @version:	Protocol version
 */
struct scmi_protocol_version_out {
	s32 status;
	u32 version;
};

/**
 * struct scmi_protocol_attrs_out - Response for SCMI_PROTOCOL_ATTRIBUTES
 *					command
 * @status:	SCMI command status
 * @attributes:	Protocol attributes or implementation details
 */
struct scmi_protocol_attrs_out {
	s32 status;
	u32 attributes;
};

#define SCMI_PROTOCOL_ATTRS_NUM_AGENTS(attributes) \
				(((attributes) & GENMASK(15, 8)) >> 8)
#define SCMI_PROTOCOL_ATTRS_NUM_PROTOCOLS(attributes) \
				((attributes) & GENMASK(7, 0))

/**
 * struct scmi_protocol_msg_attrs_out - Response for
 *					SCMI_PROTOCOL_MESSAGE_ATTRIBUTES command
 * @status:	SCMI command status
 * @attributes:	Message-specific attributes
 */
struct scmi_protocol_msg_attrs_out {
	s32 status;
	u32 attributes;
};

/**
 * struct scmi_base_discover_vendor_out - Response for
 *					  SCMI_BASE_DISCOVER_VENDOR or
 *					  SCMI_BASE_DISCOVER_SUB_VENDOR command
 * @status:		SCMI command status
 * @vendor_identifier:	Name of vendor or sub-vendor in string
 */
struct scmi_base_discover_vendor_out {
	s32 status;
	u8 vendor_identifier[SCMI_BASE_NAME_LENGTH_MAX];
};

/**
 * struct scmi_base_discover_impl_version_out - Response for
 *					SCMI_BASE_DISCOVER_IMPL_VERSION command
 * @status:		SCMI command status
 * @impl_version:	Vendor-specific implementation version
 */
struct scmi_base_discover_impl_version_out {
	s32 status;
	u32 impl_version;
};

/**
 * struct scmi_base_discover_list_protocols_out - Response for
 *				SCMI_BASE_DISCOVER_LIST_PROTOCOLS command
 * @status:		SCMI command status
 * @num_protocols:	Number of SCMI protocols in @protocol
 * @protocols:		Array of packed SCMI protocol ID's
 */
struct scmi_base_discover_list_protocols_out {
	s32 status;
	u32 num_protocols;
	u32 protocols[3];
};

/**
 * struct scmi_base_discover_agent_out - Response for
 *					 SCMI_BASE_DISCOVER_AGENT command
 * @status:	SCMI command status
 * @agent_id:	SCMI agent ID
 * @name:	Name of agent in string
 */
struct scmi_base_discover_agent_out {
	s32 status;
	u32 agent_id;
	u8 name[SCMI_BASE_NAME_LENGTH_MAX];
};

#define SCMI_BASE_NOTIFY_ERRORS_ENABLE BIT(0)

/**
 * struct scmi_base_set_device_permissions_in - Parameters for
 *					SCMI_BASE_SET_DEVICE_PERMISSIONS command
 * @agent_id:	SCMI agent ID
 * @device_id:	device ID
 * @flags:	A set of flags
 */
struct scmi_base_set_device_permissions_in {
	u32 agent_id;
	u32 device_id;
	u32 flags;
};

#define SCMI_BASE_SET_DEVICE_PERMISSIONS_ACCESS BIT(0)

/**
 * struct scmi_base_set_protocol_permissions_in - Parameters for
 *				SCMI_BASE_SET_PROTOCOL_PERMISSIONS command
 * @agent_id:		SCMI agent ID
 * @device_id:		device ID
 * @command_id:		command ID
 * @flags:		A set of flags
 */
struct scmi_base_set_protocol_permissions_in {
	u32 agent_id;
	u32 device_id;
	u32 command_id;
	u32 flags;
};

#define SCMI_BASE_SET_PROTOCOL_PERMISSIONS_COMMAND GENMASK(7, 0)
#define SCMI_BASE_SET_PROTOCOL_PERMISSIONS_ACCESS BIT(0)

/**
 * struct scmi_base_reset_agent_configuration_in - Parameters for
 *				SCMI_BASE_RESET_AGENT_CONFIGURATION command
 * @agent_id:	SCMI agent ID
 * @flags:	A set of flags
 */
struct scmi_base_reset_agent_configuration_in {
	u32 agent_id;
	u32 flags;
};

#define SCMI_BASE_RESET_ALL_ACCESS_PERMISSIONS BIT(0)

/**
 * struct scmi_base_ops - SCMI base protocol interfaces
 */
struct scmi_base_ops {
	/**
	 * protocol_version - get Base protocol version
	 * @dev:	SCMI protocol device
	 * @version:	Pointer to SCMI protocol version
	 *
	 * Obtain the protocol version number in @version for Base protocol.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*protocol_version)(struct udevice *dev, u32 *version);
	/**
	 * protocol_attrs - get protocol attributes
	 * @dev:		SCMI protocol device
	 * @num_agents:		Number of SCMI agents
	 * @num_protocols:	Number of SCMI protocols
	 *
	 * Obtain the protocol attributes, the number of agents and the number
	 * of protocols, in @num_agents and @num_protocols respectively, that
	 * the device provides.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*protocol_attrs)(struct udevice *dev, u32 *num_agents,
			      u32 *num_protocols);
	/**
	 * protocol_message_attrs - get message-specific attributes
	 * @dev:		SCMI protocol device
	 * @message_id:		SCMI message ID
	 * @attributes:		Message-specific attributes
	 *
	 * Obtain the message-specific attributes in @attributes.
	 * This command succeeds if the message is implemented and available.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*protocol_message_attrs)(struct udevice *dev, u32 message_id,
				      u32 *attributes);
	/**
	 * base_discover_vendor - get vendor name
	 * @dev:	SCMI protocol device
	 * @vendor:	Pointer to vendor name
	 *
	 * Obtain the vendor's name in @vendor.
	 * It is a caller's responsibility to free @vendor.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*base_discover_vendor)(struct udevice *dev, u8 **vendor);
	/**
	 * base_discover_sub_vendor - get sub-vendor name
	 * @dev:	SCMI protocol device
	 * @sub_vendor:	Pointer to sub-vendor name
	 *
	 * Obtain the sub-vendor's name in @sub_vendor.
	 * It is a caller's responsibility to free @sub_vendor.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*base_discover_sub_vendor)(struct udevice *dev, u8 **sub_vendor);
	/**
	 * base_discover_impl_version - get implementation version
	 * @dev:		SCMI protocol device
	 * @impl_version:	Pointer to implementation version
	 *
	 * Obtain the implementation version number in @impl_version.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*base_discover_impl_version)(struct udevice *dev,
					  u32 *impl_version);
	/**
	 * base_discover_list_protocols - get list of protocols
	 * @dev:	SCMI protocol device
	 * @protocols:	Pointer to array of SCMI protocols
	 *
	 * Obtain the list of protocols provided in @protocols.
	 * The number of elements in @protocols always match to the number of
	 * protocols returned by smci_protocol_attrs() when this function
	 * succeeds.
	 * It is a caller's responsibility to free @protocols.
	 *
	 * Return: the number of protocols in @protocols on success,
	 * error code on failure
	 */
	int (*base_discover_list_protocols)(struct udevice *dev,
					    u8 **protocols);
	/**
	 * base_discover_agent - identify agent
	 * @dev:		SCMI protocol device
	 * @agent_id:		SCMI agent ID
	 * @ret_agent_id:	Pointer to SCMI agent ID
	 * @name:		Pointer to SCMI agent name
	 *
	 * Obtain the agent's name in @name. If @agent_id is equal to
	 * 0xffffffff, * this function returns the caller's agent id in
	 * @ret_agent_id.
	 * It is a caller's responsibility to free @name.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*base_discover_agent)(struct udevice *dev, u32 agent_id,
				   u32 *ret_agent_id, u8 **name);
	/**
	 * base_notify_errors - configure error notification
	 * @dev:	SCMI protocol device
	 * @enable:	Operation
	 *
	 * Enable or disable error notification from SCMI firmware.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*base_notify_errors)(struct udevice *dev, u32 enable);
	/**
	 * base_set_device_permissions - configure access permission to device
	 * @dev:	SCMI protocol device
	 * @agent_id:	SCMI agent ID
	 * @device_id:	ID of device to access
	 * @flags:	A set of flags
	 *
	 * Ask for allowing or denying access permission to the device,
	 * @device_id. The meaning of @flags is defined in SCMI specification.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*base_set_device_permissions)(struct udevice *dev, u32 agent_id,
					   u32 device_id, u32 flags);
	/**
	 * base_set_protocol_permissions - configure access permission to
	 *				   protocol on device
	 * @dev:	SCMI protocol device
	 * @agent_id:	SCMI agent ID
	 * @device_id:	ID of device to access
	 * @command_id:	command ID
	 * @flags:	A set of flags
	 *
	 * Ask for allowing or denying access permission to the protocol,
	 * @command_id, on the device, @device_id.
	 * The meaning of @flags is defined in SCMI specification.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*base_set_protocol_permissions)(struct udevice *dev, u32 agent_id,
					     u32 device_id, u32 command_id,
					     u32 flags);
	/**
	 * base_reset_agent_configuration - reset resource settings
	 * @dev:	SCMI protocol device
	 * @agent_id:	SCMI agent ID
	 * @flags:	A set of flags
	 *
	 * Reset all the resource settings against @agent_id.
	 * The meaning of @flags is defined in SCMI specification.
	 *
	 * Return: 0 on success, error code on failure
	 */
	int (*base_reset_agent_configuration)(struct udevice *dev, u32 agent_id,
					      u32 flags);
};

/**
 * scmi_generic_protocol_version - get protocol version
 * @dev:	SCMI protocol device
 * @id:		SCMI protocol ID
 * @version:	Pointer to SCMI protocol version
 *
 * Obtain the protocol version number in @version.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_generic_protocol_version(struct udevice *dev,
				  enum scmi_std_protocol id, u32 *version);

/**
 * scmi_base_protocol_version - get Base protocol version
 * @dev:	SCMI protocol device
 * @version:	Pointer to SCMI protocol version
 *
 * Obtain the protocol version number in @version for Base protocol.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_base_protocol_version(struct udevice *dev, u32 *version);

/**
 * scmi_protocol_attrs - get protocol attributes
 * @dev:		SCMI protocol device
 * @num_agents:		Number of SCMI agents
 * @num_protocols:	Number of SCMI protocols
 *
 * Obtain the protocol attributes, the number of agents and the number
 * of protocols, in @num_agents and @num_protocols respectively, that
 * the device provides.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_base_protocol_attrs(struct udevice *dev, u32 *num_agents,
			     u32 *num_protocols);

/**
 * scmi_protocol_message_attrs - get message-specific attributes
 * @dev:		SCMI protocol device
 * @message_id:		SCMI message ID
 * @attributes:		Message-specific attributes
 *
 * Obtain the message-specific attributes in @attributes.
 * This command succeeds if the message is implemented and available.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_base_protocol_message_attrs(struct udevice *dev, u32 message_id,
				     u32 *attributes);

/**
 * scmi_base_discover_vendor - get vendor name
 * @dev:	SCMI protocol device
 * @vendor:	Pointer to vendor name
 *
 * Obtain the vendor's name in @vendor.
 * It is a caller's responsibility to free @vendor.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_base_discover_vendor(struct udevice *dev, u8 **vendor);

/**
 * scmi_base_discover_sub_vendor - get sub-vendor name
 * @dev:	SCMI protocol device
 * @sub_vendor:	Pointer to sub-vendor name
 *
 * Obtain the sub-vendor's name in @sub_vendor.
 * It is a caller's responsibility to free @sub_vendor.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_base_discover_sub_vendor(struct udevice *dev, u8 **sub_vendor);

/**
 * scmi_base_discover_impl_version - get implementation version
 * @dev:		SCMI protocol device
 * @impl_version:	Pointer to implementation version
 *
 * Obtain the implementation version number in @impl_version.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_base_discover_impl_version(struct udevice *dev, u32 *impl_version);

/**
 * scmi_base_discover_list_protocols - get list of protocols
 * @dev:	SCMI protocol device
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
int scmi_base_discover_list_protocols(struct udevice *dev, u8 **protocols);

/**
 * scmi_base_discover_agent - identify agent
 * @dev:		SCMI protocol device
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
int scmi_base_discover_agent(struct udevice *dev, u32 agent_id,
			     u32 *ret_agent_id, u8 **name);

/**
 * scmi_base_notify_errors - configure error notification
 * @dev:	SCMI protocol device
 * @enable:	Operation
 *
 * Enable or disable error notification from SCMI firmware.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_base_notify_errors(struct udevice *dev, u32 enable);

/**
 * scmi_base_set_device_permissions - configure access permission to device
 * @dev:	SCMI protocol device
 * @agent_id:	SCMI agent ID
 * @device_id:	ID of device to access
 * @flags:	A set of flags
 *
 * Ask for allowing or denying access permission to the device, @device_id.
 * The meaning of @flags is defined in SCMI specification.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_base_set_device_permissions(struct udevice *dev, u32 agent_id,
				     u32 device_id, u32 flags);

/**
 * scmi_base_set_protocol_permissions - configure access permission to
 *					protocol on device
 * @dev:	SCMI protocol device
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
int scmi_base_set_protocol_permissions(struct udevice *dev,
				       u32 agent_id, u32 device_id,
				       u32 command_id, u32 flags);

/**
 * scmi_base_reset_agent_configuration - reset resource settings
 * @dev:	SCMI protocol device
 * @agent_id:	SCMI agent ID
 * @flags:	A set of flags
 *
 * Reset all the resource settings against @agent_id.
 * The meaning of @flags is defined in SCMI specification.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_base_reset_agent_configuration(struct udevice *dev, u32 agent_id,
					u32 flags);

/*
 * SCMI Power Domain Management Protocol
 */

#define SCMI_PWD_PROTOCOL_VERSION 0x30000
#define SCMI_PWD_PSTATE_TYPE_LOST BIT(30)
#define SCMI_PWD_PSTATE_ID GENMASK(27, 0)

enum scmi_power_domain_message_id {
	SCMI_PWD_ATTRIBUTES = 0x3,
	SCMI_PWD_STATE_SET = 0x4,
	SCMI_PWD_STATE_GET = 0x5,
	SCMI_PWD_STATE_NOTIFY = 0x6,
	SCMI_PWD_STATE_CHANGE_REQUESTED_NOTIFY = 0x7,
	SCMI_PWD_NAME_GET = 0x8,
};

/**
 * struct scmi_pwd_protocol_attrs_out
 * @status:		SCMI command status
 * @attributes:		Protocol attributes
 * @stats_addr_low:	Lower 32 bits of address of statistics memory region
 * @stats_addr_high:	Higher 32 bits of address of statistics memory region
 * @stats_len:		Length of statistics memory region
 */
struct scmi_pwd_protocol_attrs_out {
	s32 status;
	u32 attributes;
	u32 stats_addr_low;
	u32 stats_addr_high;
	u32 stats_len;
};

#define SCMI_PWD_PROTO_ATTRS_NUM_PWD(attributes) ((attributes) & GENMASK(15, 0))

/**
 * struct scmi_pwd_protocol_msg_attrs_out
 * @status:		SCMI command status
 * @attributes:		Message-specific attributes
 */
struct scmi_pwd_protocol_msg_attrs_out {
	s32 status;
	u32 attributes;
};

#define SCMI_PWD_NAME_LENGTH_MAX 16

/**
 * struct scmi_pwd_attrs_out
 * @status:	SCMI command status
 * @attributes:	Power domain attributes
 * @name:	Name of power domain
 */
struct scmi_pwd_attrs_out {
	s32 status;
	u32 attributes;
	u8 name[SCMI_PWD_NAME_LENGTH_MAX];
};

#define SCMI_PWD_ATTR_PSTATE_CHANGE_NOTIFY	BIT(31)
#define SCMI_PWD_ATTR_PSTATE_ASYNC		BIT(30)
#define SCMI_PWD_ATTR_PSTATE_SYNC		BIT(29)
#define SCMI_PWD_ATTR_PSTATE_CHANGE_RQ_NOTIFY	BIT(28)
#define SCMI_PWD_ATTR_EXTENDED_NAME		BIT(27)

/**
 * struct scmi_pwd_state_set_in
 * @flags:	Flags
 * @domain_id:	Identifier of power domain
 * @pstate:	Power state of the domain
 */
struct scmi_pwd_state_set_in {
	u32 flags;
	u32 domain_id;
	u32 pstate;
};

#define SCMI_PWD_SET_FLAGS_ASYNC BIT(0)

/**
 * struct scmi_pwd_state_get_out
 * @status:	SCMI command status
 * @pstate:	Power state of the domain
 */
struct scmi_pwd_state_get_out {
	s32 status;
	u32 pstate;
};

#define SCMI_PWD_EXTENDED_NAME_MAX 64
/**
 * struct scmi_pwd_name_get_out
 * @status:		SCMI command status
 * @flags:		Parameter flags
 * @extended_name:	Extended name of power domain
 */
struct scmi_pwd_name_get_out {
	s32 status;
	u32 flags;
	u8 extended_name[SCMI_PWD_EXTENDED_NAME_MAX];
};

/**
 * scmi_pwd_protocol_attrs - get protocol attributes
 * @dev:	SCMI protocol device
 * @num_pwdoms:	Number of power domains
 * @stats_addr:	Address of statistics memory region
 * @stats_len:	Length of statistics memory region
 *
 * Obtain the protocol attributes, the number of power domains and
 * the information of statistics memory region.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_pwd_protocol_attrs(struct udevice *dev, int *num_pwdoms,
			    u64 *stats_addr, size_t *stats_len);
/**
 * scmi_pwd_protocol_message_attrs - get message-specific attributes
 * @dev:		SCMI protocol device
 * @message_id:		SCMI message ID
 * @attributes:		Message-specific attributes
 *
 * Obtain the message-specific attributes in @attributes.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_pwd_protocol_message_attrs(struct udevice *dev, s32 message_id,
				    u32 *attributes);
/**
 * scmi_pwd_attrs - get power domain attributes
 * @dev:	SCMI protocol device
 * @domain_id:	Identifier of power domain
 * @attributes:	Power domain attributes
 * @name:	Name of power domain
 *
 * Obtain the attributes of the given power domain, @domain_id, in @attributes
 * as well as its name in @name.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_pwd_attrs(struct udevice *dev, u32 message_id, u32 *attributes,
		   u8 **name);
/**
 * scmi_pwd_state_set - set power state
 * @dev:	SCMI protocol device
 * @flags:	Parameter flags
 * @domain_id:	Identifier of power domain
 * @pstate:	Power state
 *
 * Change the power state of the given power domain, @domain_id.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_pwd_state_set(struct udevice *dev, u32 flags, u32 domain_id,
		       u32 pstate);
/**
 * scmi_pwd_state_get - get power state
 * @dev:	SCMI protocol device
 * @domain_id:	Identifier of power domain
 * @pstate:	Power state
 *
 * Obtain the power state of the given power domain, @domain_id.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_pwd_state_get(struct udevice *dev, u32 domain_id, u32 *pstate);
/**
 * scmi_pwd_name_get - get extended name
 * @dev:	SCMI protocol device
 * @domain_id:	Identifier of power domain
 * @name:	Extended name of the domain
 *
 * Obtain the extended name of the given power domain, @domain_id, in @name.
 *
 * Return: 0 on success, error code on failure
 */
int scmi_pwd_name_get(struct udevice *dev, u32 domain_id, u8 **name);

/*
 * SCMI Clock Protocol
 */

enum scmi_clock_message_id {
	SCMI_CLOCK_ATTRIBUTES = 0x3,
	SCMI_CLOCK_RATE_SET = 0x5,
	SCMI_CLOCK_RATE_GET = 0x6,
	SCMI_CLOCK_CONFIG_SET = 0x7,
};

#define SCMI_CLK_PROTO_ATTR_COUNT_MASK	GENMASK(15, 0)
#define SCMI_CLK_RATE_ASYNC_NOTIFY	BIT(0)
#define SCMI_CLK_RATE_ASYNC_NORESP	(BIT(0) | BIT(1))
#define SCMI_CLK_RATE_ROUND_DOWN	0
#define SCMI_CLK_RATE_ROUND_UP		BIT(2)
#define SCMI_CLK_RATE_ROUND_CLOSEST	BIT(3)

#define SCMI_CLOCK_NAME_LENGTH_MAX 16

/**
 * struct scmi_clk_get_nb_out - Response for SCMI_PROTOCOL_ATTRIBUTES command
 * @status:	SCMI command status
 * @attributes:	Attributes of the clock protocol, mainly number of clocks exposed
 */
struct scmi_clk_protocol_attr_out {
	s32 status;
	u32 attributes;
};

/**
 * struct scmi_clk_attribute_in - Message payload for SCMI_CLOCK_ATTRIBUTES command
 * @clock_id:	SCMI clock ID
 */
struct scmi_clk_attribute_in {
	u32 clock_id;
};

/**
 * struct scmi_clk_get_nb_out - Response payload for SCMI_CLOCK_ATTRIBUTES command
 * @status:	SCMI command status
 * @attributes:	clock attributes
 * @clock_name:	name of the clock
 */
struct scmi_clk_attribute_out {
	s32 status;
	u32 attributes;
	char clock_name[SCMI_CLOCK_NAME_LENGTH_MAX];
};

/**
 * struct scmi_clk_state_in - Message payload for CLOCK_CONFIG_SET command
 * @clock_id:	SCMI clock ID
 * @attributes:	Attributes of the targets clock state
 */
struct scmi_clk_state_in {
	u32 clock_id;
	u32 attributes;
};

/**
 * struct scmi_clk_state_out - Response payload for CLOCK_CONFIG_SET command
 * @status:	SCMI command status
 */
struct scmi_clk_state_out {
	s32 status;
};

/**
 * struct scmi_clk_state_in - Message payload for CLOCK_RATE_GET command
 * @clock_id:	SCMI clock ID
 * @attributes:	Attributes of the targets clock state
 */
struct scmi_clk_rate_get_in {
	u32 clock_id;
};

/**
 * struct scmi_clk_rate_get_out - Response payload for CLOCK_RATE_GET command
 * @status:	SCMI command status
 * @rate_lsb:	32bit LSB of the clock rate in Hertz
 * @rate_msb:	32bit MSB of the clock rate in Hertz
 */
struct scmi_clk_rate_get_out {
	s32 status;
	u32 rate_lsb;
	u32 rate_msb;
};

/**
 * struct scmi_clk_state_in - Message payload for CLOCK_RATE_SET command
 * @flags:	Flags for the clock rate set request
 * @clock_id:	SCMI clock ID
 * @rate_lsb:	32bit LSB of the clock rate in Hertz
 * @rate_msb:	32bit MSB of the clock rate in Hertz
 */
struct scmi_clk_rate_set_in {
	u32 flags;
	u32 clock_id;
	u32 rate_lsb;
	u32 rate_msb;
};

/**
 * struct scmi_clk_rate_set_out - Response payload for CLOCK_RATE_SET command
 * @status:	SCMI command status
 */
struct scmi_clk_rate_set_out {
	s32 status;
};

/*
 * SCMI Reset Domain Protocol
 */

enum scmi_reset_domain_message_id {
	SCMI_RESET_DOMAIN_ATTRIBUTES = 0x3,
	SCMI_RESET_DOMAIN_RESET = 0x4,
};

#define SCMI_RD_NAME_LEN		16

#define SCMI_RD_ATTRIBUTES_FLAG_ASYNC	BIT(31)
#define SCMI_RD_ATTRIBUTES_FLAG_NOTIF	BIT(30)

#define SCMI_RD_RESET_FLAG_ASYNC	BIT(2)
#define SCMI_RD_RESET_FLAG_ASSERT	BIT(1)
#define SCMI_RD_RESET_FLAG_CYCLE	BIT(0)

/**
 * struct scmi_rd_attr_in - Payload for RESET_DOMAIN_ATTRIBUTES message
 * @domain_id:	SCMI reset domain ID
 */
struct scmi_rd_attr_in {
	u32 domain_id;
};

/**
 * struct scmi_rd_attr_out - Payload for RESET_DOMAIN_ATTRIBUTES response
 * @status:	SCMI command status
 * @attributes:	Retrieved attributes of the reset domain
 * @latency:	Reset cycle max lantency
 * @name:	Reset domain name
 */
struct scmi_rd_attr_out {
	s32 status;
	u32 attributes;
	u32 latency;
	char name[SCMI_RD_NAME_LEN];
};

/**
 * struct scmi_rd_reset_in - Message payload for RESET command
 * @domain_id:		SCMI reset domain ID
 * @flags:		Flags for the reset request
 * @reset_state:	Reset target state
 */
struct scmi_rd_reset_in {
	u32 domain_id;
	u32 flags;
	u32 reset_state;
};

/**
 * struct scmi_rd_reset_out - Response payload for RESET command
 * @status:	SCMI command status
 */
struct scmi_rd_reset_out {
	s32 status;
};

/*
 * SCMI Voltage Domain Protocol
 */

enum scmi_voltage_domain_message_id {
	SCMI_VOLTAGE_DOMAIN_ATTRIBUTES = 0x3,
	SCMI_VOLTAGE_DOMAIN_CONFIG_SET = 0x5,
	SCMI_VOLTAGE_DOMAIN_CONFIG_GET = 0x6,
	SCMI_VOLTAGE_DOMAIN_LEVEL_SET = 0x7,
	SCMI_VOLTAGE_DOMAIN_LEVEL_GET = 0x8,
};

#define SCMI_VOLTD_NAME_LEN		16

#define SCMI_VOLTD_CONFIG_MASK		GENMASK(3, 0)
#define SCMI_VOLTD_CONFIG_OFF		0
#define SCMI_VOLTD_CONFIG_ON		0x7

/**
 * struct scmi_voltd_attr_in - Payload for VOLTAGE_DOMAIN_ATTRIBUTES message
 * @domain_id:	SCMI voltage domain ID
 */
struct scmi_voltd_attr_in {
	u32 domain_id;
};

/**
 * struct scmi_voltd_attr_out - Payload for VOLTAGE_DOMAIN_ATTRIBUTES response
 * @status:	SCMI command status
 * @attributes:	Retrieved attributes of the voltage domain
 * @name:	Voltage domain name
 */
struct scmi_voltd_attr_out {
	s32 status;
	u32 attributes;
	char name[SCMI_VOLTD_NAME_LEN];
};

/**
 * struct scmi_voltd_config_set_in - Message payload for VOLTAGE_CONFIG_SET cmd
 * @domain_id:	SCMI voltage domain ID
 * @config:	Configuration data of the voltage domain
 */
struct scmi_voltd_config_set_in {
	u32 domain_id;
	u32 config;
};

/**
 * struct scmi_voltd_config_set_out - Response for VOLTAGE_CONFIG_SET command
 * @status:	SCMI command status
 */
struct scmi_voltd_config_set_out {
	s32 status;
};

/**
 * struct scmi_voltd_config_get_in - Message payload for VOLTAGE_CONFIG_GET cmd
 * @domain_id:	SCMI voltage domain ID
 */
struct scmi_voltd_config_get_in {
	u32 domain_id;
};

/**
 * struct scmi_voltd_config_get_out - Response for VOLTAGE_CONFIG_GET command
 * @status:	SCMI command status
 * @config:	Configuration data of the voltage domain
 */
struct scmi_voltd_config_get_out {
	s32 status;
	u32 config;
};

/**
 * struct scmi_voltd_level_set_in - Message payload for VOLTAGE_LEVEL_SET cmd
 * @domain_id:		SCMI voltage domain ID
 * @flags:		Parameter flags for configuring target level
 * @voltage_level:	Target voltage level in microvolts (uV)
 */
struct scmi_voltd_level_set_in {
	u32 domain_id;
	u32 flags;
	s32 voltage_level;
};

/**
 * struct scmi_voltd_level_set_out - Response for VOLTAGE_LEVEL_SET command
 * @status:	SCMI	command status
 */
struct scmi_voltd_level_set_out {
	s32 status;
};

/**
 * struct scmi_voltd_level_get_in - Message payload for VOLTAGE_LEVEL_GET cmd
 * @domain_id:		SCMI voltage domain ID
 */
struct scmi_voltd_level_get_in {
	u32 domain_id;
};

/**
 * struct scmi_voltd_level_get_out - Response for VOLTAGE_LEVEL_GET command
 * @status:		SCMI command status
 * @voltage_level:	Voltage level in microvolts (uV)
 */
struct scmi_voltd_level_get_out {
	s32 status;
	s32 voltage_level;
};

#endif /* _SCMI_PROTOCOLS_H */
