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
#include <ell/ell.h>

#include "mesh/mesh-defs.h"
#include "mesh/dbus.h"
#include "mesh/error.h"
#include "mesh/mesh.h"
#include "mesh/mesh-io.h"
#include "mesh/node.h"
#include "mesh/net.h"
#include "mesh/keyring.h"
#include "mesh/agent.h"
#include "mesh/provision.h"
#include "mesh/manager.h"

struct add_data{
	struct l_dbus_message *msg;
	struct mesh_agent *agent;
	struct mesh_node *node;
	uint32_t disc_watch;
	uint16_t primary;
	uint16_t net_idx;
	uint8_t num_ele;
	uint8_t uuid[16];
};

static int8_t scan_rssi;
static uint8_t scan_uuid[16];
static struct mesh_node *scan_node;
static struct l_timeout *scan_timeout;
static struct add_data *add_pending;
static const uint8_t prvb[2] = {MESH_AD_TYPE_BEACON, 0x00};

static void scan_cancel(struct l_timeout *timeout, void *user_data)
{
	struct mesh_node *node = user_data;
	struct mesh_io *io;
	struct mesh_net *net;

	l_debug("");

	if (scan_timeout)
		l_timeout_remove(scan_timeout);

	net = node_get_net(node);
	io = mesh_net_get_io(net);
	mesh_io_deregister_recv_cb(io, prvb, sizeof(prvb));
	scan_node = NULL;
	scan_timeout = NULL;
}

static void free_pending_add_call()
{
	if (!add_pending)
		return;

	if (add_pending->disc_watch)
		l_dbus_remove_watch(dbus_get_bus(),
						add_pending->disc_watch);

	if (add_pending->msg)
		l_dbus_message_unref(add_pending->msg);

	l_free(add_pending);
	add_pending = NULL;
}

static void prov_disc_cb(struct l_dbus *bus, void *user_data)
{
	if (!add_pending)
		return;

	initiator_cancel(add_pending);
	add_pending->disc_watch = 0;

	free_pending_add_call();
}

static void send_add_failed(const char *owner, const char *path,
							uint8_t status)
{
	struct l_dbus *dbus = dbus_get_bus();
	struct l_dbus_message_builder *builder;
	struct l_dbus_message *msg;

	msg = l_dbus_message_new_method_call(dbus, owner, path,
						MESH_PROVISIONER_INTERFACE,
						"AddNodeFailed");

	builder = l_dbus_message_builder_new(msg);
	dbus_append_byte_array(builder, add_pending->uuid, 16);
	l_dbus_message_builder_append_basic(builder, 's',
						mesh_prov_status_str(status));
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
	l_dbus_send(dbus, msg);

	free_pending_add_call();
}

static bool add_cmplt(void *user_data, uint8_t status,
					struct mesh_prov_node_info *info)
{
	struct add_data *pending = user_data;
	struct mesh_node *node = pending->node;
	struct l_dbus *dbus = dbus_get_bus();
	struct l_dbus_message_builder *builder;
	struct l_dbus_message *msg;
	bool result;

	if (pending != add_pending)
		return false;

	if (status != PROV_ERR_SUCCESS) {
		send_add_failed(node_get_owner(node), node_get_app_path(node),
									status);
		return false;
	}

	result = keyring_put_remote_dev_key(add_pending->node, info->unicast,
					info->num_ele, info->device_key);

	if (!result) {
		send_add_failed(node_get_owner(node), node_get_app_path(node),
						PROV_ERR_CANT_ASSIGN_ADDR);
		return false;
	}

	msg = l_dbus_message_new_method_call(dbus, node_get_owner(node),
						node_get_app_path(node),
						MESH_PROVISIONER_INTERFACE,
						"AddNodeComplete");

	builder = l_dbus_message_builder_new(msg);
	dbus_append_byte_array(builder, add_pending->uuid, 16);
	l_dbus_message_builder_append_basic(builder, 'q', &info->unicast);
	l_dbus_message_builder_append_basic(builder, 'y', &info->num_ele);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_dbus_send(dbus, msg);

	free_pending_add_call();

	return true;
}

