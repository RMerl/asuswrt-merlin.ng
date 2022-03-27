/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "btio/btio.h"
#include "sdpd.h"
#include "log.h"
#include "error.h"
#include "uuid-helper.h"
#include "dbus-common.h"
#include "sdp-client.h"
#include "sdp-xml.h"
#include "adapter.h"
#include "device.h"
#include "profile.h"
#include "service.h"

#define DUN_DEFAULT_CHANNEL	1
#define SPP_DEFAULT_CHANNEL	3
#define HFP_HF_DEFAULT_CHANNEL	7
#define OPP_DEFAULT_CHANNEL	9
#define FTP_DEFAULT_CHANNEL	10
#define BIP_DEFAULT_CHANNEL	11
#define HSP_AG_DEFAULT_CHANNEL	12
#define HFP_AG_DEFAULT_CHANNEL	13
#define SYNC_DEFAULT_CHANNEL	14
#define PBAP_DEFAULT_CHANNEL	15
#define MAS_DEFAULT_CHANNEL	16
#define MNS_DEFAULT_CHANNEL	17

#define BTD_PROFILE_PSM_AUTO	-1
#define BTD_PROFILE_CHAN_AUTO	-1

#define HFP_HF_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x111e\" />		\
				<uuid value=\"0x1203\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\" />	\
					<uint8 value=\"0x%02x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x111e\" />	\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\" />				\
		</attribute>						\
		<attribute id=\"0x0311\">				\
			<uint16 value=\"0x%04x\" />			\
		</attribute>						\
	</record>"

#define HFP_AG_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x111f\" />		\
				<uuid value=\"0x1203\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\" />	\
					<uint8 value=\"0x%02x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x111e\" />	\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\" />				\
		</attribute>						\
		<attribute id=\"0x0311\">				\
			<uint16 value=\"0x%04x\" />			\
		</attribute>						\
		<attribute id=\"0x0301\" >				\
			<uint8 value=\"0x01\" />			\
		</attribute>						\
	</record>"

#define HSP_AG_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x1112\" />		\
				<uuid value=\"0x1203\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\" />	\
					<uint8 value=\"0x%02x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1108\" />	\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\" />				\
		</attribute>						\
	</record>"

#define SPP_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x1101\" />		\
				%s					\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\" />	\
					<uint8 value=\"0x%02x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1101\" />	\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\" />				\
		</attribute>						\
	</record>"

#define DUN_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x1103\" />		\
				<uuid value=\"0x1201\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\" />	\
					<uint8 value=\"0x%02x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1103\" />	\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\" />				\
		</attribute>						\
	</record>"

#define OPP_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x1105\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\" />	\
					<uint8 value=\"0x%02x\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0008\"/>	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1105\" />	\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0303\">				\
			<sequence>					\
				<uint8 value=\"0x01\"/>			\
				<uint8 value=\"0x02\"/>			\
				<uint8 value=\"0x03\"/>			\
				<uint8 value=\"0x04\"/>			\
				<uint8 value=\"0x05\"/>			\
				<uint8 value=\"0x06\"/>			\
				<uint8 value=\"0xff\"/>			\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0200\">				\
			<uint16 value=\"%u\" name=\"psm\"/>		\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\" />				\
		</attribute>						\
	</record>"

#define FTP_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x1106\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\" />	\
					<uint8 value=\"0x%02x\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0008\"/>	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1106\" />	\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0200\">				\
			<uint16 value=\"%u\" name=\"psm\"/>		\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\" />				\
		</attribute>						\
	</record>"

#define PCE_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x112e\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1130\" />	\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\" />				\
		</attribute>						\
	</record>"

#define PSE_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x112f\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\" />	\
					<uint8 value=\"0x%02x\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0008\"/>	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1130\" />	\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\" />				\
		</attribute>						\
		<attribute id=\"0x0314\">				\
			<uint8 value=\"0x01\"/>				\
		</attribute>						\
		<attribute id=\"0x0317\">				\
			<uint32 value=\"0x00000003\"/>			\
		</attribute>						\
		<attribute id=\"0x0200\">				\
			<uint16 value=\"%u\" name=\"psm\"/>		\
		</attribute>						\
	</record>"

#define MAS_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x1132\"/>		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\"/>	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\"/>	\
					<uint8 value=\"0x%02x\"/>	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0008\"/>	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1134\"/>	\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\"/>				\
		</attribute>						\
		<attribute id=\"0x0315\">				\
			<uint8 value=\"0x00\"/>				\
		</attribute>						\
		<attribute id=\"0x0316\">				\
			<uint8 value=\"0x0F\"/>				\
		</attribute>						\
		<attribute id=\"0x0317\">				\
			<uint32 value=\"0x0000007f\"/>			\
		</attribute>						\
		<attribute id=\"0x0200\">				\
			<uint16 value=\"%u\" name=\"psm\"/>		\
		</attribute>						\
	</record>"

#define MNS_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x1133\"/>		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\"/>	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\"/>	\
					<uint8 value=\"0x%02x\"/>	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0008\"/>	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1134\"/>	\
					<uint16 value=\"0x%04x\"/>	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\"/>				\
		</attribute>						\
		<attribute id=\"0x0317\">				\
			<uint32 value=\"0x0000007f\"/>			\
		</attribute>						\
		<attribute id=\"0x0200\">				\
			<uint16 value=\"%u\" name=\"psm\"/>		\
		</attribute>						\
	</record>"

#define SYNC_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"0x1104\"/>		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\"/>	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\"/>	\
					<uint8 value=\"0x%02x\"/>	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0008\"/>	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1104\"/>	\
					<uint16 value=\"0x%04x\" />	\
				 </sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\"/>				\
		</attribute>						\
		<attribute id=\"0x0301\">				\
			<sequence>					\
				<uint8 value=\"0x01\"/>			\
			</sequence>					\
		</attribute>						\
	</record>"

#define GENERIC_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"%s\" />			\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\" />	\
					%s				\
				</sequence>				\
				%s					\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		%s							\
		<attribute id=\"0x0100\">				\
			<text value=\"%s\" />				\
		</attribute>						\
	</record>"

struct ext_io;

struct ext_profile {
	struct btd_profile p;

	char *name;
	char *owner;
	char *path;
	char *uuid;
	char *service;
	char *role;

	char *record;
	char *(*get_record)(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm);

	char *remote_uuid;

	guint id;

	BtIOMode mode;
	BtIOSecLevel sec_level;
	bool authorize;

	bool enable_client;
	bool enable_server;

	int local_psm;
	int local_chan;

	uint16_t remote_psm;
	uint8_t remote_chan;

	uint16_t version;
	uint16_t features;

	GSList *records;
	GSList *servers;
	GSList *conns;

	GSList *connects;
};

struct ext_io {
	struct ext_profile *ext;
	int proto;
	GIOChannel *io;
	guint io_id;
	struct btd_adapter *adapter;
	struct btd_device *device;
	struct btd_service *service;

	bool resolving;
	bool connected;

	uint16_t version;
	uint16_t features;

	uint16_t psm;
	uint8_t chan;

	guint auth_id;
	DBusPendingCall *pending;
};

struct ext_record {
	struct btd_adapter *adapter;
	uint32_t handle;
};

struct btd_profile_custom_property {
	char *uuid;
	char *type;
	char *name;
	btd_profile_prop_exists exists;
	btd_profile_prop_get get;
	void *user_data;
};

