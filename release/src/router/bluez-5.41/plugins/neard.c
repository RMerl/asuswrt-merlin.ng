/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2013  Tieto Poland
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <errno.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/sdp.h"

#include "gdbus/gdbus.h"

#include "src/plugin.h"
#include "src/log.h"
#include "src/dbus-common.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/eir.h"
#include "src/agent.h"
#include "src/hcid.h"

#define NEARD_NAME "org.neard"
#define NEARD_PATH "/"
#define NEARD_MANAGER_INTERFACE "org.neard.Manager"
#define AGENT_INTERFACE "org.neard.HandoverAgent"
#define AGENT_PATH "/org/bluez/neard_handover_agent"
#define AGENT_CARRIER_TYPE "bluetooth"
#define ERROR_INTERFACE "org.neard.HandoverAgent.Error"

static guint watcher_id = 0;
static char *neard_service = NULL;
static bool agent_register_postpone = false;

/* For NFC mimetype limits max OOB EIR size */
#define NFC_OOB_EIR_MAX UINT8_MAX

enum cps {
	CPS_ACTIVE,
	CPS_INACTIVE,
	CPS_ACTIVATING,
	CPS_UNKNOWN,
};

struct oob_params {
	bdaddr_t address;
	uint32_t class;
	char *name;
	GSList *services;
	uint8_t *hash;
	uint8_t *randomizer;
	uint8_t *pin;
	int pin_len;
	enum cps power_state;
};

static void free_oob_params(struct oob_params *params)
{
	g_slist_free_full(params->services, free);
	g_free(params->name);
	g_free(params->hash);
	g_free(params->randomizer);
	g_free(params->pin);
}

static DBusMessage *error_reply(DBusMessage *msg, int error)
{
	const char *name;

	if (error == EINPROGRESS)
		name = ERROR_INTERFACE ".InProgress";
	else
		name = ERROR_INTERFACE ".Failed";

	return g_dbus_create_error(msg, name , "%s", strerror(error));
}

static void register_agent(bool append_carrier);

static void register_agent_cb(DBusPendingCall *call, void *user_data)
{
	DBusMessage *reply;
	DBusError err;
	static bool try_fallback = true;

	reply = dbus_pending_call_steal_reply(call);

	dbus_error_init(&err);
	if (dbus_set_error_from_message(&err, reply)) {
		if (g_str_equal(DBUS_ERROR_UNKNOWN_METHOD, err.name) &&
				try_fallback) {
			DBG("Register to neard failed, trying legacy way");

			register_agent(false);
			try_fallback = false;
		} else {
			error("neard manager replied with an error: %s, %s",
							err.name, err.message);

			g_dbus_unregister_interface(btd_get_dbus_connection(),
						AGENT_PATH, AGENT_INTERFACE);
			try_fallback = true;
		}

		dbus_error_free(&err);
		dbus_message_unref(reply);

		return;
	}

	dbus_message_unref(reply);
	neard_service = g_strdup(dbus_message_get_sender(reply));

	try_fallback = true;

	info("Registered as neard handover agent");
}

static void register_agent(bool append_carrier)
{
	DBusMessage *message;
	DBusPendingCall *call;
	const char *path = AGENT_PATH;
	const char *carrier = AGENT_CARRIER_TYPE;

	message = dbus_message_new_method_call(NEARD_NAME, NEARD_PATH,
			NEARD_MANAGER_INTERFACE, "RegisterHandoverAgent");
	if (!message) {
		error("Couldn't allocate D-Bus message");
		return;
	}

	dbus_message_append_args(message, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID);

	if (append_carrier)
		dbus_message_append_args(message, DBUS_TYPE_STRING, &carrier,
							DBUS_TYPE_INVALID);

	if (!g_dbus_send_message_with_reply(btd_get_dbus_connection(),
							message, &call, -1)) {
		dbus_message_unref(message);
		error("D-Bus send failed");
		return;
	}

	dbus_pending_call_set_notify(call, register_agent_cb, NULL, NULL);
	dbus_pending_call_unref(call);

	dbus_message_unref(message);
}

