// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <assert.h>
#include <ctype.h>
#include <dbus/dbus.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

#include <sys/stat.h>

#include <ell/ell.h>

#include "src/shared/shell.h"
#include "src/shared/util.h"

#include "mesh/mesh.h"
#include "mesh/mesh-defs.h"

#include "tools/mesh/agent.h"
#include "tools/mesh/cfgcli.h"
#include "tools/mesh/keys.h"
#include "tools/mesh/mesh-db.h"
#include "tools/mesh/model.h"
#include "tools/mesh/remote.h"

#define PROMPT_ON	COLOR_BLUE "[mesh-cfgclient]" COLOR_OFF "# "
#define PROMPT_OFF	"Waiting to connect to bluetooth-meshd..."

#define CFG_SRV_MODEL	0x0000
#define CFG_CLI_MODEL	0x0001

#define UNPROV_SCAN_MAX_SECS	300

#define DEFAULT_START_ADDRESS	0x00aa
#define DEFAULT_MAX_ADDRESS	(VIRTUAL_ADDRESS_LOW - 1)
#define DEFAULT_NET_INDEX	0x0000
#define MAX_CRPL_SIZE		0x7fff

#define DEFAULT_CFG_FILE	"config_db.json"

struct meshcfg_el {
	const char *path;
	uint8_t index;
	uint16_t mods[2];
};

struct meshcfg_app {
	const char *path;
	const char *agent_path;
	struct meshcfg_el ele;
	uint16_t cid;
	uint16_t pid;
	uint16_t vid;
	uint16_t crpl;
	uint8_t uuid[16];
};

struct meshcfg_node {
	const char *path;
	struct l_dbus_proxy *proxy;
	struct l_dbus_proxy *mgmt_proxy;
	union {
		uint64_t u64;
		uint8_t u8[8];
	} token;
};

struct unprov_device {
	time_t last_seen;
	int16_t rssi;
	uint8_t uuid[16];
};

struct generic_request {
	uint32_t arg1;
	uint32_t arg2;
	uint32_t arg3;
	uint8_t *data1;
	uint8_t *data2;
	const char *str;
};

static struct l_dbus *dbus;

static struct l_queue *node_proxies;
static struct l_dbus_proxy *net_proxy;
static struct meshcfg_node *local;
static struct model_info *cfgcli;

static struct l_queue *devices;

static bool prov_in_progress;
static const char *caps[] = {"static-oob", "out-numeric", "in-numeric"};

static bool have_config;

static struct meshcfg_app app = {
	.path = "/mesh/cfgclient",
	.agent_path = "/mesh/cfgclient/agent",
	.cid = 0x05f1,
	.pid = 0x0002,
	.vid = 0x0001,
	.crpl = MAX_CRPL_SIZE,
	.ele = {
		.path = "/mesh/cfgclient/ele0",
		.index = 0,
		.mods = {CFG_SRV_MODEL, CFG_CLI_MODEL}
	}
};

static const struct option options[] = {
	{ "config",		required_argument, 0, 'c' },
	{ "address-start",	required_argument, 0, 'a' },
	{ "address-range",	required_argument, 0, 'r' },
	{ "net-index",		required_argument, 0, 'n' },
	{ 0, 0, 0, 0 }
};

static const char *address_opt;
static const char *range_opt;
static const char *net_idx_opt;
static const char *config_opt;

static uint32_t iv_index;
static uint16_t low_addr;
static uint16_t high_addr;
static uint16_t prov_net_idx;
static const char *cfg_fname;

static const char **optargs[] = {
	&config_opt,
	&address_opt,
	&range_opt,
	&net_idx_opt,
};

static const char *help[] = {
	"Configuration file",
	"Starting unicast address for remote nodes",
	"Net index for provisioning subnet"
};

static const struct bt_shell_opt opt = {
	.options = options,
	.optno = sizeof(options) / sizeof(struct option),
	.optstr = "c:a:n:",
	.optarg = optargs,
	.help = help,
};

static const char *dbus_err_args = "org.freedesktop.DBus.Error.InvalidArgs";
static const char *dbus_err_fail = "org.freedesktop.DBus.Error.Failed";
static const char *dbus_err_support = "org.freedesktop.DBus.Error.NotSupported";

static bool parse_argument_on_off(int argc, char *argv[], bool *value)
{
	if (!strcmp(argv[1], "on") || !strcmp(argv[1], "yes")) {
		*value = TRUE;
		return TRUE;
	}

	if (!strcmp(argv[1], "off") || !strcmp(argv[1], "no")) {
		*value = FALSE;
		return TRUE;
	}

	bt_shell_printf("Invalid argument %s\n", argv[1]);
	return FALSE;
}

static bool match_device_uuid(const void *a, const void *b)
{
	const struct unprov_device *dev = a;
	const uint8_t *uuid = b;

	return (memcmp(dev->uuid, uuid, 16) == 0);
}

static void print_device(void *a, void *b)
{
	const struct unprov_device *dev = a;
	struct tm *tm = localtime(&dev->last_seen);
	char buf[80];
	char *str;

	assert(strftime(buf, sizeof(buf), "%c", tm));

	str = l_util_hexstring_upper(dev->uuid, sizeof(dev->uuid));
	bt_shell_printf("UUID: %s, RSSI %d, Seen: %s\n",
			str, dev->rssi, buf);

	l_free(str);
}

struct send_data {
	const char *ele_path;
	bool rmt;
	bool is_dev_key;
	uint16_t dst;
	uint16_t idx;
	uint8_t *data;
	uint16_t len;
};

struct key_data {
	const char *ele_path;
	uint16_t dst;
	uint16_t idx;
	uint16_t net_idx;
	bool update;
};

static void append_dict_entry_basic(struct l_dbus_message_builder *builder,
					const char *key, const char *signature,
					const void *data)
{
	if (!builder)
		return;

	l_dbus_message_builder_enter_dict(builder, "sv");
	l_dbus_message_builder_append_basic(builder, 's', key);
	l_dbus_message_builder_enter_variant(builder, signature);
	l_dbus_message_builder_append_basic(builder, signature[0], data);
	l_dbus_message_builder_leave_variant(builder);
	l_dbus_message_builder_leave_dict(builder);
}

static void append_byte_array(struct l_dbus_message_builder *builder,
					unsigned char *data, unsigned int len)
{
	unsigned int i;

	l_dbus_message_builder_enter_array(builder, "y");

	for (i = 0; i < len; i++)
		l_dbus_message_builder_append_basic(builder, 'y', &(data[i]));

	l_dbus_message_builder_leave_array(builder);
}