static GSList *custom_props = NULL;

static GSList *profiles = NULL;
static GSList *ext_profiles = NULL;

void btd_profile_foreach(void (*func)(struct btd_profile *p, void *data),
								void *data)
{
	GSList *l, *next;

	for (l = profiles; l != NULL; l = next) {
		struct btd_profile *profile = l->data;

		next = g_slist_next(l);

		func(profile, data);
	}

	for (l = ext_profiles; l != NULL; l = next) {
		struct ext_profile *profile = l->data;

		next = g_slist_next(l);

		func(&profile->p, data);
	}
}

int btd_profile_register(struct btd_profile *profile)
{
	profiles = g_slist_append(profiles, profile);
	return 0;
}

void btd_profile_unregister(struct btd_profile *profile)
{
	profiles = g_slist_remove(profiles, profile);
}

static struct ext_profile *find_ext_profile(const char *owner,
						const char *path)
{
	GSList *l;

	for (l = ext_profiles; l != NULL; l = g_slist_next(l)) {
		struct ext_profile *ext = l->data;

		if (g_strcmp0(ext->owner, owner))
			continue;

		if (!g_strcmp0(ext->path, path))
			return ext;
	}

	return NULL;
}

static void ext_io_destroy(gpointer p)
{
	struct ext_io *ext_io = p;

	if (ext_io->io_id > 0)
		g_source_remove(ext_io->io_id);

	if (ext_io->io) {
		g_io_channel_shutdown(ext_io->io, FALSE, NULL);
		g_io_channel_unref(ext_io->io);
	}

	if (ext_io->auth_id != 0)
		btd_cancel_authorization(ext_io->auth_id);

	if (ext_io->pending) {
		dbus_pending_call_cancel(ext_io->pending);
		dbus_pending_call_unref(ext_io->pending);
	}

	if (ext_io->resolving)
		bt_cancel_discovery(btd_adapter_get_address(ext_io->adapter),
					device_get_address(ext_io->device));

	if (ext_io->adapter)
		btd_adapter_unref(ext_io->adapter);

	if (ext_io->device)
		btd_device_unref(ext_io->device);

	if (ext_io->service)
		btd_service_unref(ext_io->service);

	g_free(ext_io);
}

static gboolean ext_io_disconnected(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct ext_io *conn = user_data;
	struct ext_profile *ext = conn->ext;
	GError *gerr = NULL;
	char addr[18];

	if (cond & G_IO_NVAL)
		return FALSE;

	bt_io_get(io, &gerr, BT_IO_OPT_DEST, addr, BT_IO_OPT_INVALID);
	if (gerr != NULL) {
		error("Unable to get io data for %s: %s",
						ext->name, gerr->message);
		g_error_free(gerr);
		goto drop;
	}

	DBG("%s disconnected from %s", ext->name, addr);
drop:
	if (conn->service) {
		if (btd_service_get_state(conn->service) ==
						BTD_SERVICE_STATE_CONNECTING)
			btd_service_connecting_complete(conn->service, -EIO);
		else
			btd_service_disconnecting_complete(conn->service, 0);
	}

	ext->conns = g_slist_remove(ext->conns, conn);
	ext_io_destroy(conn);
	return FALSE;
}

static void new_conn_reply(DBusPendingCall *call, void *user_data)
{
	struct ext_io *conn = user_data;
	struct ext_profile *ext = conn->ext;
	DBusMessage *reply = dbus_pending_call_steal_reply(call);
	DBusError err;

	dbus_error_init(&err);
	dbus_set_error_from_message(&err, reply);

	dbus_message_unref(reply);

	dbus_pending_call_unref(conn->pending);
	conn->pending = NULL;

	if (!dbus_error_is_set(&err)) {
		if (conn->service)
			btd_service_connecting_complete(conn->service, 0);

		conn->connected = true;
		return;
	}

	error("%s replied with an error: %s, %s", ext->name,
						err.name, err.message);

	if (conn->service)
		btd_service_connecting_complete(conn->service, -ECONNREFUSED);

	dbus_error_free(&err);

	ext->conns = g_slist_remove(ext->conns, conn);
	ext_io_destroy(conn);
}

static void disconn_reply(DBusPendingCall *call, void *user_data)
{
	struct ext_io *conn = user_data;
	struct ext_profile *ext = conn->ext;
	DBusMessage *reply = dbus_pending_call_steal_reply(call);
	DBusError err;

	dbus_error_init(&err);
	dbus_set_error_from_message(&err, reply);

	dbus_message_unref(reply);

	dbus_pending_call_unref(conn->pending);
	conn->pending = NULL;

	if (!dbus_error_is_set(&err)) {
		if (conn->service)
			btd_service_disconnecting_complete(conn->service, 0);

		goto disconnect;
	}

	error("%s replied with an error: %s, %s", ext->name,
						err.name, err.message);

	if (conn->service)
		btd_service_disconnecting_complete(conn->service,
								-ECONNREFUSED);

	dbus_error_free(&err);

disconnect:
	ext->conns = g_slist_remove(ext->conns, conn);
	ext_io_destroy(conn);
}

struct prop_append_data {
	DBusMessageIter *dict;
	struct ext_io *io;
};

static void append_prop(gpointer a, gpointer b)
{
	struct btd_profile_custom_property *p = a;
	struct prop_append_data *data = b;
	DBusMessageIter entry, value, *dict = data->dict;
	struct btd_device *dev = data->io->device;
	struct ext_profile *ext = data->io->ext;
	const char *uuid = ext->service ? ext->service : ext->uuid;

	if (strcasecmp(p->uuid, uuid) != 0)
		return;

	if (p->exists && !p->exists(p->uuid, dev, p->user_data))
		return;

	dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY, NULL,
								&entry);
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &p->name);
	dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT, p->type,
								&value);

	p->get(p->uuid, dev, &value, p->user_data);

	dbus_message_iter_close_container(&entry, &value);
	dbus_message_iter_close_container(dict, &entry);
}

static uint16_t get_supported_features(const sdp_record_t *rec)
{
	sdp_data_t *data;

	data = sdp_data_get(rec, SDP_ATTR_SUPPORTED_FEATURES);
	if (!data || data->dtd != SDP_UINT16)
		return 0;

	return data->val.uint16;
}

static uint16_t get_profile_version(const sdp_record_t *rec)
{
	sdp_list_t *descs;
	uint16_t version;

	if (sdp_get_profile_descs(rec, &descs) < 0)
		return 0;

	if (descs && descs->data) {
		sdp_profile_desc_t *desc = descs->data;
		version = desc->version;
	} else {
		version = 0;
	}

	sdp_list_free(descs, free);

	return version;
}