static void unregister_agent(void)
{
	DBusMessage *message;
	const char *path = AGENT_PATH;
	const char *carrier = AGENT_CARRIER_TYPE;

	g_free(neard_service);
	neard_service = NULL;

	message = dbus_message_new_method_call(NEARD_NAME, NEARD_PATH,
			NEARD_MANAGER_INTERFACE, "UnregisterHandoverAgent");

	if (!message) {
		error("Couldn't allocate D-Bus message");
		goto unregister;
	}

	dbus_message_append_args(message, DBUS_TYPE_OBJECT_PATH, &path,
						DBUS_TYPE_INVALID);

	dbus_message_append_args(message, DBUS_TYPE_STRING, &carrier,
							DBUS_TYPE_INVALID);

	if (!g_dbus_send_message(btd_get_dbus_connection(), message))
		error("D-Bus send failed");

unregister:
	g_dbus_unregister_interface(btd_get_dbus_connection(), AGENT_PATH,
							AGENT_INTERFACE);
}

static void add_power_state(DBusMessageIter *dict, struct btd_adapter *adapter)
{
	const char *state;

	if (btd_adapter_get_powered(adapter) &&
					btd_adapter_get_connectable(adapter))
		state = "active";
	else
		state = "inactive";

	dict_append_entry(dict, "State", DBUS_TYPE_STRING, &state);
}

static DBusMessage *create_request_oob_reply(struct btd_adapter *adapter,
						const uint8_t *hash,
						const uint8_t *randomizer,
						DBusMessage *msg)
{
	DBusMessage *reply;
	DBusMessageIter iter;
	DBusMessageIter dict;
	uint8_t eir[NFC_OOB_EIR_MAX];
	uint8_t *peir = eir;
	int len;

	len = eir_create_oob(btd_adapter_get_address(adapter),
				btd_adapter_get_name(adapter),
				btd_adapter_get_class(adapter), hash,
				randomizer, main_opts.did_vendor,
				main_opts.did_product, main_opts.did_version,
				main_opts.did_source,
				btd_adapter_get_services(adapter), eir);

	reply = dbus_message_new_method_return(msg);
	if (!reply)
		return NULL;

	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
				DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
				DBUS_TYPE_STRING_AS_STRING
				DBUS_TYPE_VARIANT_AS_STRING
				DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
				&dict);

	dict_append_array(&dict, "EIR", DBUS_TYPE_BYTE, &peir, len);

	add_power_state(&dict, adapter);

	dbus_message_iter_close_container(&iter, &dict);

	return reply;
}

static void read_local_complete(struct btd_adapter *adapter,
				const uint8_t *hash, const uint8_t *randomizer,
				void *user_data)
{
	DBusMessage *msg = user_data;
	DBusMessage *reply;

	DBG("");

	if (neard_service == NULL) {
		dbus_message_unref(msg);

		if (agent_register_postpone) {
			agent_register_postpone = false;
			register_agent(true);
		}

		return;
	}

	if (hash && randomizer)
		reply = create_request_oob_reply(adapter, hash, randomizer,
									msg);
	else
		reply = error_reply(msg, EIO);

	dbus_message_unref(msg);

	if (!g_dbus_send_message(btd_get_dbus_connection(), reply))
		error("D-Bus send failed");
}

static void bonding_complete(struct btd_adapter *adapter,
					const bdaddr_t *bdaddr, uint8_t status,
					void *user_data)
{
	DBusMessage *msg = user_data;
	DBusMessage *reply;

	DBG("");

	if (neard_service == NULL) {
		dbus_message_unref(msg);

		if (agent_register_postpone) {
			agent_register_postpone = false;
			register_agent(true);
		}

		return;
	}

	if (status)
		reply = error_reply(msg, EIO);
	else
		reply = g_dbus_create_reply(msg, DBUS_TYPE_INVALID);

	dbus_message_unref(msg);

	if (!g_dbus_send_message(btd_get_dbus_connection(), reply))
		error("D-Bus send failed");
}