static void send_msg_setup(struct l_dbus_message *msg, void *user_data)
{
	struct send_data *req = user_data;
	struct l_dbus_message_builder *builder;

	builder = l_dbus_message_builder_new(msg);

	l_dbus_message_builder_append_basic(builder, 'o', req->ele_path);
	l_dbus_message_builder_append_basic(builder, 'q', &req->dst);
	if (req->is_dev_key)
		l_dbus_message_builder_append_basic(builder, 'b', &req->rmt);

	l_dbus_message_builder_append_basic(builder, 'q', &req->idx);

	/* Options */
	l_dbus_message_builder_enter_array(builder, "{sv}");
	l_dbus_message_builder_enter_dict(builder, "sv");
	l_dbus_message_builder_leave_dict(builder);
	l_dbus_message_builder_leave_array(builder);

	/* Data */
	append_byte_array(builder, req->data, req->len);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static bool send_msg(void *user_data, uint16_t dst, uint16_t idx,
						uint8_t *data, uint16_t len)
{
	struct send_data *req;
	uint16_t net_idx_tx = idx;
	bool is_dev_key;
	const char *method_name;

	is_dev_key = (idx == APP_IDX_DEV_REMOTE || idx == APP_IDX_DEV_LOCAL);
	method_name = is_dev_key ? "DevKeySend" : "Send";

	if (is_dev_key) {
		net_idx_tx = remote_get_subnet_idx(dst);
		if (net_idx_tx == NET_IDX_INVALID)
			return false;
	}

	req = l_new(struct send_data, 1);
	req->ele_path = user_data;
	req->dst = dst;
	req->idx = net_idx_tx;
	req->data = data;
	req->len = len;
	req->rmt = (idx == APP_IDX_DEV_REMOTE);
	req->is_dev_key = is_dev_key;

	return l_dbus_proxy_method_call(local->proxy, method_name,
					send_msg_setup, NULL, req, l_free) != 0;
}

static void send_key_setup(struct l_dbus_message *msg, void *user_data)
{
	struct key_data *req = user_data;
	struct l_dbus_message_builder *builder;

	builder = l_dbus_message_builder_new(msg);

	l_dbus_message_builder_append_basic(builder, 'o', req->ele_path);
	l_dbus_message_builder_append_basic(builder, 'q', &req->dst);
	l_dbus_message_builder_append_basic(builder, 'q', &req->idx);
	l_dbus_message_builder_append_basic(builder, 'q', &req->net_idx);
	l_dbus_message_builder_append_basic(builder, 'b', &req->update);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static bool send_key(void *user_data, uint16_t dst, uint16_t key_idx,
						bool is_appkey, bool update)
{
	struct key_data *req;
	uint16_t net_idx;
	const char *method_name = (!is_appkey) ? "AddNetKey" : "AddAppKey";

	net_idx = remote_get_subnet_idx(dst);
	if (net_idx == NET_IDX_INVALID) {
		bt_shell_printf("Node %4.4x not found\n", dst);
		return false;
	}

	if (!is_appkey && !keys_subnet_exists(key_idx)) {
		bt_shell_printf("Local NetKey %u (0x%3.3x) not found\n",
							key_idx, key_idx);
		return false;
	}

	if (is_appkey && (keys_get_bound_key(key_idx) == NET_IDX_INVALID)) {
		bt_shell_printf("Local AppKey %u (0x%3.3x) not found\n",
							key_idx, key_idx);
		return false;
	}

	req = l_new(struct key_data, 1);
	req->ele_path = user_data;
	req->dst = dst;
	req->idx = key_idx;
	req->net_idx = net_idx;
	req->update = update;

	return l_dbus_proxy_method_call(local->proxy, method_name,
				send_key_setup, NULL, req, l_free) != 0;
}

static void delete_node_setup(struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t primary;
	uint8_t ele_cnt;

	primary = (uint16_t) req->arg1;
	ele_cnt = (uint8_t) req->arg2;

	l_dbus_message_set_arguments(msg, "qy", primary, ele_cnt);
}

static void delete_node(uint16_t primary, uint8_t ele_cnt)
{
	struct generic_request *req;

	if (!local || !local->proxy || !local->mgmt_proxy) {
		bt_shell_printf("Node is not attached\n");
		return;
	}

	req = l_new(struct generic_request, 1);
	req->arg1 = primary;
	req->arg2 = ele_cnt;

	l_dbus_proxy_method_call(local->mgmt_proxy, "DeleteRemoteNode",
				delete_node_setup, NULL, req, l_free);
}

static void client_init(void)
{
	cfgcli = cfgcli_init(send_key, delete_node, (void *) app.ele.path);
	cfgcli->ops.set_send_func(send_msg, (void *) app.ele.path);
}

static bool caps_getter(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	uint32_t i;

	if (!l_dbus_message_builder_enter_array(builder, "s"))
		return false;
	for (i = 0; i < L_ARRAY_SIZE(caps); i++)
		l_dbus_message_builder_append_basic(builder, 's', caps[i]);

	l_dbus_message_builder_leave_array(builder);

	return true;
}

static void agent_input_done(oob_type_t type, void *buf, uint16_t len,
								void *user_data)
{
	struct l_dbus_message *msg = user_data;
	struct l_dbus_message *reply = NULL;
	struct l_dbus_message_builder *builder;
	uint32_t val_u32;
	uint8_t oob_data[16];

	switch (type) {
	case NONE:
	case OUTPUT:
	default:
		break;

	case ASCII:
		if (len > 8) {
			bt_shell_printf("Bad input length\n");
			break;
		}
		/* Fall Through */

	case HEXADECIMAL:
		if (len > 16) {
			bt_shell_printf("Bad input length\n");
			break;
		}
		memset(oob_data, 0, 16);
		memcpy(oob_data, buf, len);
		reply = l_dbus_message_new_method_return(msg);
		builder = l_dbus_message_builder_new(reply);
		append_byte_array(builder, oob_data, 16);
		l_dbus_message_builder_finalize(builder);
		l_dbus_message_builder_destroy(builder);
		break;

	case DECIMAL:
		if (len > 8) {
			bt_shell_printf("Bad input length\n");
			break;
		}

		val_u32 = l_get_be32(buf);
		reply = l_dbus_message_new_method_return(msg);
		l_dbus_message_set_arguments(reply, "u", val_u32);
		break;
	}

	if (!reply)
		reply = l_dbus_message_new_error(msg, dbus_err_fail, NULL);

	l_dbus_send(dbus, reply);
}

struct requested_action {
	const char *action;
	const char *description;
};

static struct requested_action display_numeric_table[] = {
	{ "push", "Push remote button %d times"},
	{ "twist", "Twist remote nob %d times"},
	{ "in-numeric", "Enter %d on remote device"},
	{ "out-numeric", "Enter %d on remote device"}
};

static struct requested_action prompt_numeric_table[] = {
	{ "blink", "Enter the number of times remote LED blinked"},
	{ "beep", "Enter the number of times remote device beeped"},
	{ "vibrate", "Enter the number of times remote device vibrated"},
	{ "in-numeric", "Enter the number displayed on remote device"},
	{ "out-numeric", "Enter the number displayed on remote device"}
};

static int get_action(char *str, bool prompt)
{
	struct requested_action *action_table;
	size_t len;
	int i, sz;

	if (!str)
		return -1;

	if (prompt) {
		len = strlen(str);
		sz = L_ARRAY_SIZE(prompt_numeric_table);
		action_table = prompt_numeric_table;
	} else {
		len = strlen(str);
		sz = L_ARRAY_SIZE(display_numeric_table);
		action_table = display_numeric_table;
	}

	for (i = 0; i < sz; ++i)
		if (len == strlen(action_table[i].action) &&
			!strcmp(str, action_table[i].action))
			return i;

	return -1;
}

static struct l_dbus_message *disp_numeric_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	char *str;
	uint32_t n;
	int action_index;

	if (!l_dbus_message_get_arguments(msg, "su", &str, &n)) {
		l_error("Cannot parse \"DisplayNumeric\" arguments");
		return l_dbus_message_new_error(msg, dbus_err_fail, NULL);
	}

	action_index = get_action(str, false);
	if (action_index < 0)
		return l_dbus_message_new_error(msg, dbus_err_support, NULL);

	str = l_strdup_printf(display_numeric_table[action_index].description,
									n);
	bt_shell_printf(COLOR_YELLOW "%s\n" COLOR_OFF, str);
	l_free(str);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *disp_string_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	const char *prompt = "Enter AlphaNumeric code on remote device:";
	char *str;

	if (!l_dbus_message_get_arguments(msg, "s", &str)) {
		l_error("Cannot parse \"DisplayString\" arguments");
		return l_dbus_message_new_error(msg, dbus_err_fail, NULL);
	}

	bt_shell_printf(COLOR_YELLOW "%s %s\n" COLOR_OFF, prompt, str);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *prompt_numeric_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	char *str;
	int action_index;
	const char *desc;

	if (!l_dbus_message_get_arguments(msg, "s", &str)) {
		l_error("Cannot parse \"PromptNumeric\" arguments");
		return l_dbus_message_new_error(msg, dbus_err_fail, NULL);
	}

	action_index = get_action(str, true);
	if (action_index < 0)
		return l_dbus_message_new_error(msg, dbus_err_support, NULL);

	desc = prompt_numeric_table[action_index].description;

	l_dbus_message_ref(msg);
	agent_input_request(DECIMAL, 8, desc, agent_input_done, msg);

	return NULL;
}

static struct l_dbus_message *prompt_static_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	char *str;

	if (!l_dbus_message_get_arguments(msg, "s", &str) || !str) {
		l_error("Cannot parse \"PromptStatic\" arguments");
		return l_dbus_message_new_error(msg, dbus_err_fail, NULL);
	}

	if (!strcmp(str, "in-alpha") || !strcmp(str, "out-alpha")) {
		l_dbus_message_ref(msg);
		agent_input_request(ASCII, 8, "Enter displayed Ascii code",
							agent_input_done, msg);
	} else if (!strcmp(str, "static-oob")) {
		l_dbus_message_ref(msg);
		agent_input_request(HEXADECIMAL, 16, "Enter Static Key",
							agent_input_done, msg);
	} else
		return l_dbus_message_new_error(msg, dbus_err_support, NULL);

	return NULL;
}