static bool send_new_connection(struct ext_profile *ext, struct ext_io *conn)
{
	DBusMessage *msg;
	DBusMessageIter iter, dict;
	struct prop_append_data data = { &dict, conn };
	const char *remote_uuid = ext->remote_uuid;
	const sdp_record_t *rec;
	const char *path;
	int fd;

	msg = dbus_message_new_method_call(ext->owner, ext->path,
							"org.bluez.Profile1",
							"NewConnection");
	if (!msg) {
		error("Unable to create NewConnection call for %s", ext->name);
		return false;
	}

	if (remote_uuid) {
		rec = btd_device_get_record(conn->device, remote_uuid);
		if (rec) {
			conn->features = get_supported_features(rec);
			conn->version = get_profile_version(rec);
		}
	}

	dbus_message_iter_init_append(msg, &iter);

	path = device_get_path(conn->device);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH, &path);

	fd = g_io_channel_unix_get_fd(conn->io);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_UNIX_FD, &fd);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict);

	if (conn->version)
		dict_append_entry(&dict, "Version", DBUS_TYPE_UINT16,
							&conn->version);

	if (conn->features)
		dict_append_entry(&dict, "Features", DBUS_TYPE_UINT16,
							&conn->features);

	g_slist_foreach(custom_props, append_prop, &data);

	dbus_message_iter_close_container(&iter, &dict);

	if (!g_dbus_send_message_with_reply(btd_get_dbus_connection(),
						msg, &conn->pending, -1)) {
		error("%s: sending NewConnection failed", ext->name);
		dbus_message_unref(msg);
		return false;
	}

	dbus_message_unref(msg);

	dbus_pending_call_set_notify(conn->pending, new_conn_reply, conn, NULL);

	return true;
}

static void ext_connect(GIOChannel *io, GError *err, gpointer user_data)
{
	struct ext_io *conn = user_data;
	struct ext_profile *ext = conn->ext;
	GError *io_err = NULL;
	char addr[18];

	if (!bt_io_get(io, &io_err,
				BT_IO_OPT_DEST, addr,
				BT_IO_OPT_INVALID)) {
		error("Unable to get connect data for %s: %s", ext->name,
							io_err->message);
		if (err) {
			g_error_free(io_err);
			io_err = NULL;
		} else {
			err = io_err;
		}
		goto drop;
	}

	if (err != NULL) {
		error("%s failed to connect to %s: %s", ext->name, addr,
								err->message);
		goto drop;
	}

	DBG("%s connected to %s", ext->name, addr);

	if (conn->io_id == 0) {
		GIOCondition cond = G_IO_HUP | G_IO_ERR | G_IO_NVAL;
		conn->io_id = g_io_add_watch(io, cond, ext_io_disconnected,
									conn);
	}

	if (conn->service && service_accept(conn->service) < 0)
		goto drop;

	if (send_new_connection(ext, conn))
		return;

drop:
	if (conn->service)
		btd_service_connecting_complete(conn->service,
						err ? -err->code : -EIO);

	if (io_err)
		g_error_free(io_err);

	ext->conns = g_slist_remove(ext->conns, conn);
	ext_io_destroy(conn);
}

static void ext_auth(DBusError *err, void *user_data)
{
	struct ext_io *conn = user_data;
	struct ext_profile *ext = conn->ext;
	GError *gerr = NULL;
	char addr[18];

	conn->auth_id = 0;

	bt_io_get(conn->io, &gerr, BT_IO_OPT_DEST, addr, BT_IO_OPT_INVALID);
	if (gerr != NULL) {
		error("Unable to get connect data for %s: %s",
						ext->name, gerr->message);
		g_error_free(gerr);
		goto drop;
	}

	if (err && dbus_error_is_set(err)) {
		error("%s rejected %s: %s", ext->name, addr, err->message);
		goto drop;
	}

	if (!bt_io_accept(conn->io, ext_connect, conn, NULL, &gerr)) {
		error("bt_io_accept: %s", gerr->message);
		g_error_free(gerr);
		goto drop;
	}

	DBG("%s authorized to connect to %s", addr, ext->name);

	return;

drop:
	ext->conns = g_slist_remove(ext->conns, conn);
	ext_io_destroy(conn);
}

static struct ext_io *create_conn(struct ext_io *server, GIOChannel *io,
						bdaddr_t *src, bdaddr_t *dst)
{
	struct btd_device *device;
	struct btd_service *service;
	struct ext_io *conn;
	GIOCondition cond;
	char addr[18];

	device = btd_adapter_find_device(server->adapter, dst, BDADDR_BREDR);
	if (device == NULL) {
		ba2str(dst, addr);
		error("%s device %s not found", server->ext->name, addr);
		return NULL;
	}

	/* Do not add UUID if client role is not enabled */
	if (!server->ext->enable_client) {
		service = NULL;
		goto done;
	}

	btd_device_add_uuid(device, server->ext->remote_uuid);
	service = btd_device_get_service(device, server->ext->remote_uuid);
	if (service == NULL) {
		ba2str(dst, addr);
		error("%s service not found for device %s", server->ext->name,
									addr);
		return NULL;
	}

done:
	conn = g_new0(struct ext_io, 1);
	conn->io = g_io_channel_ref(io);
	conn->proto = server->proto;
	conn->ext = server->ext;
	conn->adapter = btd_adapter_ref(server->adapter);
	conn->device = btd_device_ref(device);

	if (service)
		conn->service = btd_service_ref(service);

	cond = G_IO_HUP | G_IO_ERR | G_IO_NVAL;
	conn->io_id = g_io_add_watch(io, cond, ext_io_disconnected, conn);

	return conn;
}

static void ext_confirm(GIOChannel *io, gpointer user_data)
{
	struct ext_io *server = user_data;
	struct ext_profile *ext = server->ext;
	const char *uuid = ext->service ? ext->service : ext->uuid;
	struct ext_io *conn;
	GError *gerr = NULL;
	bdaddr_t src, dst;
	char addr[18];

	bt_io_get(io, &gerr,
			BT_IO_OPT_SOURCE_BDADDR, &src,
			BT_IO_OPT_DEST_BDADDR, &dst,
			BT_IO_OPT_DEST, addr,
			BT_IO_OPT_INVALID);
	if (gerr != NULL) {
		error("%s failed to get connect data: %s", ext->name,
								gerr->message);
		g_error_free(gerr);
		return;
	}

	DBG("incoming connect from %s", addr);

	conn = create_conn(server, io, &src, &dst);
	if (conn == NULL)
		return;

	conn->auth_id = btd_request_authorization(&src, &dst, uuid, ext_auth,
									conn);
	if (conn->auth_id == 0) {
		error("%s authorization failure", ext->name);
		ext_io_destroy(conn);
		return;
	}

	ext->conns = g_slist_append(ext->conns, conn);

	DBG("%s authorizing connection from %s", ext->name, addr);
}

static void ext_direct_connect(GIOChannel *io, GError *err, gpointer user_data)
{
	struct ext_io *server = user_data;
	struct ext_profile *ext = server->ext;
	GError *gerr = NULL;
	struct ext_io *conn;
	bdaddr_t src, dst;

	bt_io_get(io, &gerr,
			BT_IO_OPT_SOURCE_BDADDR, &src,
			BT_IO_OPT_DEST_BDADDR, &dst,
			BT_IO_OPT_INVALID);
	if (gerr != NULL) {
		error("%s failed to get connect data: %s", ext->name,
								gerr->message);
		g_error_free(gerr);
		return;
	}

	conn = create_conn(server, io, &src, &dst);
	if (conn == NULL)
		return;

	ext->conns = g_slist_append(ext->conns, conn);

	ext_connect(io, err, conn);
}

static uint32_t ext_register_record(struct ext_profile *ext,
							struct ext_io *l2cap,
							struct ext_io *rfcomm,
							struct btd_adapter *a)
{
	sdp_record_t *rec;
	char *dyn_record = NULL;
	const char *record = ext->record;