static int check_device(struct btd_device *device)
{
	if (!device)
		return -ENOENT;

	/* If already paired */
	if (device_is_paired(device, BDADDR_BREDR)) {
		DBG("already paired");
		return -EALREADY;
	}

	/* Pairing in progress... */
	if (device_is_bonding(device, NULL)) {
		DBG("pairing in progress");
		return -EINPROGRESS;
	}

	return 0;
}

static int process_eir(uint8_t *eir, size_t size, struct oob_params *remote)
{
	struct eir_data eir_data;

	DBG("size %zu", size);

	memset(&eir_data, 0, sizeof(eir_data));

	if (eir_parse_oob(&eir_data, eir, size) < 0)
		return -EINVAL;

	bacpy(&remote->address, &eir_data.addr);

	remote->class = eir_data.class;

	remote->name = eir_data.name;
	eir_data.name = NULL;

	remote->services = eir_data.services;
	eir_data.services = NULL;

	remote->hash = eir_data.hash;
	eir_data.hash = NULL;

	remote->randomizer = eir_data.randomizer;
	eir_data.randomizer = NULL;

	eir_data_free(&eir_data);

	return 0;
}

/*
 * This is (barely documented) Nokia extension format, most work was done by
 * reverse engineering.
 *
 * Binary format varies among different devices, type depends on first byte
 * 0x00 - BT address not reversed, 16 bytes authentication data (all zeros)
 * 0x01 - BT address not reversed, 16 bytes authentication data (4 digit PIN,
 *        padded with zeros)
 * 0x02 - BT address not reversed, 16 bytes authentication data (not sure if
 *        16 digit PIN or link key?, Nokia refers to it as ' Public Key')
 * 0x10 - BT address reversed, no authentication data
 * 0x24 - BT address not reversed, 4 bytes authentication data (4 digit PIN)
 *
 * General structure:
 * 1 byte  - marker
 * 6 bytes - BT address (reversed or not, depends on marker)
 * 3 bytes - Class of Device
 * 0, 4 or 16 bytes - authentication data, interpretation depends on marker
 * 1 bytes - name length
 * N bytes - name
 */

static int process_nokia_long (void *data, size_t size, uint8_t marker,
						struct oob_params *remote)
{
	struct {
		bdaddr_t address;
		uint8_t class[3];
		uint8_t authentication[16];
		uint8_t name_len;
		uint8_t name[0];
	} __attribute__((packed)) *n = data;

	if (size != sizeof(*n) + n->name_len)
		return -EINVAL;

	/* address is not reverted */
	baswap(&remote->address, &n->address);

	remote->class = n->class[0] | (n->class[1] << 8) | (n->class[2] << 16);

	if (n->name_len > 0)
		remote->name = g_strndup((char *)n->name, n->name_len);

	if (marker == 0x01) {
		remote->pin = g_memdup(n->authentication, 4);
		remote->pin_len = 4;
	} else if (marker == 0x02) {
		remote->pin = g_memdup(n->authentication, 16);
		remote->pin_len = 16;
	}

	return 0;
}

static int process_nokia_short (void *data, size_t size,
						struct oob_params *remote)
{
	struct {
		bdaddr_t address;
		uint8_t class[3];
		uint8_t authentication[4];
		uint8_t name_len;
		uint8_t name[0];
	} __attribute__((packed)) *n = data;

	if (size != sizeof(*n) + n->name_len)
		return -EINVAL;

	/* address is not reverted */
	baswap(&remote->address, &n->address);

	remote->class = n->class[0] | (n->class[1] << 8) | (n->class[2] << 16);

	if (n->name_len > 0)
		remote->name = g_strndup((char *)n->name, n->name_len);

	remote->pin = g_memdup(n->authentication, 4);
	remote->pin_len = 4;

	return 0;
}