static void setup_agent_iface(struct l_dbus_interface *iface)
{
	l_dbus_interface_property(iface, "Capabilities", 0, "as", caps_getter,
								NULL);
	/* TODO: Other properties */
	l_dbus_interface_method(iface, "DisplayString", 0, disp_string_call,
							"", "s", "value");
	l_dbus_interface_method(iface, "DisplayNumeric", 0, disp_numeric_call,
						"", "su", "type", "number");
	l_dbus_interface_method(iface, "PromptNumeric", 0, prompt_numeric_call,
						"u", "s", "number", "type");
	l_dbus_interface_method(iface, "PromptStatic", 0, prompt_static_call,
						"ay", "s", "data", "type");
}

static bool register_agent(void)
{
	if (!l_dbus_register_interface(dbus, MESH_PROVISION_AGENT_INTERFACE,
					setup_agent_iface, NULL, false)) {
		l_error("Unable to register agent interface");
		return false;
	}

	if (!l_dbus_register_object(dbus, app.agent_path, NULL, NULL,
				MESH_PROVISION_AGENT_INTERFACE, NULL, NULL)) {
		l_error("Failed to register object %s", app.agent_path);
		return false;
	}

	if (!l_dbus_object_add_interface(dbus, app.agent_path,
					 L_DBUS_INTERFACE_PROPERTIES, NULL)) {
		l_error("Failed to add interface %s",
					L_DBUS_INTERFACE_PROPERTIES);
		return false;
	}

	return true;
}

static void try_set_node_proxy(void *a, void *b)
{
	struct l_dbus_proxy *proxy = a;
	const char *interface = l_dbus_proxy_get_interface(proxy);
	const char *path = l_dbus_proxy_get_path(proxy);

	if (strcmp(local->path, path))
		return;

	if (!strcmp(interface, MESH_MANAGEMENT_INTERFACE))
		local->mgmt_proxy = proxy;
	else if (!strcmp(interface, MESH_NODE_INTERFACE))
		local->proxy = proxy;
}

static void attach_node_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *msg, void *user_data)
{
	struct meshcfg_node *node = user_data;
	struct l_dbus_message_iter iter_cfg;
	uint32_t ivi;

	if (l_dbus_message_is_error(msg)) {
		const char *name;

		l_dbus_message_get_error(msg, &name, NULL);
		l_error("Failed to attach node: %s", name);
		goto fail;

	}

	if (!l_dbus_message_get_arguments(msg, "oa(ya(qa{sv}))",
						&local->path, &iter_cfg))
		goto fail;

	bt_shell_printf("Attached with path %s\n", local->path);

	/* Populate node's proxies */
	l_queue_foreach(node_proxies, try_set_node_proxy, node);

	/* Remove from orphaned proxies list */
	if (local->proxy)
		l_queue_remove(node_proxies, local->proxy);

	if (local->mgmt_proxy)
		l_queue_remove(node_proxies, local->mgmt_proxy);

	/* Inititalize config client model */
	client_init();

	if (l_dbus_proxy_get_property(local->proxy, "IvIndex", "u", &ivi) &&
							ivi != iv_index) {
		iv_index = ivi;
		mesh_db_set_iv_index(ivi);
		remote_clear_blacklisted_addresses(ivi);
	}

	return;

fail:
	l_free(node);
	node = NULL;
}

static void attach_node_setup(struct l_dbus_message *msg, void *user_data)
{
	l_dbus_message_set_arguments(msg, "ot", app.path,
						l_get_be64(local->token.u8));
}

static void create_net_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *msg, void *user_data)
{
	if (l_dbus_message_is_error(msg)) {
		const char *name;

		l_dbus_message_get_error(msg, &name, NULL);
		l_error("Failed to create network: %s", name);
		return;
	}
}