	if (!record && ext->get_record) {
		dyn_record = ext->get_record(ext, l2cap, rfcomm);
		record = dyn_record;
	}

	if (!record)
		return 0;

	rec = sdp_xml_parse_record(record, strlen(record));

	g_free(dyn_record);

	if (!rec) {
		error("Unable to parse record for %s", ext->name);
		return 0;
	}

	if (adapter_service_add(a, rec) < 0) {
		error("Failed to register service record");
		sdp_record_free(rec);
		return 0;
	}

	return rec->handle;
}

static uint32_t ext_start_servers(struct ext_profile *ext,
						struct btd_adapter *adapter)
{
	struct ext_io *l2cap = NULL;
	struct ext_io *rfcomm = NULL;
	BtIOConfirm confirm;
	BtIOConnect connect;
	GError *err = NULL;
	GIOChannel *io;

	if (ext->authorize) {
		confirm = ext_confirm;
		connect = NULL;
	} else {
		confirm = NULL;
		connect = ext_direct_connect;
	}

	if (ext->local_psm) {
		uint16_t psm;

		if (ext->local_psm > 0)
			psm = ext->local_psm;
		else
			psm = 0;

		l2cap = g_new0(struct ext_io, 1);
		l2cap->ext = ext;

		io = bt_io_listen(connect, confirm, l2cap, NULL, &err,
					BT_IO_OPT_SOURCE_BDADDR,
					btd_adapter_get_address(adapter),
					BT_IO_OPT_MODE, ext->mode,
					BT_IO_OPT_PSM, psm,
					BT_IO_OPT_SEC_LEVEL, ext->sec_level,
					BT_IO_OPT_INVALID);
		if (err != NULL) {
			error("L2CAP server failed for %s: %s",
						ext->name, err->message);
			g_free(l2cap);
			l2cap = NULL;
			g_clear_error(&err);
			goto failed;
		} else {
			if (psm == 0)
				bt_io_get(io, NULL, BT_IO_OPT_PSM, &psm,
							BT_IO_OPT_INVALID);
			l2cap->io = io;
			l2cap->proto = BTPROTO_L2CAP;
			l2cap->psm = psm;
			l2cap->adapter = btd_adapter_ref(adapter);
			ext->servers = g_slist_append(ext->servers, l2cap);
			DBG("%s listening on PSM %u", ext->name, psm);
		}
	}

	if (ext->local_chan) {
		uint8_t chan;

		if (ext->local_chan > 0)
			chan = ext->local_chan;
		else
			chan = 0;

		rfcomm = g_new0(struct ext_io, 1);
		rfcomm->ext = ext;

		io = bt_io_listen(connect, confirm, rfcomm, NULL, &err,
					BT_IO_OPT_SOURCE_BDADDR,
					btd_adapter_get_address(adapter),
					BT_IO_OPT_CHANNEL, chan,
					BT_IO_OPT_SEC_LEVEL, ext->sec_level,
					BT_IO_OPT_INVALID);
		if (err != NULL) {
			error("RFCOMM server failed for %s: %s",
						ext->name, err->message);
			g_free(rfcomm);
			g_clear_error(&err);
			goto failed;
		} else {
			if (chan == 0)
				bt_io_get(io, NULL, BT_IO_OPT_CHANNEL, &chan,
							BT_IO_OPT_INVALID);
			rfcomm->io = io;
			rfcomm->proto = BTPROTO_RFCOMM;
			rfcomm->chan = chan;
			rfcomm->adapter = btd_adapter_ref(adapter);
			ext->servers = g_slist_append(ext->servers, rfcomm);
			DBG("%s listening on chan %u", ext->name, chan);
		}
	}

	return ext_register_record(ext, l2cap, rfcomm, adapter);

failed:
	if (l2cap) {
		ext->servers = g_slist_remove(ext->servers, l2cap);
		ext_io_destroy(l2cap);
	}

	return 0;
}

static struct ext_profile *find_ext(struct btd_profile *p)
{
	GSList *l;

	l = g_slist_find(ext_profiles, p);
	if (!l)
		return NULL;

	return l->data;
}

static int ext_adapter_probe(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct ext_profile *ext;
	struct ext_record *rec;
	uint32_t handle;

	ext = find_ext(p);
	if (!ext)
		return -ENOENT;

	DBG("\"%s\" probed", ext->name);

	handle = ext_start_servers(ext, adapter);
	if (!handle)
		return 0;

	rec = g_new0(struct ext_record, 1);
	rec->adapter = btd_adapter_ref(adapter);
	rec->handle = handle;

	ext->records = g_slist_append(ext->records, rec);

	return 0;
}

static void ext_remove_records(struct ext_profile *ext,
						struct btd_adapter *adapter)
{
	GSList *l, *next;

	for (l = ext->records; l != NULL; l = next) {
		struct ext_record *r = l->data;

		next = g_slist_next(l);

		if (adapter && r->adapter != adapter)
			continue;

		ext->records = g_slist_remove(ext->records, r);

		adapter_service_remove(adapter, r->handle);
		btd_adapter_unref(r->adapter);
		g_free(r);
	}
}

static void ext_adapter_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct ext_profile *ext;
	GSList *l, *next;

	ext = find_ext(p);
	if (!ext)
		return;

	DBG("\"%s\" removed", ext->name);

	ext_remove_records(ext, adapter);

	for (l = ext->servers; l != NULL; l = next) {
		struct ext_io *server = l->data;

		next = g_slist_next(l);

		if (server->adapter != adapter)
			continue;

		ext->servers = g_slist_remove(ext->servers, server);
		ext_io_destroy(server);
	}
}

static int ext_device_probe(struct btd_service *service)
{
	struct btd_profile *p = btd_service_get_profile(service);
	struct ext_profile *ext;

	ext = find_ext(p);
	if (!ext)
		return -ENOENT;

	DBG("%s probed with UUID %s", ext->name, p->remote_uuid);

	return 0;
}

static struct ext_io *find_connection(struct ext_profile *ext,
							struct btd_device *dev)
{
	GSList *l;

	for (l = ext->conns; l != NULL; l = g_slist_next(l)) {
		struct ext_io *conn = l->data;

		if (conn->device == dev)
			return conn;
	}

	return NULL;
}

static void ext_device_remove(struct btd_service *service)
{
	struct btd_profile *p = btd_service_get_profile(service);
	struct btd_device *dev = btd_service_get_device(service);
	struct ext_profile *ext;
	struct ext_io *conn;

	ext = find_ext(p);
	if (!ext)
		return;

	DBG("%s", ext->name);

	conn = find_connection(ext, dev);
	if (conn) {
		ext->conns = g_slist_remove(ext->conns, conn);
		ext_io_destroy(conn);
	}
}

