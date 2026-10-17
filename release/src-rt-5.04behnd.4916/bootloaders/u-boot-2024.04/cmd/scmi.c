// SPDX-License-Identifier: GPL-2.0+
/*
 *  SCMI (System Control and Management Interface) utility command
 *
 *  Copyright (c) 2023 Linaro Limited
 *		Author: AKASHI Takahiro
 */

#include <command.h>
#include <exports.h>
#include <scmi_agent.h>
#include <scmi_agent-uclass.h>
#include <stdlib.h>
#include <asm/types.h>
#include <dm/device.h>
#include <dm/uclass.h> /* uclass_get_device */
#include <linux/bitfield.h>
#include <linux/bitops.h>

struct {
	enum scmi_std_protocol id;
	const char *name;
} protocol_name[] = {
	{SCMI_PROTOCOL_ID_BASE, "Base"},
	{SCMI_PROTOCOL_ID_POWER_DOMAIN, "Power domain management"},
	{SCMI_PROTOCOL_ID_SYSTEM, "System power management"},
	{SCMI_PROTOCOL_ID_PERF, "Performance domain management"},
	{SCMI_PROTOCOL_ID_CLOCK, "Clock management"},
	{SCMI_PROTOCOL_ID_SENSOR, "Sensor management"},
	{SCMI_PROTOCOL_ID_RESET_DOMAIN, "Reset domain management"},
	{SCMI_PROTOCOL_ID_VOLTAGE_DOMAIN, "Voltage domain management"},
};

/**
 * get_agent() - get SCMI agent device
 *
 * Return:	Pointer to SCMI agent device on success, NULL on failure
 */
static struct udevice *get_agent(void)
{
	struct udevice *agent;

	if (uclass_get_device(UCLASS_SCMI_AGENT, 0, &agent)) {
		printf("Cannot find any SCMI agent\n");
		return NULL;
	}

	return agent;
}

/**
 * get_base_proto() - get SCMI base protocol device
 * @agent:	SCMI agent device
 *
 * Return:	Pointer to SCMI base protocol device on success,
 *		NULL on failure
 */
static struct udevice *get_base_proto(struct udevice *agent)
{
	struct udevice *base_proto;

	if (!agent) {
		agent = get_agent();
		if (!agent)
			return NULL;
	}

	base_proto = scmi_get_protocol(agent, SCMI_PROTOCOL_ID_BASE);
	if (!base_proto) {
		printf("SCMI base protocol not found\n");
		return NULL;
	}

	return base_proto;
}

/**
 * get_proto_name() - get the name of SCMI protocol
 *
 * @id:		SCMI Protocol ID
 *
 * Get the printable name of the protocol, @id
 *
 * Return:	Name string on success, NULL on failure
 */
static const char *get_proto_name(enum scmi_std_protocol id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(protocol_name); i++)
		if (id == protocol_name[i].id)
			return protocol_name[i].name;

	return NULL;
}

/**
 * do_scmi_info() - get the information of SCMI services
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 *
 * Get the information of SCMI services using various interfaces
 * provided by the Base protocol.
 *
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 */
static int do_scmi_info(struct cmd_tbl *cmdtp, int flag, int argc,
			char * const argv[])
{
	struct udevice *agent, *base_proto;
	u32 agent_id, num_protocols;
	u8 *agent_name, *protocols;
	int i, ret;

	if (argc != 1)
		return CMD_RET_USAGE;

	agent = get_agent();
	if (!agent)
		return CMD_RET_FAILURE;
	base_proto = get_base_proto(agent);
	if (!base_proto)
		return CMD_RET_FAILURE;

	printf("SCMI device: %s\n", agent->name);
	printf("  protocol version: 0x%x\n", scmi_version(agent));
	printf("  # of agents: %d\n", scmi_num_agents(agent));
	for (i = 0; i < scmi_num_agents(agent); i++) {
		ret = scmi_base_discover_agent(base_proto, i, &agent_id,
					       &agent_name);
		if (ret) {
			if (ret != -EOPNOTSUPP)
				printf("base_discover_agent() failed for id: %d (%d)\n",
				       i, ret);
			break;
		}
		printf("    %c%2d: %s\n", i == scmi_agent_id(agent) ? '>' : ' ',
		       i, agent_name);
		free(agent_name);
	}
	printf("  # of protocols: %d\n", scmi_num_protocols(agent));
	num_protocols = scmi_num_protocols(agent);
	protocols = scmi_protocols(agent);
	if (protocols)
		for (i = 0; i < num_protocols; i++)
			printf("      %s\n", get_proto_name(protocols[i]));
	printf("  vendor: %s\n", scmi_vendor(agent));
	printf("  sub vendor: %s\n", scmi_sub_vendor(agent));
	printf("  impl version: 0x%x\n", scmi_impl_version(agent));

	return CMD_RET_SUCCESS;
}