static void mgr_prov_data (struct l_dbus_message *reply, void *user_data)
{
	struct add_data *pending = user_data;
	uint16_t net_idx;
	uint16_t primary;

	if (pending != add_pending)
		return;

	if (l_dbus_message_is_error(reply))
		return;

	if (!l_dbus_message_get_arguments(reply, "qq", &net_idx, &primary))
		return;

	add_pending->primary = primary;
	add_pending->net_idx = net_idx;
	initiator_prov_data(net_idx, primary, add_pending);
}

static bool add_data_get(void *user_data, uint8_t num_ele)
{
	struct add_data *pending = user_data;
	struct l_dbus_message *msg;
	struct l_dbus *dbus;
	const char *app_path;
	const char *sender;

	if (pending != add_pending)
		return false;

	dbus = dbus_get_bus();
	app_path = node_get_app_path(add_pending->node);
	sender = node_get_owner(add_pending->node);

	msg = l_dbus_message_new_method_call(dbus, sender, app_path,
						MESH_PROVISIONER_INTERFACE,
						"RequestProvData");

	l_dbus_message_set_arguments(msg, "y", num_ele);
	l_dbus_send_with_reply(dbus, msg, mgr_prov_data, add_pending, NULL);

	add_pending->num_ele = num_ele;

	return true;
}

static void add_start(void *user_data, int err)
{
	struct l_dbus_message *reply;

	l_debug("Start callback");

	if (err == MESH_ERROR_NONE)
		reply = l_dbus_message_new_method_return(add_pending->msg);
	else
		reply = dbus_error(add_pending->msg, MESH_ERROR_FAILED,
				"Failed to start provisioning initiator");

	l_dbus_send(dbus_get_bus(), reply);
	l_dbus_message_unref(add_pending->msg);

	add_pending->msg = NULL;
}

static struct l_dbus_message *add_node_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	struct l_dbus_message_iter iter_uuid, options;
	struct l_dbus_message *reply;
	uint8_t *uuid;
	uint32_t n = 22;

	l_debug("AddNode request");

	if (!l_dbus_message_get_arguments(msg, "aya{sv}", &iter_uuid, &options))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!l_dbus_message_iter_get_fixed_array(&iter_uuid, &uuid, &n)
								|| n != 16)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Bad device UUID");

	/* Allow AddNode to cancel Scanning if from the same node */
	if (scan_node) {
		if (scan_node != node)
			return dbus_error(msg, MESH_ERROR_BUSY, NULL);

		scan_cancel(NULL, node);
	}

	/* Invoke Prov Initiator */
	add_pending = l_new(struct add_data, 1);
	memcpy(add_pending->uuid, uuid, 16);
	add_pending->node = node;
	add_pending->agent = node_get_agent(node);

	if (!node_is_provisioner(node) || (add_pending->agent == NULL)) {
		l_debug("Provisioner: %d", node_is_provisioner(node));
		l_debug("Agent: %p", add_pending->agent);
		reply = dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED,
							"Missing Interfaces");
		goto fail;
	}

	add_pending->msg = l_dbus_message_ref(msg);
	initiator_start(PB_ADV, uuid, 99, 60, add_pending->agent, add_start,
				add_data_get, add_cmplt, node, add_pending);

	add_pending->disc_watch = l_dbus_add_disconnect_watch(dbus,
						node_get_owner(node),
						prov_disc_cb, NULL, NULL);

	return NULL;
fail:
	l_free(add_pending);
	add_pending = NULL;
	return reply;
}


static struct l_dbus_message *import_node_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	struct l_dbus_message_iter iter_key;
	uint16_t primary;
	uint8_t num_ele;
	uint8_t *key;
	uint32_t n;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "qyay", &primary, &num_ele,
								&iter_key))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!l_dbus_message_iter_get_fixed_array(&iter_key, &key, &n)
								|| n != 16)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Bad device key");

	if (!keyring_put_remote_dev_key(node, primary, num_ele, key))
		return dbus_error(msg, MESH_ERROR_FAILED, NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *delete_node_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	struct mesh_net *net = node_get_net(node);
	uint16_t primary;
	uint8_t num_ele;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "qy", &primary, &num_ele))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (mesh_net_is_local_address(net, primary, num_ele))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
					"Cannot remove local device key");

	keyring_del_remote_dev_key(node, primary, num_ele);

	return l_dbus_message_new_method_return(msg);
}