static int connect_io(struct ext_io *conn, const bdaddr_t *src,
							const bdaddr_t *dst)
{
	struct ext_profile *ext = conn->ext;
	GError *gerr = NULL;
	GIOChannel *io;

	if (conn->psm) {
		conn->proto = BTPROTO_L2CAP;
		io = bt_io_connect(ext_connect, conn, NULL, &gerr,
					BT_IO_OPT_SOURCE_BDADDR, src,
					BT_IO_OPT_DEST_BDADDR, dst,
					BT_IO_OPT_SEC_LEVEL, ext->sec_level,
					BT_IO_OPT_PSM, conn->psm,
					BT_IO_OPT_INVALID);
	} else {
		conn->proto = BTPROTO_RFCOMM;
		io = bt_io_connect(ext_connect, conn, NULL, &gerr,
					BT_IO_OPT_SOURCE_BDADDR, src,
					BT_IO_OPT_DEST_BDADDR, dst,
					BT_IO_OPT_SEC_LEVEL, ext->sec_level,
					BT_IO_OPT_CHANNEL, conn->chan,
					BT_IO_OPT_INVALID);
	}

	if (gerr != NULL) {
		error("Unable to connect %s: %s", ext->name, gerr->message);
		g_error_free(gerr);
		return -EIO;
	}

	conn->io = io;

	return 0;
}

static uint16_t get_goep_l2cap_psm(sdp_record_t *rec)
{
	sdp_data_t *data;

	data = sdp_data_get(rec, SDP_ATTR_GOEP_L2CAP_PSM);
	if (!data)
		return 0;

	if (data->dtd != SDP_UINT16)
		return 0;

	/* PSM must be odd and lsb of upper byte must be 0 */
	if ((data->val.uint16 & 0x0101) != 0x0001)
		return 0;

	return data->val.uint16;
}

static void record_cb(sdp_list_t *recs, int err, gpointer user_data)
{
	struct ext_io *conn = user_data;
	struct ext_profile *ext = conn->ext;
	sdp_list_t *r;

	conn->resolving = false;

	if (err < 0) {
		error("Unable to get %s SDP record: %s", ext->name,
							strerror(-err));
		goto failed;
	}

	if (!recs || !recs->data) {
		error("No SDP records found for %s", ext->name);
		err = -ENOTSUP;
		goto failed;
	}

	for (r = recs; r != NULL; r = r->next) {
		sdp_record_t *rec = r->data;
		sdp_list_t *protos;
		int port;

		if (sdp_get_access_protos(rec, &protos) < 0) {
			error("Unable to get proto list from %s record",
								ext->name);
			err = -ENOTSUP;
			goto failed;
		}

		port = sdp_get_proto_port(protos, L2CAP_UUID);
		if (port > 0)
			conn->psm = port;

		port = sdp_get_proto_port(protos, RFCOMM_UUID);
		if (port > 0)
			conn->chan = port;

		if (conn->psm == 0 && sdp_get_proto_desc(protos, OBEX_UUID))
			conn->psm = get_goep_l2cap_psm(rec);

		conn->features = get_supported_features(rec);
		conn->version = get_profile_version(rec);

		sdp_list_foreach(protos, (sdp_list_func_t) sdp_list_free,
									NULL);
		sdp_list_free(protos, NULL);

		if (conn->chan || conn->psm)
			break;
	}

	if (!conn->chan && !conn->psm) {
		error("Failed to find L2CAP PSM or RFCOMM channel for %s",
								ext->name);
		err = -ENOTSUP;
		goto failed;
	}

	err = connect_io(conn, btd_adapter_get_address(conn->adapter),
					device_get_address(conn->device));
	if (err < 0) {
		error("Connecting %s failed: %s", ext->name, strerror(-err));
		goto failed;
	}

	return;

failed:
	if (conn->service)
		btd_service_connecting_complete(conn->service, err);

	ext->conns = g_slist_remove(ext->conns, conn);
	ext_io_destroy(conn);
}

static int resolve_service(struct ext_io *conn, const bdaddr_t *src,
							const bdaddr_t *dst)
{
	struct ext_profile *ext = conn->ext;
	uuid_t uuid;
	int err;

	bt_string2uuid(&uuid, ext->remote_uuid);
	sdp_uuid128_to_uuid(&uuid);

	err = bt_search_service(src, dst, &uuid, record_cb, conn, NULL, 0);
	if (err == 0)
		conn->resolving = true;

	return err;
}

static int ext_connect_dev(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);
	struct btd_profile *profile = btd_service_get_profile(service);
	struct btd_adapter *adapter;
	struct ext_io *conn;
	struct ext_profile *ext;
	int err;

	ext = find_ext(profile);
	if (!ext)
		return -ENOENT;

	conn = find_connection(ext, dev);
	if (conn)
		return -EALREADY;

	adapter = device_get_adapter(dev);

	conn = g_new0(struct ext_io, 1);
	conn->ext = ext;

	if (ext->remote_psm || ext->remote_chan) {
		conn->psm = ext->remote_psm;
		conn->chan = ext->remote_chan;
		err = connect_io(conn, btd_adapter_get_address(adapter),
						device_get_address(dev));
	} else {
		err = resolve_service(conn, btd_adapter_get_address(adapter),
						device_get_address(dev));
	}

	if (err < 0)
		goto failed;

	conn->adapter = btd_adapter_ref(adapter);
	conn->device = btd_device_ref(dev);
	conn->service = btd_service_ref(service);

	ext->conns = g_slist_append(ext->conns, conn);

	return 0;

failed:
	g_free(conn);
	return err;
}

static int send_disconn_req(struct ext_profile *ext, struct ext_io *conn)
{
	DBusMessage *msg;
	const char *path;

	msg = dbus_message_new_method_call(ext->owner, ext->path,
						"org.bluez.Profile1",
						"RequestDisconnection");
	if (!msg) {
		error("Unable to create RequestDisconnection call for %s",
								ext->name);
		return -ENOMEM;
	}

	path = device_get_path(conn->device);
	dbus_message_append_args(msg, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID);

	if (!g_dbus_send_message_with_reply(btd_get_dbus_connection(),
						msg, &conn->pending, -1)) {
		error("%s: sending RequestDisconnection failed", ext->name);
		dbus_message_unref(msg);
		return -EIO;
	}

	dbus_message_unref(msg);

	dbus_pending_call_set_notify(conn->pending, disconn_reply, conn, NULL);

	return 0;
}

static int ext_disconnect_dev(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);
	struct btd_profile *profile = btd_service_get_profile(service);
	struct ext_profile *ext;
	struct ext_io *conn;
	int err;

	ext = find_ext(profile);
	if (!ext)
		return -ENOENT;

	conn = find_connection(ext, dev);
	if (!conn || !conn->connected)
		return -ENOTCONN;

	if (conn->pending)
		return -EBUSY;

	err = send_disconn_req(ext, conn);
	if (err < 0)
		return err;

	return 0;
}

static char *get_hfp_hf_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	return g_strdup_printf(HFP_HF_RECORD, rfcomm->chan, ext->version,
						ext->name, ext->features);
}

static char *get_hfp_ag_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	return g_strdup_printf(HFP_AG_RECORD, rfcomm->chan, ext->version,
						ext->name, ext->features);
}

static char *get_hsp_ag_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	return g_strdup_printf(HSP_AG_RECORD, rfcomm->chan, ext->version,
						ext->name);
}

static char *get_spp_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	char *svc, *rec;

	if (ext->service)
		svc = g_strdup_printf("<uuid value=\"%s\" />", ext->service);
	else
		svc = g_strdup("");

	rec = g_strdup_printf(SPP_RECORD, svc, rfcomm->chan, ext->version,
								ext->name);
	g_free(svc);
	return rec;
}