static void create_net_setup(struct l_dbus_message *msg, void *user_data)
{
	struct l_dbus_message_builder *builder;

	/* Generate random UUID */
	l_uuid_v4(app.uuid);

	builder = l_dbus_message_builder_new(msg);

	l_dbus_message_builder_append_basic(builder, 'o', app.path);
	append_byte_array(builder, app.uuid, 16);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void cmd_create_network(int argc, char *argv[])
{
	if (have_config) {
		l_error("Mesh network configuration exists (%s)", cfg_fname);
		return;
	}

	l_dbus_proxy_method_call(net_proxy, "CreateNetwork", create_net_setup,
						create_net_reply, NULL,
						NULL);

}

static void scan_reply(struct l_dbus_proxy *proxy, struct l_dbus_message *msg,
								void *user_data)
{
	if (l_dbus_message_is_error(msg)) {
		const char *name;

		l_dbus_message_get_error(msg, &name, NULL);
		l_error("Failed to start unprovisioned scan: %s", name);
		return;
	}

	bt_shell_printf("Unprovisioned scan started\n");
}

static void scan_setup(struct l_dbus_message *msg, void *user_data)
{
	uint16_t secs = (uint16_t) L_PTR_TO_UINT(user_data);
	struct l_dbus_message_builder *builder;

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_enter_array(builder, "{sv}");
	append_dict_entry_basic(builder, "Seconds", "q", &secs);
	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void cmd_scan_unprov(int argc, char *argv[])
{
	uint32_t secs = 0;
	bool enable;

	if (!local || !local->proxy || !local->mgmt_proxy) {
		bt_shell_printf("Node is not attached\n");
		return;
	}

	if (parse_argument_on_off(argc, argv, &enable) == FALSE) {
		bt_shell_printf("Failed to parse input\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (argc == 3)
		sscanf(argv[2], "%u", &secs);

	if (secs > UNPROV_SCAN_MAX_SECS)
		secs = UNPROV_SCAN_MAX_SECS;

	if (enable)
		l_dbus_proxy_method_call(local->mgmt_proxy, "UnprovisionedScan",
						scan_setup, scan_reply,
						L_UINT_TO_PTR(secs), NULL);
	else
		l_dbus_proxy_method_call(local->mgmt_proxy,
						"UnprovisionedScanCancel",
						NULL, NULL, NULL, NULL);

}

static void cmd_list_unprov(int argc, char *argv[])
{
	bt_shell_printf(COLOR_YELLOW "Unprovisioned devices:\n" COLOR_OFF);
	l_queue_foreach(devices, print_device, NULL);
}

static void cmd_list_nodes(int argc, char *argv[])
{
	remote_print_all();
}

static void cmd_keys(int argc, char *argv[])
{
	keys_print_keys();
}

static void free_generic_request(void *data)
{
	struct generic_request *req = data;

	l_free(req->data1);
	l_free(req->data2);
	l_free(req);
}

static void import_node_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t primary, net_idx;
	uint8_t ele_cnt;

	if (l_dbus_message_is_error(msg)) {
		const char *name;

		l_dbus_message_get_error(msg, &name, NULL);
		l_error("Failed to import remote node: %s", name);
		return;
	}

	net_idx = (uint16_t) req->arg1;
	primary = (uint16_t) req->arg2;
	ele_cnt = (uint8_t) req->arg3;

	remote_add_node(req->data1, primary, ele_cnt, net_idx);
}

static void import_node_setup(struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t primary;
	uint8_t ele_cnt;
	struct l_dbus_message_builder *builder;

	primary = (uint16_t) req->arg2;
	ele_cnt = (uint8_t) req->arg3;

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'q', &primary);
	l_dbus_message_builder_append_basic(builder, 'y', &ele_cnt);
	append_byte_array(builder, req->data2, 16);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void cmd_import_node(int argc, char *argv[])
{
	struct generic_request *req;
	size_t sz;

	if (!local || !local->proxy || !local->mgmt_proxy) {
		bt_shell_printf("Node is not attached\n");
		return;
	}

	if (argc < 6) {
		bt_shell_printf("UUID, element count and device key");
		bt_shell_printf("Unicast, element count and device key");
		bt_shell_printf("are required\n");
		return;
	}

	req = l_new(struct generic_request, 1);

	/* Device UUID */
	req->data1 = l_util_from_hexstring(argv[1], &sz);
	if (!req->data1 || sz != 16 || !l_uuid_is_valid(req->data1)) {
		l_error("Failed to generate UUID array from %s", argv[1]);
		goto fail;
	}

	/* NetKey Index*/
	if (sscanf(argv[2], "%04x", &req->arg1) != 1)
		goto fail;

	/* Unicast of the primary element */
	if (sscanf(argv[3], "%04x", &req->arg2) != 1)
		goto fail;

	/* Number of elements */
	if (sscanf(argv[4], "%u", &req->arg3) != 1)
		return;

	/* DevKey */
	req->data2 = l_util_from_hexstring(argv[5], &sz);
	if (!req->data2 || sz != 16) {
		l_error("Failed to generate DevKey array from %s", argv[5]);
		goto fail;
	}

	l_dbus_proxy_method_call(local->mgmt_proxy, "ImportRemoteNode",
					import_node_setup, import_node_reply,
					req, free_generic_request);

	return;

fail:
	free_generic_request(req);
}

static void subnet_set_phase_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t net_idx;
	uint8_t phase;

	if (l_dbus_message_is_error(msg)) {
		const char *name;

		l_dbus_message_get_error(msg, &name, NULL);
		l_error("Failed to set subnet phase: %s", name);
		return;
	}

	net_idx = (uint16_t) req->arg1;
	phase = (uint8_t) req->arg2;

	if (phase == KEY_REFRESH_PHASE_THREE)
		phase = KEY_REFRESH_PHASE_NONE;

	keys_set_net_key_phase(net_idx, phase, true);
}

static void subnet_set_phase_setup(struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t net_idx;
	uint8_t phase;

	net_idx = (uint16_t) req->arg1;
	phase = (uint8_t) req->arg2;

	l_dbus_message_set_arguments(msg, "qy", net_idx, phase);
}

static void cmd_subnet_set_phase(int argc, char *argv[])
{
	struct generic_request *req;

	if (!local || !local->proxy || !local->mgmt_proxy) {
		bt_shell_printf("Node is not attached\n");
		return;
	}

	if (argc < 3) {
		bt_shell_printf("NetKey index and phase are required\n");
		return;
	}

	req = l_new(struct generic_request, 1);

	if (sscanf(argv[1], "%04x", &req->arg1) != 1)
		goto fail;

	if (sscanf(argv[2], "%d", &req->arg2) != 1)
		goto fail;

	l_dbus_proxy_method_call(local->mgmt_proxy, "SetKeyPhase",
					subnet_set_phase_setup,
					subnet_set_phase_reply, req, l_free);

	return;

fail:
	l_free(req);
}

static void mgr_key_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t idx = (uint16_t) req->arg1;
	const char *method = req->str;

	if (l_dbus_message_is_error(msg)) {
		const char *name;

		l_dbus_message_get_error(msg, &name, NULL);
		l_error("Method %s returned error: %s", method, name);
		bt_shell_printf("Method %s returned error: %s\n", method, name);
		return;
	}

	if (!strcmp("CreateSubnet", method)) {
		keys_add_net_key(idx);
		mesh_db_net_key_add(idx);
	} else if (!strcmp("DeleteSubnet", method)) {
		keys_del_net_key(idx);
		mesh_db_net_key_del(idx);
	} else if (!strcmp("UpdateSubnet", method)) {
		keys_set_net_key_phase(idx, KEY_REFRESH_PHASE_ONE, true);
	} else if (!strcmp("DeleteAppKey", method)) {
		keys_del_app_key(idx);
		mesh_db_app_key_del(idx);
	}
}

static void mgr_key_setup(struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t idx = (uint16_t) req->arg1;

	l_dbus_message_set_arguments(msg, "q", idx);
}

static void mgr_key_cmd(int argc, char *argv[], const char *method_name)
{
	struct generic_request *req;

	if (!local || !local->proxy || !local->mgmt_proxy) {
		bt_shell_printf("Node is not attached\n");
		return;
	}

	if (argc < 2) {
		bt_shell_printf("Missing required arguments\n");
		return;
	}

	req = l_new(struct generic_request, 1);

	if (sscanf(argv[1], "%04x", &req->arg1) != 1) {
		l_free(req);
		return;
	}

	req->str = method_name;

	l_dbus_proxy_method_call(local->mgmt_proxy, method_name,
					mgr_key_setup, mgr_key_reply,
					req, l_free);
}

static void cmd_delete_appkey(int argc, char *argv[])
{
	mgr_key_cmd(argc, argv, "DeleteAppKey");
}

static void cmd_update_appkey(int argc, char *argv[])
{
	mgr_key_cmd(argc, argv, "UpdateAppKey");
}

static void cmd_delete_subnet(int argc, char *argv[])
{
	mgr_key_cmd(argc, argv, "DeleteSubnet");
}

static void cmd_update_subnet(int argc, char *argv[])
{
	mgr_key_cmd(argc, argv, "UpdateSubnet");
}

static void cmd_create_subnet(int argc, char *argv[])
{
	mgr_key_cmd(argc, argv, "CreateSubnet");
}

static void add_key_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t net_idx, app_idx;
	const char *method = req->str;

	if (l_dbus_message_is_error(msg)) {
		const char *name;

		l_dbus_message_get_error(msg, &name, NULL);
		l_error("%s failed: %s", method, name);
		return;
	}

	net_idx = (uint16_t) req->arg1;

	if (!strcmp(method, "ImportSubnet")) {
		keys_add_net_key(net_idx);
		mesh_db_net_key_add(net_idx);
		return;
	}

	app_idx = (uint16_t) req->arg2;
	keys_add_app_key(net_idx, app_idx);
	mesh_db_app_key_add(net_idx, app_idx);
}