static void prov_beacon_recv(void *user_data, struct mesh_io_recv_info *info,
					const uint8_t *data, uint16_t len)
{
	struct mesh_node *node = user_data;
	struct l_dbus_message_builder *builder;
	struct l_dbus_message *msg;
	struct l_dbus *dbus;
	int16_t rssi;

	if (scan_node != node || len < sizeof(scan_uuid) + 2 || data[1] != 0x00)
		return;

	if (!memcmp(data + 2, scan_uuid, sizeof(scan_uuid))) {
		if (info->rssi <= scan_rssi)
			return;
	}

	memcpy(scan_uuid, data + 2, sizeof(scan_uuid));
	scan_rssi = info->rssi;
	rssi = info->rssi;

	dbus = dbus_get_bus();
	msg = l_dbus_message_new_method_call(dbus, node_get_owner(node),
						node_get_app_path(node),
						MESH_PROVISIONER_INTERFACE,
						"ScanResult");

	builder = l_dbus_message_builder_new(msg);
	l_dbus_message_builder_append_basic(builder, 'n', &rssi);
	dbus_append_byte_array(builder, data + 2, len -2);
	l_dbus_message_builder_enter_array(builder, "{sv}");
	/* TODO: populate with options when defined */
	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_dbus_send(dbus, msg);
}

static struct l_dbus_message *start_scan_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	uint16_t duration = 0;
	struct mesh_io *io;
	struct mesh_net *net;
	const char *key;
	struct l_dbus_message_iter options, var;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "a{sv}", &options))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	while (l_dbus_message_iter_next_entry(&options, &key, &var)) {
		bool failed = true;

		if (!strcmp(key, "Seconds")) {
			if (l_dbus_message_iter_get_variant(&var, "q",
							    &duration)) {
				failed = false;
			}
		}

		if (failed)
			return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Invalid options");
	}

	if (scan_node && scan_node != node)
		return dbus_error(msg, MESH_ERROR_BUSY, NULL);

	if (!node_is_provisioner(node))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (scan_timeout)
		l_timeout_remove(scan_timeout);

	memset(scan_uuid, 0, sizeof(scan_uuid));
	scan_rssi = -128;
	scan_timeout = NULL;
	net = node_get_net(node);
	io = mesh_net_get_io(net);
	scan_node = node;
	mesh_io_register_recv_cb(io, prvb, sizeof(prvb),
						prov_beacon_recv, node);

	if (duration)
		scan_timeout = l_timeout_create(duration, scan_cancel,
								node, NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *cancel_scan_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)) || !node_is_provisioner(node))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (scan_node) {
		if (scan_node != node)
			return dbus_error(msg, MESH_ERROR_BUSY, NULL);

		scan_cancel(NULL, node);
	}

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *store_new_subnet(struct mesh_node *node,
					struct l_dbus_message *msg,
					uint16_t net_idx, uint8_t *new_key)
{
	struct keyring_net_key key;

	if (net_idx > MAX_KEY_IDX)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (keyring_get_net_key(node, net_idx, &key)) {
		/* Allow redundant calls only if key values match */
		if (!memcmp(key.old_key, new_key, 16))
			return l_dbus_message_new_method_return(msg);

		return dbus_error(msg, MESH_ERROR_ALREADY_EXISTS, NULL);
	}

	memcpy(key.old_key, new_key, 16);
	memcpy(key.new_key, new_key, 16);
	key.net_idx = net_idx;
	key.phase = KEY_REFRESH_PHASE_NONE;

	if (!keyring_put_net_key(node, net_idx, &key))
		return dbus_error(msg, MESH_ERROR_FAILED, NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *create_subnet_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	uint8_t key[16];
	uint16_t net_idx;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "q", &net_idx) ||
						net_idx == PRIMARY_NET_IDX)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	/* Generate key and store */
	l_getrandom(key, sizeof(key));

	return store_new_subnet(node, msg, net_idx, key);
}

static struct l_dbus_message *update_subnet_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	struct keyring_net_key key;
	uint16_t net_idx;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "q", &net_idx) ||
						net_idx > MAX_KEY_IDX)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!keyring_get_net_key(node, net_idx, &key))
		return dbus_error(msg, MESH_ERROR_DOES_NOT_EXIST, NULL);

	switch (key.phase) {
	case KEY_REFRESH_PHASE_NONE:
		/* Generate Key and update phase */
		l_getrandom(key.new_key, sizeof(key.new_key));
		key.phase = KEY_REFRESH_PHASE_ONE;

		if (!keyring_put_net_key(node, net_idx, &key))
			return dbus_error(msg, MESH_ERROR_FAILED, NULL);

		/* Fall Through */

	case KEY_REFRESH_PHASE_ONE:
		/* Allow redundant calls to start Key Refresh */
		return l_dbus_message_new_method_return(msg);

	default:
		break;
	}

	/* All other phases mean KR already in progress over-the-air */
	return dbus_error(msg, MESH_ERROR_IN_PROGRESS,
					"Key Refresh in progress");
}