static char *get_dun_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	return g_strdup_printf(DUN_RECORD, rfcomm->chan, ext->version,
								ext->name);
}

static char *get_pce_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	return g_strdup_printf(PCE_RECORD, ext->version, ext->name);
}

static char *get_pse_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	uint16_t psm = 0;
	uint8_t chan = 0;

	if (l2cap)
		psm = l2cap->psm;
	if (rfcomm)
		chan = rfcomm->chan;

	return g_strdup_printf(PSE_RECORD, chan, ext->version, ext->name, psm);
}

static char *get_mas_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	uint16_t psm = 0;
	uint8_t chan = 0;

	if (l2cap)
		psm = l2cap->psm;
	if (rfcomm)
		chan = rfcomm->chan;

	return g_strdup_printf(MAS_RECORD, chan, ext->version, ext->name, psm);
}

static char *get_mns_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	uint16_t psm = 0;
	uint8_t chan = 0;

	if (l2cap)
		psm = l2cap->psm;
	if (rfcomm)
		chan = rfcomm->chan;

	return g_strdup_printf(MNS_RECORD, chan, ext->version, ext->name, psm);
}

static char *get_sync_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	return g_strdup_printf(SYNC_RECORD, rfcomm->chan, ext->version,
								ext->name);
}

static char *get_opp_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	uint16_t psm = 0;
	uint8_t chan = 0;

	if (l2cap)
		psm = l2cap->psm;
	if (rfcomm)
		chan = rfcomm->chan;

	return g_strdup_printf(OPP_RECORD, chan, ext->version, psm, ext->name);
}

static char *get_ftp_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	uint16_t psm = 0;
	uint8_t chan = 0;

	if (l2cap)
		psm = l2cap->psm;
	if (rfcomm)
		chan = rfcomm->chan;

	return g_strdup_printf(FTP_RECORD, chan, ext->version, psm, ext->name);
}

#define RFCOMM_SEQ	"<sequence>				\
				<uuid value=\"0x0003\" />	\
				<uint8 value=\"0x%02x\" />	\
			</sequence>"

#define VERSION_ATTR							\
		"<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"%s\" />		\
					<uint16 value=\"0x%04x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>"

static char *get_generic_record(struct ext_profile *ext, struct ext_io *l2cap,
							struct ext_io *rfcomm)
{
	char uuid_str[MAX_LEN_UUID_STR], svc_str[MAX_LEN_UUID_STR], psm[30];
	char *rf_seq, *ver_attr, *rec;
	uuid_t uuid;

	bt_string2uuid(&uuid, ext->uuid);
	sdp_uuid2strn(&uuid, uuid_str, sizeof(uuid_str));

	if (ext->service) {
		bt_string2uuid(&uuid, ext->service);
		sdp_uuid2strn(&uuid, svc_str, sizeof(svc_str));
	} else {
		strncpy(svc_str, uuid_str, sizeof(svc_str));
	}

	if (l2cap)
		snprintf(psm, sizeof(psm), "<uint16 value=\"0x%04x\" />",
								l2cap->psm);
	else
		psm[0] = '\0';

	if (rfcomm)
		rf_seq = g_strdup_printf(RFCOMM_SEQ, rfcomm->chan);
	else
		rf_seq = g_strdup("");

	if (ext->version)
		ver_attr = g_strdup_printf(VERSION_ATTR, uuid_str,
								ext->version);
	else
		ver_attr = g_strdup("");

	rec = g_strdup_printf(GENERIC_RECORD, svc_str, psm, rf_seq, ver_attr,
								ext->name);

	g_free(rf_seq);
	g_free(ver_attr);

	return rec;
}

static struct default_settings {
	const char	*uuid;
	const char	*name;
	int		priority;
	const char	*remote_uuid;
	int		channel;
	int		psm;
	BtIOMode	mode;
	BtIOSecLevel	sec_level;
	bool		authorize;
	bool		auto_connect;
	char *		(*get_record)(struct ext_profile *ext,
					struct ext_io *l2cap,
					struct ext_io *rfcomm);
	uint16_t	version;
	uint16_t	features;
} defaults[] = {
	{
		.uuid		= SPP_UUID,
		.name		= "Serial Port",
		.channel	= SPP_DEFAULT_CHANNEL,
		.authorize	= true,
		.get_record	= get_spp_record,
		.version	= 0x0102,
	}, {
		.uuid		= DUN_GW_UUID,
		.name		= "Dial-Up Networking",
		.channel	= DUN_DEFAULT_CHANNEL,
		.authorize	= true,
		.get_record	= get_dun_record,
		.version	= 0x0102,
	}, {
		.uuid		= HFP_HS_UUID,
		.name		= "Hands-Free unit",
		.priority	= BTD_PROFILE_PRIORITY_HIGH,
		.remote_uuid	= HFP_AG_UUID,
		.channel	= HFP_HF_DEFAULT_CHANNEL,
		.authorize	= true,
		.auto_connect	= true,
		.get_record	= get_hfp_hf_record,
		.version	= 0x0105,
	}, {
		.uuid		= HFP_AG_UUID,
		.name		= "Hands-Free Voice gateway",
		.priority	= BTD_PROFILE_PRIORITY_HIGH,
		.remote_uuid	= HFP_HS_UUID,
		.channel	= HFP_AG_DEFAULT_CHANNEL,
		.authorize	= true,
		.auto_connect	= true,
		.get_record	= get_hfp_ag_record,
		.version	= 0x0105,
	}, {
		.uuid		= HSP_AG_UUID,
		.name		= "Headset Voice gateway",
		.priority	= BTD_PROFILE_PRIORITY_HIGH,
		.remote_uuid	= HSP_HS_UUID,
		.channel	= HSP_AG_DEFAULT_CHANNEL,
		.authorize	= true,
		.auto_connect	= true,
		.get_record	= get_hsp_ag_record,
		.version	= 0x0102,
	}, {
		.uuid		= OBEX_OPP_UUID,
		.name		= "Object Push",
		.channel	= OPP_DEFAULT_CHANNEL,
		.psm		= BTD_PROFILE_PSM_AUTO,
		.mode		= BT_IO_MODE_ERTM,
		.sec_level	= BT_IO_SEC_LOW,
		.authorize	= false,
		.get_record	= get_opp_record,
		.version	= 0x0102,
	}, {
		.uuid		= OBEX_FTP_UUID,
		.name		= "File Transfer",
		.channel	= FTP_DEFAULT_CHANNEL,
		.psm		= BTD_PROFILE_PSM_AUTO,
		.mode		= BT_IO_MODE_ERTM,
		.authorize	= true,
		.get_record	= get_ftp_record,
		.version	= 0x0102,
	}, {
		.uuid		= OBEX_SYNC_UUID,
		.name		= "Synchronization",
		.channel	= SYNC_DEFAULT_CHANNEL,
		.authorize	= true,
		.get_record	= get_sync_record,
		.version	= 0x0100,
	}, {
		.uuid		= OBEX_PSE_UUID,
		.name		= "Phone Book Access",
		.channel	= PBAP_DEFAULT_CHANNEL,
		.psm		= BTD_PROFILE_PSM_AUTO,
		.mode		= BT_IO_MODE_ERTM,
		.authorize	= true,
		.get_record	= get_pse_record,
		.version	= 0x0101,
	}, {
		.uuid		= OBEX_PCE_UUID,
		.name		= "Phone Book Access Client",
		.remote_uuid	= OBEX_PSE_UUID,
		.authorize	= true,
		.get_record	= get_pce_record,
		.version	= 0x0102,
	}, {
		.uuid		= OBEX_MAS_UUID,
		.name		= "Message Access",
		.channel	= MAS_DEFAULT_CHANNEL,
		.psm		= BTD_PROFILE_PSM_AUTO,
		.mode		= BT_IO_MODE_ERTM,
		.authorize	= true,
		.get_record	= get_mas_record,
		.version	= 0x0100
	}, {
		.uuid		= OBEX_MNS_UUID,
		.name		= "Message Notification",
		.channel	= MNS_DEFAULT_CHANNEL,
		.psm		= BTD_PROFILE_PSM_AUTO,
		.mode		= BT_IO_MODE_ERTM,
		.authorize	= true,
		.get_record	= get_mns_record,
		.version	= 0x0102
	},
};