/**
 * do_scmi_set_dev() - set access permission to device
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 *
 * Set access permission to device with SCMI_BASE_SET_DEVICE_PERMISSIONS
 *
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 */
static int do_scmi_set_dev(struct cmd_tbl *cmdtp, int flag, int argc,
			   char * const argv[])
{
	u32 agent_id, device_id, flags, attributes;
	char *end;
	struct udevice *base_proto;
	int ret;

	if (argc != 4)
		return CMD_RET_USAGE;

	agent_id = simple_strtoul(argv[1], &end, 16);
	if (*end != '\0')
		return CMD_RET_USAGE;

	device_id = simple_strtoul(argv[2], &end, 16);
	if (*end != '\0')
		return CMD_RET_USAGE;

	flags = simple_strtoul(argv[3], &end, 16);
	if (*end != '\0')
		return CMD_RET_USAGE;

	base_proto = get_base_proto(NULL);
	if (!base_proto)
		return CMD_RET_FAILURE;

	ret = scmi_base_protocol_message_attrs(base_proto,
					       SCMI_BASE_SET_DEVICE_PERMISSIONS,
					       &attributes);
	if (ret) {
		printf("This operation is not supported\n");
		return CMD_RET_FAILURE;
	}

	ret = scmi_base_set_device_permissions(base_proto, agent_id,
					       device_id, flags);
	if (ret) {
		printf("%s access to device:%u failed (%d)\n",
		       flags ? "Allowing" : "Denying", device_id, ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

/**
 * do_scmi_set_proto() - set protocol permission to device
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 *
 * Set protocol permission to device with SCMI_BASE_SET_PROTOCOL_PERMISSIONS
 *
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 */
static int do_scmi_set_proto(struct cmd_tbl *cmdtp, int flag, int argc,
			     char * const argv[])
{
	u32 agent_id, device_id, protocol_id, flags, attributes;
	char *end;
	struct udevice *base_proto;
	int ret;

	if (argc != 5)
		return CMD_RET_USAGE;

	agent_id = simple_strtoul(argv[1], &end, 16);
	if (*end != '\0')
		return CMD_RET_USAGE;

	device_id = simple_strtoul(argv[2], &end, 16);
	if (*end != '\0')
		return CMD_RET_USAGE;

	protocol_id = simple_strtoul(argv[3], &end, 16);
	if (*end != '\0')
		return CMD_RET_USAGE;

	flags = simple_strtoul(argv[4], &end, 16);
	if (*end != '\0')
		return CMD_RET_USAGE;

	base_proto = get_base_proto(NULL);
	if (!base_proto)
		return CMD_RET_FAILURE;

	ret = scmi_base_protocol_message_attrs(base_proto,
					       SCMI_BASE_SET_PROTOCOL_PERMISSIONS,
					       &attributes);
	if (ret) {
		printf("This operation is not supported\n");
		return CMD_RET_FAILURE;
	}

	ret = scmi_base_set_protocol_permissions(base_proto, agent_id,
						 device_id, protocol_id,
						 flags);
	if (ret) {
		printf("%s access to protocol:0x%x on device:%u failed (%d)\n",
		       flags ? "Allowing" : "Denying", protocol_id, device_id,
		       ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

/**
 * do_scmi_reset() - reset platform resource settings
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 *
 * Reset platform resource settings with BASE_RESET_AGENT_CONFIGURATION
 *
 * Return:	CMD_RET_SUCCESS on success, CMD_RET_RET_FAILURE on failure
 */
static int do_scmi_reset(struct cmd_tbl *cmdtp, int flag, int argc,
			 char * const argv[])
{
	u32 agent_id, flags, attributes;
	char *end;
	struct udevice *base_proto;
	int ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	agent_id = simple_strtoul(argv[1], &end, 16);
	if (*end != '\0')
		return CMD_RET_USAGE;

	flags = simple_strtoul(argv[2], &end, 16);
	if (*end != '\0')
		return CMD_RET_USAGE;

	base_proto = get_base_proto(NULL);
	if (!base_proto)
		return CMD_RET_FAILURE;

	ret = scmi_base_protocol_message_attrs(base_proto,
					       SCMI_BASE_RESET_AGENT_CONFIGURATION,
					       &attributes);
	if (ret) {
		printf("Reset is not supported\n");
		return CMD_RET_FAILURE;
	}

	ret = scmi_base_reset_agent_configuration(base_proto, agent_id, flags);
	if (ret) {
		printf("Reset failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static struct cmd_tbl cmd_scmi_sub[] = {
	U_BOOT_CMD_MKENT(info, CONFIG_SYS_MAXARGS, 1,
			 do_scmi_info, "", ""),
	U_BOOT_CMD_MKENT(perm_dev, CONFIG_SYS_MAXARGS, 1,
			 do_scmi_set_dev, "", ""),
	U_BOOT_CMD_MKENT(perm_proto, CONFIG_SYS_MAXARGS, 1,
			 do_scmi_set_proto, "", ""),
	U_BOOT_CMD_MKENT(reset, CONFIG_SYS_MAXARGS, 1,
			 do_scmi_reset, "", ""),
};

/**
 * do_scmi() - SCMI utility
 *
 * @cmdtp:	Command table
 * @flag:	Command flag
 * @argc:	Number of arguments
 * @argv:	Argument array
 *
 * Provide user interfaces to SCMI protocols.
 *
 * Return:	CMD_RET_SUCCESS on success,
 *		CMD_RET_USAGE or CMD_RET_RET_FAILURE on failure
 */
static int do_scmi(struct cmd_tbl *cmdtp, int flag,
		   int argc, char *const argv[])
{
	struct cmd_tbl *cp;

	if (argc < 2)
		return CMD_RET_USAGE;

	argc--; argv++;

	cp = find_cmd_tbl(argv[0], cmd_scmi_sub, ARRAY_SIZE(cmd_scmi_sub));
	if (!cp)
		return CMD_RET_USAGE;

	return cp->cmd(cmdtp, flag, argc, argv);
}

static char scmi_help_text[] =
	" - SCMI utility\n"
	" info - get the info of SCMI services\n"
	" perm_dev <agent-id in hex> <device-id in hex> <flags in hex>\n"
	"   - set access permission to device\n"
	" perm_proto <agent-id in hex> <device-id in hex> <protocol-id in hex> <flags in hex>\n"
	"   - set protocol permission to device\n"
	" reset <agent-id in hex> <flags in hex>\n"
	"   - reset platform resource settings\n"
	"";

U_BOOT_CMD(scmi, CONFIG_SYS_MAXARGS, 0, do_scmi, "SCMI utility",
	   scmi_help_text);