static struct l_dbus_message *delete_subnet_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	uint16_t net_idx;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "q", &net_idx) ||
						net_idx > MAX_KEY_IDX)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	keyring_del_net_key(node, net_idx);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *import_subnet_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	struct l_dbus_message_iter iter_key;
	uint16_t net_idx;
	uint8_t *key;
	uint32_t n;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "qay", &net_idx, &iter_key))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!l_dbus_message_iter_get_fixed_array(&iter_key, &key, &n)
								|| n != 16)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Bad network key");

	return store_new_subnet(node, msg, net_idx, key);
}

static struct l_dbus_message *store_new_appkey(struct mesh_node *node,
					struct l_dbus_message *msg,
					uint16_t net_idx, uint16_t app_idx,
					uint8_t *new_key)
{
	struct keyring_net_key net_key;
	struct keyring_app_key app_key;

	if (net_idx > MAX_KEY_IDX || app_idx > MAX_KEY_IDX)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!keyring_get_net_key(node, net_idx, &net_key))
		return dbus_error(msg, MESH_ERROR_DOES_NOT_EXIST,
						"Bound net key not found");

	if (keyring_get_app_key(node, app_idx, &app_key)) {
		/* Allow redundant calls with identical values */
		if (!memcmp(app_key.old_key, new_key, 16) &&
						app_key.net_idx == net_idx)
			return l_dbus_message_new_method_return(msg);

		return dbus_error(msg, MESH_ERROR_ALREADY_EXISTS, NULL);
	}

	memcpy(app_key.old_key, new_key, 16);
	memcpy(app_key.new_key, new_key, 16);
	app_key.net_idx = net_idx;
	app_key.app_idx = app_idx;

	if (!keyring_put_app_key(node, app_idx, net_idx, &app_key))
		return dbus_error(msg, MESH_ERROR_FAILED, NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *create_appkey_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	uint16_t net_idx, app_idx;
	uint8_t key[16];
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "qq", &net_idx, &app_idx))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	l_getrandom(key, sizeof(key));

	return store_new_appkey(node, msg, net_idx, app_idx, key);
}

static struct l_dbus_message *update_appkey_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	struct keyring_net_key net_key;
	struct keyring_app_key app_key;
	uint16_t app_idx;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "q", &app_idx) ||
							app_idx > MAX_KEY_IDX)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!keyring_get_app_key(node, app_idx, &app_key) ||
			!keyring_get_net_key(node, app_key.net_idx, &net_key))
		return dbus_error(msg, MESH_ERROR_DOES_NOT_EXIST, NULL);

	if (net_key.phase != KEY_REFRESH_PHASE_ONE)
		return dbus_error(msg, MESH_ERROR_FAILED, "Invalid Phase");

	/* Generate Key if in acceptable phase */
	l_getrandom(app_key.new_key, sizeof(app_key.new_key));

	if (!keyring_put_app_key(node, app_idx, app_key.net_idx, &app_key))
		return dbus_error(msg, MESH_ERROR_FAILED, NULL);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *delete_appkey_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	uint16_t app_idx;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "q", &app_idx))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	keyring_del_app_key(node, app_idx);

	return l_dbus_message_new_method_return(msg);
}

static struct l_dbus_message *import_appkey_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	struct l_dbus_message_iter iter_key;
	uint16_t net_idx, app_idx;
	uint8_t *key;
	uint32_t n;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "qqay", &net_idx, &app_idx,
								&iter_key))
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!l_dbus_message_iter_get_fixed_array(&iter_key, &key, &n)
								|| n != 16)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS,
							"Bad application key");

	return store_new_appkey(node, msg, net_idx, app_idx, key);
}