static void ext_set_defaults(struct ext_profile *ext)
{
	unsigned int i;

	ext->mode = BT_IO_MODE_BASIC;
	ext->sec_level = BT_IO_SEC_MEDIUM;
	ext->authorize = true;
	ext->enable_client = true;
	ext->enable_server = true;
	ext->remote_uuid = NULL;

	for (i = 0; i < G_N_ELEMENTS(defaults); i++) {
		struct default_settings *settings = &defaults[i];
		const char *remote_uuid;

		if (strcasecmp(ext->uuid, settings->uuid) != 0)
			continue;

		if (settings->remote_uuid)
			remote_uuid = settings->remote_uuid;
		else
			remote_uuid = ext->uuid;

		ext->remote_uuid = g_strdup(remote_uuid);

		if (settings->channel)
			ext->local_chan = settings->channel;

		if (settings->psm)
			ext->local_psm = settings->psm;

		if (settings->sec_level)
			ext->sec_level = settings->sec_level;

		if (settings->mode)
			ext->mode = settings->mode;

		ext->authorize = settings->authorize;

		if (settings->auto_connect)
			ext->p.auto_connect = true;

		if (settings->priority)
			ext->p.priority = settings->priority;

		if (settings->get_record)
			ext->get_record = settings->get_record;

		if (settings->version)
			ext->version = settings->version;

		if (settings->features)
			ext->features = settings->features;

		if (settings->name)
			ext->name = g_strdup(settings->name);
	}
}

static int parse_ext_opt(struct ext_profile *ext, const char *key,
							DBusMessageIter *value)
{
	int type = dbus_message_iter_get_arg_type(value);
	const char *str;
	uint16_t u16;
	dbus_bool_t b;

	if (strcasecmp(key, "Name") == 0) {
		if (type != DBUS_TYPE_STRING)
			return -EINVAL;
		dbus_message_iter_get_basic(value, &str);
		g_free(ext->name);
		ext->name = g_strdup(str);
	} else if (strcasecmp(key, "AutoConnect") == 0) {
		if (type != DBUS_TYPE_BOOLEAN)
			return -EINVAL;
		dbus_message_iter_get_basic(value, &b);
		ext->p.auto_connect = b;
	} else if (strcasecmp(key, "PSM") == 0) {
		if (type != DBUS_TYPE_UINT16)
			return -EINVAL;
		dbus_message_iter_get_basic(value, &u16);
		ext->local_psm = u16 ? u16 : BTD_PROFILE_PSM_AUTO;
	} else if (strcasecmp(key, "Channel") == 0) {
		if (type != DBUS_TYPE_UINT16)
			return -EINVAL;

		dbus_message_iter_get_basic(value, &u16);
		if (u16 > 31)
			return -EINVAL;
		ext->local_chan = u16 ? u16 : BTD_PROFILE_CHAN_AUTO;
	} else if (strcasecmp(key, "RequireAuthentication") == 0) {
		if (type != DBUS_TYPE_BOOLEAN)
			return -EINVAL;

		dbus_message_iter_get_basic(value, &b);
		if (b)
			ext->sec_level = BT_IO_SEC_MEDIUM;
		else
			ext->sec_level = BT_IO_SEC_LOW;
	} else if (strcasecmp(key, "RequireAuthorization") == 0) {
		if (type != DBUS_TYPE_BOOLEAN)
			return -EINVAL;
		dbus_message_iter_get_basic(value, &b);
		ext->authorize = b;
	} else if (strcasecmp(key, "Role") == 0) {
		if (type != DBUS_TYPE_STRING)
			return -EINVAL;
		dbus_message_iter_get_basic(value, &str);
		g_free(ext->role);
		ext->role = g_strdup(str);

		if (g_str_equal(ext->role, "client")) {
			ext->enable_server = false;
			ext->enable_client = true;
		} else if (g_str_equal(ext->role, "server")) {
			ext->enable_server = true;
			ext->enable_client = false;
		}
	} else if (strcasecmp(key, "ServiceRecord") == 0) {
		if (type != DBUS_TYPE_STRING)
			return -EINVAL;
		dbus_message_iter_get_basic(value, &str);
		g_free(ext->record);
		ext->record = g_strdup(str);
		ext->enable_server = true;
	} else if (strcasecmp(key, "Version") == 0) {
		uint16_t ver;

		if (type != DBUS_TYPE_UINT16)
			return -EINVAL;

		dbus_message_iter_get_basic(value, &ver);
		ext->version = ver;
	} else if (strcasecmp(key, "Features") == 0) {
		uint16_t feat;

		if (type != DBUS_TYPE_UINT16)
			return -EINVAL;

		dbus_message_iter_get_basic(value, &feat);
		ext->features = feat;
	} else if (strcasecmp(key, "Service") == 0) {
		if (type != DBUS_TYPE_STRING)
			return -EINVAL;
		dbus_message_iter_get_basic(value, &str);
		free(ext->service);
		ext->service = bt_name2string(str);
	}

	return 0;
}

static void set_service(struct ext_profile *ext)
{
	if (strcasecmp(ext->uuid, HSP_HS_UUID) == 0) {
		ext->service = strdup(ext->uuid);
	} else if (strcasecmp(ext->uuid, HSP_AG_UUID) == 0) {
		ext->service = ext->uuid;
		ext->uuid = strdup(HSP_HS_UUID);
	} else if (strcasecmp(ext->uuid, HFP_HS_UUID) == 0) {
		ext->service = strdup(ext->uuid);
	} else if (strcasecmp(ext->uuid, HFP_AG_UUID) == 0) {
		ext->service = ext->uuid;
		ext->uuid = strdup(HFP_HS_UUID);
	} else if (strcasecmp(ext->uuid, OBEX_SYNC_UUID) == 0 ||
			strcasecmp(ext->uuid, OBEX_OPP_UUID) == 0 ||
			strcasecmp(ext->uuid, OBEX_FTP_UUID) == 0) {
		ext->service = strdup(ext->uuid);
	} else if (strcasecmp(ext->uuid, OBEX_PSE_UUID) == 0 ||
			strcasecmp(ext->uuid, OBEX_PCE_UUID) ==  0) {
		ext->service = ext->uuid;
		ext->uuid = strdup(OBEX_PBAP_UUID);
	} else if (strcasecmp(ext->uuid, OBEX_MAS_UUID) == 0 ||
			strcasecmp(ext->uuid, OBEX_MNS_UUID) == 0) {
		ext->service = ext->uuid;
		ext->uuid = strdup(OBEX_MAP_UUID);
	}
}