static void import_appkey_setup(struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t net_idx, app_idx;
	struct l_dbus_message_builder *builder;

	net_idx = (uint16_t) req->arg1;
	app_idx = (uint16_t) req->arg2;

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'q', &net_idx);
	l_dbus_message_builder_append_basic(builder, 'q', &app_idx);
	append_byte_array(builder, req->data1, 16);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void cmd_import_appkey(int argc, char *argv[])
{
	struct generic_request *req;
	size_t sz;

	if (!local || !local->proxy || !local->mgmt_proxy) {
		bt_shell_printf("Node is not attached\n");
		return;
	}

	if (argc < 4) {
		bt_shell_printf("Netkey and AppKey indices and");
		bt_shell_printf("key value are required\n");
		return;
	}

	req = l_new(struct generic_request, 1);

	if (sscanf(argv[1], "%04x", &req->arg1) != 1)
		goto fail;

	if (sscanf(argv[2], "%04x", &req->arg2) != 1)
		goto fail;

	req->data1 = l_util_from_hexstring(argv[3], &sz);
	if (!req->data1 || sz != 16) {
		l_error("Failed to generate key array from %s", argv[3]);
		goto fail;
	}

	req->str = "ImportAppKey";

	l_dbus_proxy_method_call(local->mgmt_proxy, "ImportAppKey",
					import_appkey_setup, add_key_reply,
					req, free_generic_request);

	return;

fail:
	free_generic_request(req);
}

static void import_subnet_setup(struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t net_idx;
	struct l_dbus_message_builder *builder;

	net_idx = (uint16_t) req->arg1;

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'q', &net_idx);
	append_byte_array(builder, req->data1, 16);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void cmd_import_subnet(int argc, char *argv[])
{
	struct generic_request *req;
	size_t sz;

	if (!local || !local->proxy || !local->mgmt_proxy) {
		bt_shell_printf("Node is not attached\n");
		return;
	}

	if (argc < 3) {
		bt_shell_printf("NetKey index and value are required\n");
		return;
	}

	req = l_new(struct generic_request, 1);

	if (sscanf(argv[1], "%04x", &req->arg1) != 1)
		goto fail;

	req->data1 = l_util_from_hexstring(argv[2], &sz);
	if (!req->data1 || sz != 16) {
		l_error("Failed to generate key array from %s", argv[2]);
		goto fail;
	}

	req->str = "ImportSubnet";

	l_dbus_proxy_method_call(local->mgmt_proxy, "ImportSubnet",
					import_subnet_setup, add_key_reply,
					req, free_generic_request);
	return;

fail:
	free_generic_request(req);
}

static void create_appkey_setup(struct l_dbus_message *msg, void *user_data)
{
	struct generic_request *req = user_data;
	uint16_t net_idx, app_idx;

	net_idx = (uint16_t) req->arg1;
	app_idx = (uint16_t) req->arg2;

	l_dbus_message_set_arguments(msg, "qq", net_idx, app_idx);
}

static void cmd_create_appkey(int argc, char *argv[])
{
	struct generic_request *req;

	if (!local || !local->proxy || !local->mgmt_proxy) {
		bt_shell_printf("Node is not attached\n");
		return;
	}

	if (argc < 3) {
		bt_shell_printf("AppKey index is required\n");
		return;
	}

	req = l_new(struct generic_request, 1);

	if (sscanf(argv[1], "%04x", &req->arg1) != 1)
		goto fail;

	if (sscanf(argv[2], "%04x", &req->arg2) != 1)
		goto fail;

	req->str = "CreateAppKey";

	l_dbus_proxy_method_call(local->mgmt_proxy, "CreateAppKey",
					create_appkey_setup, add_key_reply,
					req, l_free);
	return;

fail:
	l_free(req);
}

static void add_node_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *msg, void *user_data)
{
	if (l_dbus_message_is_error(msg)) {
		const char *name;

		prov_in_progress = false;
		l_dbus_message_get_error(msg, &name, NULL);
		l_error("Failed to start provisioning: %s", name);
		return;
	}

	bt_shell_printf("Provisioning started\n");
}

static void add_node_setup(struct l_dbus_message *msg, void *user_data)
{
	char *str = user_data;
	size_t sz;
	unsigned char *uuid;
	struct l_dbus_message_builder *builder;

	uuid = l_util_from_hexstring(str, &sz);
	if (!uuid || sz != 16 || !l_uuid_is_valid(uuid)) {
		l_error("Failed to generate UUID array from %s", str);
		return;
	}

	builder = l_dbus_message_builder_new(msg);
	append_byte_array(builder, uuid, 16);
	l_dbus_message_builder_enter_array(builder, "{sv}");
	/* TODO: populate with options when defined */
	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_free(uuid);
}

static void cmd_start_prov(int argc, char *argv[])
{
	if (!local || !local->proxy || !local->mgmt_proxy) {
		bt_shell_printf("Node is not attached\n");
		return;
	}

	if (prov_in_progress) {
		bt_shell_printf("Provisioning is already in progress\n");
		return;
	}

	if (!argv[1] || (strlen(argv[1]) != 32)) {
		bt_shell_printf(COLOR_RED "Requires UUID\n" COLOR_RED);
		return;
	}

	if (l_dbus_proxy_method_call(local->mgmt_proxy, "AddNode",
						add_node_setup, add_node_reply,
						argv[1], NULL))
		prov_in_progress = true;
}

static const struct bt_shell_menu main_menu = {
	.name = "main",
	.entries = {
	{ "create", "[unicast_range_low]", cmd_create_network,
			"Create new mesh network with one initial node" },
	{ "discover-unprovisioned", "<on/off> [seconds]", cmd_scan_unprov,
			"Look for devices to provision" },
	{ "appkey-create", "<net_idx> <app_idx>", cmd_create_appkey,
			"Create a new local AppKey" },
	{ "appkey-import", "<net_idx> <app_idx> <key>", cmd_import_appkey,
			"Import a new local AppKey" },
	{ "appkey-update", "<app_idx>", cmd_update_appkey,
			"Update local AppKey" },
	{ "appkey-delete", "<app_idx>", cmd_delete_appkey,
			"Delete local AppKey" },
	{ "subnet-create", "<net_idx>", cmd_create_subnet,
			"Create a new local subnet (NetKey)" },
	{ "subnet-import", "<net_idx> <key>", cmd_import_subnet,
			"Import a new local subnet (NetKey)" },
	{ "subnet-update", "<net_idx>", cmd_update_subnet,
			"Update local subnet (NetKey)" },
	{ "subnet-delete", "<net_idx>", cmd_delete_subnet,
			"Delete local subnet (NetKey)" },
	{ "subnet-set-phase", "<net_idx> <phase>", cmd_subnet_set_phase,
			"Set subnet (NetKey) phase" },
	{ "list-unprovisioned", NULL, cmd_list_unprov,
			"List unprovisioned devices" },
	{ "provision", "<uuid>", cmd_start_prov,
			"Initiate provisioning"},
	{ "node-import", "<uuid> <net_idx> <primary> <ele_count> <dev_key>",
			cmd_import_node,
			"Import an externally provisioned remote node"},
	{ "list-nodes", NULL, cmd_list_nodes,
			"List remote mesh nodes"},
	{ "keys", NULL, cmd_keys,
			"List available keys"},
	{ } },
};