static int process_nokia_extra_short (void *data, size_t size,
						struct oob_params *remote)
{
	struct {
		bdaddr_t address;
		uint8_t class[3];
		uint8_t name_len;
		uint8_t name[0];
	} __attribute__((packed)) *n = data;

	if (size != sizeof(*n) + n->name_len)
		return -EINVAL;

	bacpy(&remote->address, &n->address);

	remote->class = n->class[0] | (n->class[1] << 8) | (n->class[2] << 16);

	if (n->name_len > 0)
		remote->name = g_strndup((char *)n->name, n->name_len);

	return 0;
}

static int process_nokia_com_bt(uint8_t *data, size_t size,
						struct oob_params *remote)
{
	uint8_t marker;

	marker = *data++;
	size--;

	DBG("marker: 0x%.2x  size: %zu", marker, size);

	switch (marker) {
	case 0x00:
	case 0x01:
	case 0x02:
		return process_nokia_long(data, size, marker, remote);
	case 0x10:
		return process_nokia_extra_short(data, size, remote);
	case 0x24:
		return process_nokia_short(data, size, remote);
	default:
		warn("Not supported Nokia NFC extension (0x%.2x)", marker);
		return -EPROTONOSUPPORT;
	}
}

static enum cps process_state(const char *state)
{
	if (strcasecmp(state, "active") == 0)
		return CPS_ACTIVE;

	if (strcasecmp(state, "activating") == 0)
		return CPS_ACTIVATING;

	if (strcasecmp(state, "inactive") == 0)
		return CPS_INACTIVE;

	return CPS_UNKNOWN;
}

static int process_message(DBusMessage *msg, struct oob_params *remote)
{
	DBusMessageIter iter;
	DBusMessageIter dict;

	dbus_message_iter_init(msg, &iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return -EINVAL;

	/* set CPS to unknown in case State was not provided */
	remote->power_state = CPS_UNKNOWN;

	dbus_message_iter_recurse(&iter, &dict);

	while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter value;
		DBusMessageIter entry;
		const char *key;

		dbus_message_iter_recurse(&dict, &entry);

		if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_STRING)
			goto error;

		dbus_message_iter_get_basic(&entry, &key);
		dbus_message_iter_next(&entry);

		dbus_message_iter_recurse(&entry, &value);

		if (strcasecmp(key, "EIR") == 0) {
			DBusMessageIter array;
			uint8_t *eir;
			int size;

			/* nokia.com:bt and EIR should not be passed together */
			if (bacmp(&remote->address, BDADDR_ANY) != 0)
				goto error;

			if (dbus_message_iter_get_arg_type(&value) !=
					DBUS_TYPE_ARRAY)
				goto error;

			dbus_message_iter_recurse(&value, &array);
			dbus_message_iter_get_fixed_array(&array, &eir, &size);

			if (process_eir(eir, size, remote) < 0)
				goto error;
		} else if (strcasecmp(key, "nokia.com:bt") == 0) {
			DBusMessageIter array;
			uint8_t *data;
			int size;

			/* nokia.com:bt and EIR should not be passed together */
			if (bacmp(&remote->address, BDADDR_ANY) != 0)
				goto error;

			if (dbus_message_iter_get_arg_type(&value) !=
					DBUS_TYPE_ARRAY)
				goto error;

			dbus_message_iter_recurse(&value, &array);
			dbus_message_iter_get_fixed_array(&array, &data, &size);

			if (process_nokia_com_bt(data, size, remote))
				goto error;
		} else if (strcasecmp(key, "State") == 0) {
			DBusMessageIter array;
			const char *state;

			if (dbus_message_iter_get_arg_type(&value) !=
					DBUS_TYPE_STRING)
				goto error;

			dbus_message_iter_recurse(&value, &array);
			dbus_message_iter_get_basic(&value, &state);

			remote->power_state = process_state(state);
			if (remote->power_state == CPS_UNKNOWN)
				goto error;
		}

		dbus_message_iter_next(&dict);
	}

	/* Check if 'State' was passed along with one of other fields */
	if (remote->power_state != CPS_UNKNOWN
			&& bacmp(&remote->address, BDADDR_ANY) == 0)
		return -EINVAL;

	return 0;