static struct ext_profile *create_ext(const char *owner, const char *path,
					const char *uuid,
					DBusMessageIter *opts)
{
	struct btd_profile *p;
	struct ext_profile *ext;

	ext = g_new0(struct ext_profile, 1);

	ext->uuid = bt_name2string(uuid);
	if (ext->uuid == NULL) {
		g_free(ext);
		return NULL;
	}

	ext->owner = g_strdup(owner);
	ext->path = g_strdup(path);

	ext_set_defaults(ext);

	while (dbus_message_iter_get_arg_type(opts) == DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter value, entry;
		const char *key;

		dbus_message_iter_recurse(opts, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		if (parse_ext_opt(ext, key, &value) < 0)
			error("Invalid value for profile option %s", key);

		dbus_message_iter_next(opts);
	}

	if (!ext->service)
		set_service(ext);

	if (ext->enable_server && !(ext->record || ext->get_record))
		ext->get_record = get_generic_record;

	if (!ext->name)
		ext->name = g_strdup_printf("%s%s/%s", owner, path, uuid);

	if (!ext->remote_uuid) {
		if (ext->service)
			ext->remote_uuid = g_strdup(ext->service);
		else
			ext->remote_uuid = g_strdup(ext->uuid);
	}

	p = &ext->p;

	p->name = ext->name;
	p->local_uuid = ext->service ? ext->service : ext->uuid;
	p->remote_uuid = ext->remote_uuid;
	p->external = true;

	if (ext->enable_server) {
		p->adapter_probe = ext_adapter_probe;
		p->adapter_remove = ext_adapter_remove;
	}

	if (ext->enable_client) {
		p->device_probe = ext_device_probe;
		p->device_remove = ext_device_remove;
		p->connect = ext_connect_dev;
		p->disconnect = ext_disconnect_dev;
	}

	DBG("Created \"%s\"", ext->name);

	ext_profiles = g_slist_append(ext_profiles, ext);

	adapter_foreach(adapter_add_profile, &ext->p);

	return ext;
}

static void remove_ext(struct ext_profile *ext)
{
	adapter_foreach(adapter_remove_profile, &ext->p);

	ext_profiles = g_slist_remove(ext_profiles, ext);

	DBG("Removed \"%s\"", ext->name);

	ext_remove_records(ext, NULL);

	g_slist_free_full(ext->servers, ext_io_destroy);
	g_slist_free_full(ext->conns, ext_io_destroy);

	g_free(ext->remote_uuid);
	g_free(ext->name);
	g_free(ext->owner);
	free(ext->uuid);
	free(ext->service);
	g_free(ext->role);
	g_free(ext->path);
	g_free(ext->record);

	g_free(ext);
}

static void ext_exited(DBusConnection *conn, void *user_data)
{
	struct ext_profile *ext = user_data;

	DBG("\"%s\" exited", ext->name);

	remove_ext(ext);
}

static DBusMessage *register_profile(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *path, *sender, *uuid;
	DBusMessageIter args, opts;
	struct ext_profile *ext;

	sender = dbus_message_get_sender(msg);

	DBG("sender %s", sender);

	dbus_message_iter_init(msg, &args);

	dbus_message_iter_get_basic(&args, &path);
	dbus_message_iter_next(&args);

	ext = find_ext_profile(sender, path);
	if (ext)
		return btd_error_already_exists(msg);

	dbus_message_iter_get_basic(&args, &uuid);
	dbus_message_iter_next(&args);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY)
		return btd_error_invalid_args(msg);

	dbus_message_iter_recurse(&args, &opts);

	ext = create_ext(sender, path, uuid, &opts);
	if (!ext)
		return btd_error_invalid_args(msg);

	ext->id = g_dbus_add_disconnect_watch(conn, sender, ext_exited, ext,
									NULL);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *unregister_profile(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *path, *sender;
	struct ext_profile *ext;

	sender = dbus_message_get_sender(msg);

	DBG("sender %s", sender);

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	ext = find_ext_profile(sender, path);
	if (!ext)
		return btd_error_does_not_exist(msg);

	g_dbus_remove_watch(conn, ext->id);
	remove_ext(ext);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable methods[] = {
	{ GDBUS_METHOD("RegisterProfile",
			GDBUS_ARGS({ "profile", "o"}, { "UUID", "s" },
						{ "options", "a{sv}" }),
			NULL, register_profile) },
	{ GDBUS_METHOD("UnregisterProfile", GDBUS_ARGS({ "profile", "o" }),
			NULL, unregister_profile) },
	{ }
};

static struct btd_profile_custom_property *find_custom_prop(const char *uuid,
							const char *name)
{
	GSList *l;

	for (l = custom_props; l; l = l->next) {
		struct btd_profile_custom_property *prop = l->data;

		if (strcasecmp(prop->uuid, uuid) != 0)
			continue;

		if (g_strcmp0(prop->name, name) == 0)
			return prop;
	}

	return NULL;
}

bool btd_profile_add_custom_prop(const char *uuid, const char *type,
					const char *name,
					btd_profile_prop_exists exists,
					btd_profile_prop_get get,
					void *user_data)
{
	struct btd_profile_custom_property *prop;

	prop = find_custom_prop(uuid, name);
	if (prop != NULL)
		return false;

	prop = g_new0(struct btd_profile_custom_property, 1);

	prop->uuid = strdup(uuid);
	prop->type = g_strdup(type);
	prop->name = g_strdup(name);
	prop->exists = exists;
	prop->get = get;
	prop->user_data = user_data;

	custom_props = g_slist_append(custom_props, prop);

	return true;
}

static void free_property(gpointer data)
{
	struct btd_profile_custom_property *p = data;

	g_free(p->uuid);
	g_free(p->type);
	g_free(p->name);

	g_free(p);
}

bool btd_profile_remove_custom_prop(const char *uuid, const char *name)
{
	struct btd_profile_custom_property *prop;

	prop = find_custom_prop(uuid, name);
	if (prop == NULL)
		return false;

	custom_props = g_slist_remove(custom_props, prop);
	free_property(prop);

	return false;
}

void btd_profile_init(void)
{
	g_dbus_register_interface(btd_get_dbus_connection(),
				"/org/bluez", "org.bluez.ProfileManager1",
				methods, NULL, NULL, NULL, NULL);
}

void btd_profile_cleanup(void)
{
	while (ext_profiles) {
		struct ext_profile *ext = ext_profiles->data;
		DBusConnection *conn = btd_get_dbus_connection();
		DBusMessage *msg;

		DBG("Releasing \"%s\"", ext->name);

		g_slist_free_full(ext->conns, ext_io_destroy);
		ext->conns = NULL;

		msg = dbus_message_new_method_call(ext->owner, ext->path,
							"org.bluez.Profile1",
							"Release");
		if (msg)
			g_dbus_send_message(conn, msg);

		g_dbus_remove_watch(conn, ext->id);
		remove_ext(ext);

	}

	g_slist_free_full(custom_props, free_property);
	custom_props = NULL;

	g_dbus_unregister_interface(btd_get_dbus_connection(),
				"/org/bluez", "org.bluez.ProfileManager1");
}