static void proxy_added(struct l_dbus_proxy *proxy, void *user_data)
{
	const char *interface = l_dbus_proxy_get_interface(proxy);
	const char *path = l_dbus_proxy_get_path(proxy);

	bt_shell_printf("Proxy added: %s (%s)\n", interface, path);

	if (!strcmp(interface, MESH_NETWORK_INTERFACE)) {
		net_proxy = proxy;

		/*
		 * If mesh network configuration has been read from
		 * storage, attach the provisioner/config-client node.
		 */
		if (local)
			l_dbus_proxy_method_call(net_proxy, "Attach",
						attach_node_setup,
						attach_node_reply, NULL,
						NULL);
		return;
	}

	if (!strcmp(interface, MESH_MANAGEMENT_INTERFACE)) {

		if (local && local->path) {
			if (!strcmp(local->path, path))
				local->mgmt_proxy = proxy;
		} else
			l_queue_push_tail(node_proxies, proxy);
		return;
	}

	if (!strcmp(interface, MESH_NODE_INTERFACE)) {

		if (local && local->path) {
			if (!strcmp(local->path, path))
				local->proxy = proxy;
		} else
			l_queue_push_tail(node_proxies, proxy);
	}
}

static void proxy_removed(struct l_dbus_proxy *proxy, void *user_data)
{
	const char *interface = l_dbus_proxy_get_interface(proxy);
	const char *path = l_dbus_proxy_get_path(proxy);

	bt_shell_printf("Proxy removed: %s (%s)\n", interface, path);

	if (!strcmp(interface, MESH_NETWORK_INTERFACE)) {
		bt_shell_printf("Mesh removed, terminating.\n");
		l_main_quit();
		return;
	}

	if (!strcmp(interface, MESH_NODE_INTERFACE)) {
		if (local && local->path && !strcmp(local->path, path))
			local->proxy = NULL;

		l_queue_remove(node_proxies, proxy);
		return;
	}

	if (!strcmp(interface, MESH_MANAGEMENT_INTERFACE)) {
		if (local && local->path && !strcmp(local->path, path))
			local->mgmt_proxy = NULL;

		l_queue_remove(node_proxies, proxy);
	}
}

static void build_model(struct l_dbus_message_builder *builder, uint16_t mod_id,
					bool pub_enable, bool sub_enable)
{
	l_dbus_message_builder_enter_struct(builder, "qa{sv}");
	l_dbus_message_builder_append_basic(builder, 'q', &mod_id);
	l_dbus_message_builder_enter_array(builder, "{sv}");
	append_dict_entry_basic(builder, "Subscribe", "b", &sub_enable);
	append_dict_entry_basic(builder, "Publish", "b", &pub_enable);
	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_leave_struct(builder);
}

static bool mod_getter(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	l_dbus_message_builder_enter_array(builder, "(qa{sv})");
	build_model(builder, app.ele.mods[0], false, false);
	build_model(builder, app.ele.mods[1], false, false);
	l_dbus_message_builder_leave_array(builder);

	return true;
}

static bool vmod_getter(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	l_dbus_message_builder_enter_array(builder, "(qqa{sv})");
	l_dbus_message_builder_leave_array(builder);

	return true;
}

static bool ele_idx_getter(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	l_dbus_message_builder_append_basic(builder, 'y', &app.ele.index);

	return true;
}

static struct l_dbus_message *dev_msg_recv_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct l_dbus_message_iter iter;
	uint16_t src, idx;
	uint8_t *data;
	uint32_t n;
	bool rmt;

	if (!l_dbus_message_get_arguments(msg, "qbqay", &src, &rmt, &idx,
								&iter)) {
		l_error("Cannot parse received message");
		return l_dbus_message_new_error(msg, dbus_err_args, NULL);
	}

	if (!l_dbus_message_iter_get_fixed_array(&iter, &data, &n)) {
		l_error("Cannot parse received message: data");
		return l_dbus_message_new_error(msg, dbus_err_args, NULL);
	}

	bt_shell_printf("Received dev key message (len %u):", n);

	/* Pass to the configuration client */
	if (cfgcli && cfgcli->ops.recv)
		cfgcli->ops.recv(src, APP_IDX_DEV_REMOTE, data, n);

	return l_dbus_message_new_method_return(msg);
}

static void setup_ele_iface(struct l_dbus_interface *iface)
{
	/* Properties */
	l_dbus_interface_property(iface, "Index", 0, "y", ele_idx_getter,
									NULL);
	l_dbus_interface_property(iface, "VendorModels", 0, "a(qqa{sv})",
							vmod_getter, NULL);
	l_dbus_interface_property(iface, "Models", 0, "a(qa{sv})", mod_getter,
									NULL);

	/* Methods */
	l_dbus_interface_method(iface, "DevKeyMessageReceived", 0,
				dev_msg_recv_call, "", "qbqay", "source",
				"remote", "net_index", "data");

	/* TODO: Other methods */
}

static struct l_dbus_message *scan_result_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct l_dbus_message_iter iter, opts;
	int16_t rssi;
	uint32_t n;
	uint8_t *prov_data;
	char *str;
	struct unprov_device *dev;
	const char *sig = "naya{sv}";

	if (!l_dbus_message_get_arguments(msg, sig, &rssi, &iter, &opts)) {
		l_error("Cannot parse scan results");
		return l_dbus_message_new_error(msg, dbus_err_args, NULL);
	}

	if (!l_dbus_message_iter_get_fixed_array(&iter, &prov_data, &n) ||
								n < 16) {
		l_error("Cannot parse scan result: data");
		return l_dbus_message_new_error(msg, dbus_err_args, NULL);
	}

	bt_shell_printf("Scan result:\n");
	bt_shell_printf("\t" COLOR_GREEN "rssi = %d\n" COLOR_OFF, rssi);
	str = l_util_hexstring_upper(prov_data, 16);
	bt_shell_printf("\t" COLOR_GREEN "UUID = %s\n" COLOR_OFF, str);
	l_free(str);

	if (n >= 18) {
		str = l_util_hexstring_upper(prov_data + 16, 2);
		bt_shell_printf("\t" COLOR_GREEN "OOB = %s\n" COLOR_OFF, str);
		l_free(str);
	}

	if (n >= 22) {
		str = l_util_hexstring_upper(prov_data + 18, 4);
		bt_shell_printf("\t" COLOR_GREEN "URI Hash = %s\n" COLOR_OFF,
									str);
		l_free(str);
	}

	/* TODO: Handle the rest of provisioning data if present */

	dev = l_queue_find(devices, match_device_uuid, prov_data);
	if (!dev) {
		dev = l_new(struct unprov_device, 1);
		memcpy(dev->uuid, prov_data, sizeof(dev->uuid));
		/* TODO: timed self-destructor */
		l_queue_push_tail(devices, dev);
	}

	/* Update with the latest rssi */
	dev->rssi = rssi;
	dev->last_seen = time(NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *req_prov_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	uint8_t cnt;
	uint16_t unicast;
	struct l_dbus_message *reply;

	if (!l_dbus_message_get_arguments(msg, "y", &cnt)) {
		l_error("Cannot parse request for prov data");
		return l_dbus_message_new_error(msg, dbus_err_args, NULL);

	}

	unicast = remote_get_next_unicast(low_addr, high_addr, cnt);

	if (unicast == 0) {
		l_error("Failed to allocate addresses for %u elements\n", cnt);
		return l_dbus_message_new_error(msg,
					"org.freedesktop.DBus.Error."
					"Failed to allocate address", NULL);
	}

	bt_shell_printf("Assign addresses for %u elements\n", cnt);

	reply = l_dbus_message_new_method_return(msg);
	l_dbus_message_set_arguments(reply, "qq", prov_net_idx, unicast);

	return reply;
}