static struct l_dbus_message *set_key_phase_call(struct l_dbus *dbus,
						struct l_dbus_message *msg,
						void *user_data)
{
	struct mesh_node *node = user_data;
	struct keyring_net_key key;
	uint16_t net_idx;
	uint8_t phase;
	const char *sender = l_dbus_message_get_sender(msg);

	if (strcmp(sender, node_get_owner(node)))
		return dbus_error(msg, MESH_ERROR_NOT_AUTHORIZED, NULL);

	if (!l_dbus_message_get_arguments(msg, "qy", &net_idx, &phase) ||
					phase == KEY_REFRESH_PHASE_ONE ||
					phase > KEY_REFRESH_PHASE_THREE)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (!keyring_get_net_key(node, net_idx, &key))
		return dbus_error(msg, MESH_ERROR_DOES_NOT_EXIST, NULL);

	/* Canceling Key Refresh only valid from Phase One */
	if (phase == KEY_REFRESH_PHASE_NONE &&
					key.phase >= KEY_REFRESH_PHASE_TWO)
		return dbus_error(msg, MESH_ERROR_INVALID_ARGS, NULL);

	if (phase == KEY_REFRESH_PHASE_THREE) {

		/* If we are already in Phase None, then nothing to do */
		if (key.phase == KEY_REFRESH_PHASE_NONE)
			return l_dbus_message_new_method_return(msg);

		memcpy(key.old_key, key.new_key, 16);
		key.phase = KEY_REFRESH_PHASE_THREE;

		if (!keyring_put_net_key(node, net_idx, &key))
			return dbus_error(msg, MESH_ERROR_FAILED, NULL);

		if (!keyring_finalize_app_keys(node, net_idx))
			return dbus_error(msg, MESH_ERROR_FAILED, NULL);

		key.phase = KEY_REFRESH_PHASE_NONE;
	} else
		key.phase = phase;

	if (!keyring_put_net_key(node, net_idx, &key))
		return dbus_error(msg, MESH_ERROR_FAILED, NULL);

	return l_dbus_message_new_method_return(msg);
}

static void setup_management_interface(struct l_dbus_interface *iface)
{
	l_dbus_interface_method(iface, "AddNode", 0, add_node_call, "",
						"aya{sv}", "uuid", "options");
	l_dbus_interface_method(iface, "ImportRemoteNode", 0, import_node_call,
				"", "qyay", "primary", "count", "dev_key");
	l_dbus_interface_method(iface, "DeleteRemoteNode", 0, delete_node_call,
						"", "qy", "primary", "count");
	l_dbus_interface_method(iface, "UnprovisionedScan", 0, start_scan_call,
							"", "a{sv}", "options");
	l_dbus_interface_method(iface, "UnprovisionedScanCancel", 0,
						cancel_scan_call, "", "");
	l_dbus_interface_method(iface, "CreateSubnet", 0, create_subnet_call,
							"", "q", "net_index");
	l_dbus_interface_method(iface, "UpdateSubnet", 0, update_subnet_call,
							"", "q", "net_index");
	l_dbus_interface_method(iface, "DeleteSubnet", 0, delete_subnet_call,
							"", "q", "net_index");
	l_dbus_interface_method(iface, "ImportSubnet", 0, import_subnet_call,
					"", "qay", "net_index", "net_key");
	l_dbus_interface_method(iface, "CreateAppKey", 0, create_appkey_call,
					"", "qq", "net_index", "app_index");
	l_dbus_interface_method(iface, "UpdateAppKey", 0, update_appkey_call,
							"", "q", "app_index");
	l_dbus_interface_method(iface, "DeleteAppKey", 0, delete_appkey_call,
							"", "q", "app_index");
	l_dbus_interface_method(iface, "ImportAppKey", 0, import_appkey_call,
							"", "qqay", "net_index",
							"app_index", "app_key");
	l_dbus_interface_method(iface, "SetKeyPhase", 0, set_key_phase_call, "",
						"qy", "net_index", "phase");
}

bool manager_dbus_init(struct l_dbus *bus)
{
	if (!l_dbus_register_interface(bus, MESH_MANAGEMENT_INTERFACE,
						setup_management_interface,
						NULL, false)) {
		l_info("Unable to register %s interface",
						MESH_MANAGEMENT_INTERFACE);
		return false;
	}

	return true;
}

void manager_scan_cancel(struct mesh_node *node)
{
	if (scan_node != node)
		return;

	scan_cancel(NULL, node);
}