error:
	if (bacmp(&remote->address, BDADDR_ANY) != 0) {
		free_oob_params(remote);
		memset(remote, 0, sizeof(*remote));
	}

	return -EINVAL;
}

static int check_adapter(struct btd_adapter *adapter)
{
	if (!adapter)
		return -ENOENT;

	if (btd_adapter_check_oob_handler(adapter))
		return -EINPROGRESS;

	if (!btd_adapter_ssp_enabled(adapter))
		return -ENOTSUP;

	return 0;
}

static void store_params(struct btd_adapter *adapter, struct btd_device *device,
						struct oob_params *params)
{
	if (params->class != 0)
		device_set_class(device, params->class);

	if (params->name) {
		device_store_cached_name(device, params->name);
		btd_device_device_set_name(device, params->name);
	}

	if (params->services)
		device_add_eir_uuids(device, params->services);

	if (params->hash) {
		btd_adapter_add_remote_oob_data(adapter, &params->address,
							params->hash,
							params->randomizer);
	} else if (params->pin_len) {
		/* TODO
		 * Handle PIN, for now only discovery mode and 'common' PINs
		 * that might be provided by agent will work correctly.
		 */
	}
}

static DBusMessage *push_oob(DBusConnection *conn, DBusMessage *msg, void *data)
{
	struct btd_adapter *adapter;
	struct agent *agent;
	struct oob_handler *handler;
	struct oob_params remote;
	struct btd_device *device;
	uint8_t io_cap;
	int err;

	if (neard_service == NULL ||
			!g_str_equal(neard_service, dbus_message_get_sender(msg)))
		return error_reply(msg, EPERM);

	DBG("");

	adapter = btd_adapter_get_default();

	err = check_adapter(adapter);
	if (err < 0)
		return error_reply(msg, -err);

	if (!btd_adapter_get_powered(adapter))
		return error_reply(msg, ENONET);

	agent = adapter_get_agent(adapter);
	if (!agent)
		return error_reply(msg, ENONET);

	io_cap = agent_get_io_capability(agent);
	agent_unref(agent);

	memset(&remote, 0, sizeof(remote));

	err = process_message(msg, &remote);
	if (err < 0)
		return error_reply(msg, -err);

	if (bacmp(&remote.address, BDADDR_ANY) == 0) {
		free_oob_params(&remote);

		return error_reply(msg, EINVAL);
	}

	device = btd_adapter_get_device(adapter, &remote.address,
								BDADDR_BREDR);

	err = check_device(device);
	if (err < 0) {
		free_oob_params(&remote);

		/* already paired, reply immediately */
		if (err == -EALREADY)
			return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);

		return error_reply(msg, -err);
	}

	if (!btd_adapter_get_pairable(adapter)) {
		free_oob_params(&remote);

		return error_reply(msg, ENONET);
	}

	store_params(adapter, device, &remote);

	free_oob_params(&remote);

	err = adapter_create_bonding(adapter, device_get_address(device),
							BDADDR_BREDR, io_cap);
	if (err < 0)
		return error_reply(msg, -err);

	handler = g_new0(struct oob_handler, 1);
	handler->bonding_cb = bonding_complete;
	bacpy(&handler->remote_addr, device_get_address(device));
	handler->user_data = dbus_message_ref(msg);

	btd_adapter_set_oob_handler(adapter, handler);

	return NULL;
}