static void remove_device(uint8_t *uuid)
{
	struct unprov_device *dev;

	dev = l_queue_remove_if(devices, match_device_uuid, uuid);
	l_free(dev);
}

static struct l_dbus_message *add_node_cmplt_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct l_dbus_message_iter iter;
	int16_t unicast;
	uint8_t cnt;
	uint32_t n;
	uint8_t *uuid;

	if (!prov_in_progress)
		return l_dbus_message_new_error(msg, dbus_err_fail, NULL);

	prov_in_progress = false;

	if (!l_dbus_message_get_arguments(msg, "ayqy", &iter, &unicast, &cnt)) {
		l_error("Cannot parse add node complete message");
		return l_dbus_message_new_error(msg, dbus_err_args, NULL);

	}

	if (!l_dbus_message_iter_get_fixed_array(&iter, &uuid, &n) ||
								n != 16) {
		l_error("Cannot parse add node complete message: uuid");
		return l_dbus_message_new_error(msg, dbus_err_args, NULL);
	}

	remote_add_node(uuid, unicast, cnt, prov_net_idx);

	bt_shell_printf("Provisioning done:\n");
	remote_print_node(unicast);

	remove_device(uuid);

	if (!mesh_db_add_node(uuid, cnt, unicast, prov_net_idx))
		l_error("Failed to store new remote node");

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *add_node_fail_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct l_dbus_message_iter iter;
	uint32_t n;
	uint8_t *uuid;
	char *str, *reason;

	if (!prov_in_progress)
		return l_dbus_message_new_error(msg, dbus_err_fail, NULL);

	prov_in_progress = false;

	if (!l_dbus_message_get_arguments(msg, "ays", &iter, &reason)) {
		l_error("Cannot parse add node failed message");
		return l_dbus_message_new_error(msg, dbus_err_args, NULL);

	}

	if (!l_dbus_message_iter_get_fixed_array(&iter, &uuid, &n) ||
								n != 16) {
		l_error("Cannot parse add node failed message: uuid");
		return l_dbus_message_new_error(msg, dbus_err_args, NULL);
	}

	bt_shell_printf("Provisioning failed:\n");
	str = l_util_hexstring_upper(uuid, 16);
	bt_shell_printf("\t" COLOR_RED "UUID = %s\n" COLOR_OFF, str);
	l_free(str);
	bt_shell_printf("\t" COLOR_RED "%s\n" COLOR_OFF, reason);

	remove_device(uuid);

	return l_dbus_message_new_method_return(msg);
}

static void setup_prov_iface(struct l_dbus_interface *iface)
{
	l_dbus_interface_method(iface, "ScanResult", 0, scan_result_call, "",
					"naya{sv}", "rssi", "data", "options");

	l_dbus_interface_method(iface, "RequestProvData", 0, req_prov_call,
				"qq", "y", "net_index", "unicast", "count");

	l_dbus_interface_method(iface, "AddNodeComplete", 0,
					add_node_cmplt_call, "", "ayqy",
					"uuid", "unicast", "count");

	l_dbus_interface_method(iface, "AddNodeFailed", 0, add_node_fail_call,
					"", "ays", "uuid", "reason");
}

static bool cid_getter(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	l_dbus_message_builder_append_basic(builder, 'q', &app.cid);

	return true;
}

static bool pid_getter(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	l_dbus_message_builder_append_basic(builder, 'q', &app.pid);

	return true;
}

static bool vid_getter(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	l_dbus_message_builder_append_basic(builder, 'q', &app.vid);

	return true;
}
static bool crpl_getter(struct l_dbus *dbus,
				struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	l_dbus_message_builder_append_basic(builder, 'q', &app.crpl);

	return true;
}

static void attach_node(void *user_data)
{
	l_dbus_proxy_method_call(net_proxy, "Attach", attach_node_setup,
						attach_node_reply, NULL,
						NULL);
}

static struct l_dbus_message *join_complete(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	char *str;
	uint64_t tmp;

	if (!l_dbus_message_get_arguments(message, "t", &tmp))
		return l_dbus_message_new_error(message, dbus_err_args, NULL);

	local = l_new(struct meshcfg_node, 1);
	local->token.u64 = l_get_be64(&tmp);
	str = l_util_hexstring(&local->token.u8[0], 8);
	bt_shell_printf("Created new node with token %s\n", str);
	l_free(str);

	if (!mesh_db_create(cfg_fname, local->token.u8,
					"Mesh Config Client Network")) {
		l_free(local);
		local = NULL;
		return l_dbus_message_new_error(message, dbus_err_fail, NULL);
	}

	mesh_db_set_addr_range(low_addr, high_addr);
	keys_add_net_key(PRIMARY_NET_IDX);
	mesh_db_net_key_add(PRIMARY_NET_IDX);

	remote_add_node(app.uuid, 0x0001, 1, PRIMARY_NET_IDX);
	mesh_db_add_node(app.uuid, 0x0001, 1, PRIMARY_NET_IDX);

	l_idle_oneshot(attach_node, NULL, NULL);

	return l_dbus_message_new_method_return(message);
}

static void property_changed(struct l_dbus_proxy *proxy, const char *name,
				struct l_dbus_message *msg, void *user_data)
{
	const char *interface = l_dbus_proxy_get_interface(proxy);
	const char *path = l_dbus_proxy_get_path(proxy);

	if (strcmp(path, local->path))
		return;

	bt_shell_printf("Property changed: %s %s %s\n", name, path, interface);

	if (!strcmp(interface, "org.bluez.mesh.Node1")) {

		if (!strcmp(name, "IvIndex")) {
			uint32_t ivi;

			if (!l_dbus_message_get_arguments(msg, "u", &ivi))
				return;

			bt_shell_printf("New IV Index: %u\n", ivi);

			iv_index = ivi;
			mesh_db_set_iv_index(ivi);
			remote_clear_blacklisted_addresses(ivi);
		}
	}
}

static void setup_app_iface(struct l_dbus_interface *iface)
{
	l_dbus_interface_property(iface, "CompanyID", 0, "q", cid_getter,
									NULL);
	l_dbus_interface_property(iface, "VersionID", 0, "q", vid_getter,
									NULL);
	l_dbus_interface_property(iface, "ProductID", 0, "q", pid_getter,
									NULL);
	l_dbus_interface_property(iface, "CRPL", 0, "q", crpl_getter, NULL);

	l_dbus_interface_method(iface, "JoinComplete", 0, join_complete,
							"", "t", "token");

	/* TODO: Methods */
}

static bool register_app(void)
{
	if (!l_dbus_register_interface(dbus, MESH_APPLICATION_INTERFACE,
						setup_app_iface, NULL, false)) {
		l_error("Failed to register interface %s",
						MESH_APPLICATION_INTERFACE);
		return false;
	}

	if (!l_dbus_register_interface(dbus, MESH_PROVISIONER_INTERFACE,
					setup_prov_iface, NULL, false)) {
		l_error("Failed to register interface %s",
						MESH_PROVISIONER_INTERFACE);
		return false;
	}

	if (!l_dbus_register_object(dbus, app.path, NULL, NULL,
					MESH_APPLICATION_INTERFACE, NULL,
					MESH_PROVISIONER_INTERFACE, NULL,
									NULL)) {
		l_error("Failed to register object %s", app.path);
		return false;
	}

	if (!register_agent())
		return false;

	if (!l_dbus_register_interface(dbus, MESH_ELEMENT_INTERFACE,
						setup_ele_iface, NULL, false)) {
		l_error("Failed to register interface %s",
						MESH_ELEMENT_INTERFACE);
		return false;
	}

	if (!l_dbus_register_object(dbus, app.ele.path, NULL, NULL,
				    MESH_ELEMENT_INTERFACE, NULL, NULL)) {
		l_error("Failed to register object %s", app.ele.path);
		return false;
	}

	if (!l_dbus_object_add_interface(dbus, app.path,
					 L_DBUS_INTERFACE_OBJECT_MANAGER,
									NULL)) {
		l_error("Failed to add interface %s",
					L_DBUS_INTERFACE_OBJECT_MANAGER);
		return false;
	}

	return true;
}

static void client_ready(struct l_dbus_client *client, void *user_data)
{
	bt_shell_printf("D-Bus client ready\n");
	if (!register_app())
		bt_shell_quit(EXIT_FAILURE);

	bt_shell_attach(fileno(stdin));
}

static void client_connected(struct l_dbus *dbus, void *user_data)
{
	bt_shell_printf("D-Bus client connected\n");
	bt_shell_set_prompt(PROMPT_ON);
}

static void client_disconnected(struct l_dbus *dbus, void *user_data)
{
	bt_shell_printf("D-Bus client disconnected, exit\n");
	bt_shell_quit(EXIT_SUCCESS);
}

static void ready_callback(void *user_data)
{
	bt_shell_printf("Connected to D-Bus\n");
	if (!l_dbus_object_manager_enable(dbus, "/"))
		bt_shell_printf("Failed to register the ObjectManager\n");
}

static bool setup_cfg_storage(void)
{
	struct stat st;

	if (!config_opt) {
		char *home;
		char *mesh_dir;

		home = getenv("XDG_CONFIG_HOME");

		if (home) {
			mesh_dir = l_strdup_printf("%s/meshcfg", home);
		} else {
			home = getenv("HOME");
			if (!home) {
				l_error("\"HOME\" not set\n");
				return false;
			}

			mesh_dir = l_strdup_printf("%s/.config/meshcfg", home);
		}

		if (!mesh_dir)
			return false;

		if (stat(mesh_dir, &st) == 0) {
			if (!S_ISDIR(st.st_mode)) {
				l_error("%s not a directory", mesh_dir);
				return false;
			}
		} else if (errno == ENOENT) {
			if (mkdir(mesh_dir, 0700) != 0)
				return false;
		} else {
			perror("Cannot open config directory");
			return false;
		}

		cfg_fname = l_strdup_printf("%s/%s", mesh_dir,
							DEFAULT_CFG_FILE);
		l_free(mesh_dir);

	} else {
		cfg_fname = l_strdup_printf("%s", config_opt);
	}

	if (stat(cfg_fname, &st) == -1) {
		if (errno == ENOENT) {
			l_warn("\nWarning: config file \"%s\" not found",
								cfg_fname);
			return true;
		}

		perror("\nFailed to open config file");
		return false;
	}

	have_config = true;
	return true;
}

static bool read_mesh_config(void)
{
	uint16_t range_l, range_h;

	if (!mesh_db_load(cfg_fname)) {
		l_error("Failed to load config from %s", cfg_fname);
		return false;
	}

	local = l_new(struct meshcfg_node, 1);

	if (!mesh_db_get_token(local->token.u8)) {
		l_error("Failed to read the provisioner's token ID");
		l_error("Check config file %s", cfg_fname);
		l_free(local);
		local = NULL;

		return false;
	}

	l_info("Mesh configuration loaded from %s", cfg_fname);
	if (mesh_db_get_addr_range(&range_l, &range_h)) {
		low_addr = range_l;
		high_addr = range_h;
	}

	iv_index = mesh_db_get_iv_index();

	return true;
}

int main(int argc, char *argv[])
{
	struct l_dbus_client *client;
	uint32_t val;
	int status;

	bt_shell_init(argc, argv, &opt);
	bt_shell_set_menu(&main_menu);

	l_log_set_stderr();

	if (address_opt && sscanf(address_opt, "%04x", &val) == 1)
		low_addr = (uint16_t) val;

	if (low_addr > DEFAULT_MAX_ADDRESS) {
		l_error("Invalid start address");
			bt_shell_cleanup();
			return EXIT_FAILURE;
	}

	if (!low_addr)
		low_addr = DEFAULT_START_ADDRESS;

	if (range_opt && sscanf(address_opt, "%04x", &val) == 1) {
		if (val == 0) {
			l_error("Invalid address range");
			bt_shell_cleanup();
			return EXIT_FAILURE;
		}

		/* Inclusive */
		high_addr = low_addr + val - 1;
	}

	if (!high_addr || high_addr > DEFAULT_MAX_ADDRESS)
		high_addr = DEFAULT_MAX_ADDRESS;

	if (net_idx_opt && sscanf(net_idx_opt, "%04x", &val) == 1)
		prov_net_idx = (uint16_t) val;
	else
		prov_net_idx = DEFAULT_NET_INDEX;

	if (!setup_cfg_storage()) {
		bt_shell_cleanup();
		return EXIT_FAILURE;
	}

	if (have_config && !read_mesh_config()) {
		bt_shell_cleanup();
		return EXIT_FAILURE;
	}

	bt_shell_set_prompt(PROMPT_OFF);

	dbus = l_dbus_new_default(L_DBUS_SYSTEM_BUS);

	l_dbus_set_ready_handler(dbus, ready_callback, NULL, NULL);
	client = l_dbus_client_new(dbus, BLUEZ_MESH_NAME, "/org/bluez/mesh");

	l_dbus_client_set_connect_handler(client, client_connected, NULL, NULL);
	l_dbus_client_set_disconnect_handler(client, client_disconnected, NULL,
									NULL);
	l_dbus_client_set_proxy_handlers(client, proxy_added, proxy_removed,
						property_changed, NULL, NULL);
	l_dbus_client_set_ready_handler(client, client_ready, NULL, NULL);

	node_proxies = l_queue_new();
	devices = l_queue_new();

	status = bt_shell_run();

	l_dbus_client_destroy(client);
	l_dbus_destroy(dbus);

	cfgcli_cleanup();

	return status;
}