static DBusMessage *request_oob(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	struct btd_adapter *adapter;
	struct oob_handler *handler;
	struct oob_params remote;
	struct btd_device *device;
	int err;

	if (neard_service == NULL ||
			!g_str_equal(neard_service, dbus_message_get_sender(msg)))
		return error_reply(msg, EPERM);

	DBG("");

	adapter = btd_adapter_get_default();

	err = check_adapter(adapter);
	if (err < 0)
		return error_reply(msg, -err);

	memset(&remote, 0, sizeof(remote));

	err = process_message(msg, &remote);
	if (err < 0)
		return error_reply(msg, -err);

	if (bacmp(&remote.address, BDADDR_ANY) == 0) {
		if (btd_adapter_get_powered(adapter))
			goto read_local;

		goto done;
	}

	device = btd_adapter_get_device(adapter, &remote.address, BDADDR_BREDR);

	err = check_device(device);
	if (err < 0)
		goto done;

	if (!btd_adapter_get_pairable(adapter)) {
		err = -ENONET;
		goto done;
	}

	store_params(adapter, device, &remote);

	if (remote.hash && btd_adapter_get_powered(adapter))
		goto read_local;
done:
	free_oob_params(&remote);

	if (err < 0 && err != -EALREADY)
		return error_reply(msg, -err);

	return create_request_oob_reply(adapter, NULL, NULL, msg);

read_local:
	free_oob_params(&remote);

	err = btd_adapter_read_local_oob_data(adapter);
	if (err < 0)
		return error_reply(msg, -err);

	handler = g_new0(struct oob_handler, 1);
	handler->read_local_cb = read_local_complete;
	handler->user_data = dbus_message_ref(msg);

	btd_adapter_set_oob_handler(adapter, handler);

	return NULL;
}

static DBusMessage *release(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	if (neard_service == NULL ||
			!g_str_equal(neard_service, dbus_message_get_sender(msg)))
		return error_reply(msg, EPERM);

	DBG("");

	g_free(neard_service);
	neard_service = NULL;

	g_dbus_unregister_interface(conn, AGENT_PATH, AGENT_INTERFACE);

	return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
}

static const GDBusMethodTable neard_methods[] = {
	{ GDBUS_ASYNC_METHOD("RequestOOB",
			GDBUS_ARGS({ "data", "a{sv}" }),
			GDBUS_ARGS({ "data", "a{sv}" }), request_oob) },
	{ GDBUS_ASYNC_METHOD("PushOOB",
			GDBUS_ARGS({ "data", "a{sv}"}), NULL, push_oob) },
	{ GDBUS_METHOD("Release", NULL, NULL, release) },
	{ }
};

static void neard_appeared(DBusConnection *conn, void *user_data)
{
	struct btd_adapter *adapter;

	DBG("");

	if (!g_dbus_register_interface(conn, AGENT_PATH, AGENT_INTERFACE,
						neard_methods,
						NULL, NULL, NULL, NULL)) {
		error("neard interface init failed on path " AGENT_PATH);
		return;
	}

	/*
	 * If there is pending action ongoing when neard appeared, possibly
	 * due to neard crash or release before action was completed, postpone
	 * register until action is finished.
	 */
	adapter = btd_adapter_get_default();

	if (adapter && btd_adapter_check_oob_handler(adapter))
		agent_register_postpone = true;
	else
		register_agent(true);
}

static void neard_vanished(DBusConnection *conn, void *user_data)
{
	DBG("");

	/* neard existed without unregistering agent */
	if (neard_service != NULL) {
		g_free(neard_service);
		neard_service = NULL;

		g_dbus_unregister_interface(conn, AGENT_PATH, AGENT_INTERFACE);
	}
}

static int neard_init(void)
{
	DBG("Setup neard plugin");

	watcher_id = g_dbus_add_service_watch(btd_get_dbus_connection(),
						NEARD_NAME, neard_appeared,
						neard_vanished, NULL, NULL);
	if (watcher_id == 0)
		return -ENOMEM;

	return 0;
}

static void neard_exit(void)
{
	DBG("Cleanup neard plugin");

	g_dbus_remove_watch(btd_get_dbus_connection(), watcher_id);
	watcher_id = 0;

	if (neard_service != NULL)
		unregister_agent();
}

BLUETOOTH_PLUGIN_DEFINE(neard, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
						neard_init, neard_exit)
